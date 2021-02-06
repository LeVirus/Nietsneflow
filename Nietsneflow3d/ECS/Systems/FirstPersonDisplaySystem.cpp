#include "FirstPersonDisplaySystem.hpp"
#include <CollisionUtils.hpp>
#include <PhysicalEngine.hpp>
#include <ECS/Components/VisionComponent.hpp>
#include <ECS/Components/GeneralCollisionComponent.hpp>
#include <ECS/Components/MapCoordComponent.hpp>
#include <ECS/Components/PositionVertexComponent.hpp>
#include <ECS/Components/RectangleCollisionComponent.hpp>
#include <ECS/Components/MoveableComponent.hpp>
#include <ECS/Components/SpriteTextureComponent.hpp>
#include <ECS/Components/CircleCollisionComponent.hpp>
#include <PictureData.hpp>
#include <cmath>

//===================================================================
FirstPersonDisplaySystem::FirstPersonDisplaySystem()
{
    setUsedComponents();
}

//===================================================================
void FirstPersonDisplaySystem::setUsedComponents()
{
    bAddComponentToSystem(Components_e::VISION_COMPONENT);
}

//===================================================================
void FirstPersonDisplaySystem::execSystem()
{
    System::execSystem();
    confCompVertexMemEntities();
    drawVertex();
}

//===================================================================
void FirstPersonDisplaySystem::confCompVertexMemEntities()
{
    uint32_t numEntity = mVectNumEntity.size();
    m_numVertexToDraw.resize(numEntity);
    //treat one player
    uint32_t toRemove = 0;
    VisionComponent *visionComp;
    MoveableComponent *moveComp;
    MapCoordComponent *mapCompA;
    GeneralCollisionComponent *genCollComp;
    MapCoordComponent *mapCompB;
    for(uint32_t i = 0; i < numEntity; ++i)
    {
        visionComp = stairwayToComponentManager().
                searchComponentByType<VisionComponent>(mVectNumEntity[i], Components_e::VISION_COMPONENT);
        moveComp = stairwayToComponentManager().
                searchComponentByType<MoveableComponent>(mVectNumEntity[i], Components_e::MOVEABLE_COMPONENT);
        mapCompA = stairwayToComponentManager().
                searchComponentByType<MapCoordComponent>(mVectNumEntity[i], Components_e::MAP_COORD_COMPONENT);
        assert(visionComp);
        assert(mapCompA);
        assert(moveComp);
        toRemove = 0;
        m_numVertexToDraw[i] = visionComp->m_vectVisibleEntities.size();
        m_entitiesNumMem.clear();
        for(uint32_t j = 0; j < m_numVertexToDraw[i]; ++j)
        {
            genCollComp = stairwayToComponentManager().
                    searchComponentByType<GeneralCollisionComponent>(visionComp->m_vectVisibleEntities[j],
                                                                     Components_e::GENERAL_COLLISION_COMPONENT);
            mapCompB = stairwayToComponentManager().
                    searchComponentByType<MapCoordComponent>(visionComp->m_vectVisibleEntities[j],
                                                             Components_e::MAP_COORD_COMPONENT);
            assert(mapCompB);
            assert(genCollComp);
            treatDisplayEntity(genCollComp, mapCompA, mapCompB, visionComp,
                               toRemove, moveComp->m_degreeOrientation, j);
        }
        m_numVertexToDraw[i] -= toRemove;
    }
}

//===================================================================
void FirstPersonDisplaySystem::treatDisplayEntity(GeneralCollisionComponent *genCollComp, MapCoordComponent *mapCompA,
                                                  MapCoordComponent *mapCompB, VisionComponent *visionComp,
                                                  uint32_t &toRemove, float observerAngle, uint32_t numIteration)
{
    float radiantObserverAngle = getRadiantAngle(observerAngle);
    uint32_t numEntity = visionComp->m_vectVisibleEntities[numIteration];
    if(genCollComp->m_tag == CollisionTag_e::WALL_CT)
    {
        SpriteTextureComponent *spriteComp = stairwayToComponentManager().
                searchComponentByType<SpriteTextureComponent>(
                    numEntity,
                    Components_e::SPRITE_TEXTURE_COMPONENT);
        assert(spriteComp);
        float depthGL[4];
        pairFloat_t absolPos[4];
        float lateralPos[3];
        bool pointIn[3];
        bool leftLimit[3];
        uint32_t angleToTreat;
        //calculate distance
        fillWallEntitiesData(numEntity, absolPos, depthGL,
                             mapCompA, mapCompB, radiantObserverAngle, pointIn,
                             leftLimit, angleToTreat);
        if(angleToTreat < 2)
        {
            return;
        }
        pairFloat_t result;
        //calculate all 3 display position
        for(uint32_t i = 1; i < angleToTreat; ++i)
        {
            result = getPairFPSLateralGLPosFromAngle(observerAngle,
                                                     mapCompA->m_absoluteMapPositionPX,
                                                     absolPos[i - 1], absolPos[i],
                                                     &pointIn[i - 1], &leftLimit[i - 1]);
            if(i == 1)
            {
                lateralPos[i - 1] = result.first;
                lateralPos[i] = result.second;
            }
            else
            {
                lateralPos[i] = result.second;
            }
        }
        //conf screen position
        confWallEntityVertex(numEntity, visionComp, lateralPos, depthGL, (angleToTreat == 3));
        float cameraDistance = (getCameraDistance(mapCompA->m_absoluteMapPositionPX,
                                                  absolPos[1], radiantObserverAngle) / LEVEL_TILE_SIZE_PX);
        fillVertexFromEntity(numEntity, numIteration, cameraDistance, true);
    }
    else
    {
        pairFloat_t centerPosB = getCenterPosition(mapCompB, genCollComp, numEntity);
        SpriteTextureComponent *spriteComp = stairwayToComponentManager().
                    searchComponentByType<SpriteTextureComponent>(numEntity, Components_e::SPRITE_TEXTURE_COMPONENT);
        assert(spriteComp);
        float distance = getCameraDistance(mapCompA->m_absoluteMapPositionPX,
                                           mapCompB->m_absoluteMapPositionPX, radiantObserverAngle) / LEVEL_TILE_SIZE_PX;
        float depthSimpleGL;
        if(distance > visionComp->m_distanceVisibility)
        {
            ++toRemove;
            return;
        }
        depthSimpleGL = spriteComp->m_glFpsSize.second / distance;
        float lateralPos = getFPSLateralGLPosFromAngle(observerAngle,
                                                       mapCompA->m_absoluteMapPositionPX, centerPosB);
        confNormalEntityVertex(numEntity, visionComp, lateralPos, depthSimpleGL);
        fillVertexFromEntity(numEntity, numIteration, distance);
    }
}

//===================================================================
float getFPSLateralGLPosFromAngle(float centerAngleVision, const pairFloat_t &observerPoint,
                                  const pairFloat_t &targetPoint)
{
    float trigoAngle = getTrigoAngle(observerPoint, targetPoint);
    float elementAngle = centerAngleVision - trigoAngle;
    // Trigo calculate TOA divide by camera distance ::
    // Op = (tan(angle) * cameraDist) / cameraDist == tan(angle)
    return std::tan(getRadiantAngle(elementAngle));
}

//===================================================================
pairFloat_t getPairFPSLateralGLPosFromAngle(float centerAngleVision, const pairFloat_t &observerPoint,
                                            const pairFloat_t &targetPointA, const pairFloat_t &targetPointB,
                                            bool pointIn[], bool outLeft[])
{
    float resultA = 0.0f, resultB = 0.0f, intersectA = 0.0f, intersectB = 0.0f, diffIntersect, absLateralPosInside;
    float trigoAngle;
    bool YIntersect = (static_cast<int>(targetPointA.first) == static_cast<int>(targetPointB.first));
    if(pointIn[0])
    {
        trigoAngle = getTrigoAngle(observerPoint, targetPointA);
        //get lateral pos from angle
        resultA = getLateralAngle(centerAngleVision, trigoAngle);
    }
    else
    {
        intersectA = getIntersectCoord(observerPoint, targetPointA,
                                       centerAngleVision, outLeft[0], YIntersect);
    }
    if(pointIn[1])
    {
        trigoAngle = getTrigoAngle(observerPoint, targetPointB);
        //get lateral pos from angle
        resultB = getLateralAngle(centerAngleVision, trigoAngle);
    }
    else
    {
        intersectB = getIntersectCoord(observerPoint, targetPointB,
                                       centerAngleVision, outLeft[1], YIntersect);
    }
    if(!pointIn[0] && pointIn[1])
    {
        if(YIntersect)
        {
            diffIntersect = std::abs(targetPointB.second - intersectA);
        }
        else
        {
            diffIntersect = std::abs(targetPointB.first - intersectA);
        }
        //calculate absolute diff position from inside position to left limit screen
        absLateralPosInside = std::abs(resultB + 1.0f);
        resultA = resultB - (LEVEL_TILE_SIZE_PX * absLateralPosInside) / diffIntersect;
    }
    else if(pointIn[0] && !pointIn[1])
    {
        if(YIntersect)
        {
            diffIntersect = std::abs(targetPointA.second - intersectB);
        }
        else
        {
            diffIntersect = std::abs(targetPointA.first - intersectB);
        }
        //calculate absolute diff position from inside position to left limit screen
        absLateralPosInside = std::abs(resultA - 1.0f);
        resultB = resultA + (LEVEL_TILE_SIZE_PX * absLateralPosInside) / diffIntersect;
    }
    else if(!pointIn[0] && !pointIn[1])
    {
        //TO TREAT
    }
    return {resultA, resultB};
}

//===================================================================
float getLateralAngle(float centerAngleVision, float trigoAngle)
{
    return std::tan(getRadiantAngle(centerAngleVision - trigoAngle));
}

//===================================================================
float getIntersectCoord(const pairFloat_t &observerPoint, const pairFloat_t &targetPointA,
                        float centerAngleVision, bool outLeft, bool YIntersect)
{
    float angle, adj, diffAngle;
    //X case
    if(!YIntersect)
    {
        //look up
        if(observerPoint.second > targetPointA.second)
        {
            angle = 90.0f;
        }
        //look down
        else
        {
            angle = 270.0f;
        }
        adj = std::abs(observerPoint.second - targetPointA.second);
    }
    //Y case
    else
    {
        //look left
        if(observerPoint.first > targetPointA.first)
        {
            angle = 180.0f;
        }
        //look right
        else
        {
            angle = 0.0f;
            if(std::abs(angle - centerAngleVision) > 180.0f)
            {
                centerAngleVision += 360.0f;
            }
        }
        adj = std::abs(observerPoint.first - targetPointA.first);
    }
    if(outLeft)
    {
        diffAngle = getRadiantAngle(angle - std::fmod(centerAngleVision + 45.0f, 360.0f));
    }
    else
    {
        diffAngle = getRadiantAngle(angle - std::fmod(centerAngleVision - 45.0f, 360.0f));
    }
    float diff;
    diff = (adj * std::tan(diffAngle));
    if(YIntersect)
    {
        return (observerPoint.second + diff);
    }
    else
    {
        return (observerPoint.first + diff);
    }
}

//===================================================================
void FirstPersonDisplaySystem::fillAbsolAndDistanceWall(pairFloat_t absolPos[], float distance[],
                                                        MapCoordComponent *mapCompCamera, MapCoordComponent *mapCompWall,
                                                        uint32_t numEntity, uint32_t &distanceToTreat)
{
    RectangleCollisionComponent *rectComp = stairwayToComponentManager().
            searchComponentByType<RectangleCollisionComponent>(numEntity, Components_e::RECTANGLE_COLLISION_COMPONENT);
    assert(rectComp);
    //get all points WALL rect position
    //up left
    absolPos[0] = mapCompWall->m_absoluteMapPositionPX;
    absolPos[1] = {mapCompWall->m_absoluteMapPositionPX.first + rectComp->m_size.first,
                   mapCompWall->m_absoluteMapPositionPX.second};
    absolPos[2] = {mapCompWall->m_absoluteMapPositionPX.first + rectComp->m_size.first,
                   mapCompWall->m_absoluteMapPositionPX.second + rectComp->m_size.second};
    absolPos[3] = {mapCompWall->m_absoluteMapPositionPX.first,
                   mapCompWall->m_absoluteMapPositionPX.second + rectComp->m_size.second};

    //up left
    distance[0] = getDistance(mapCompCamera->m_absoluteMapPositionPX, absolPos[0]);
    //up right
    distance[1] = getDistance(mapCompCamera->m_absoluteMapPositionPX, absolPos[1]);
    //down right
    distance[2] = getDistance(mapCompCamera->m_absoluteMapPositionPX, absolPos[2]);
    //down left
    distance[3] = getDistance(mapCompCamera->m_absoluteMapPositionPX, absolPos[3]);

    uint32_t minVal = getMinValueFromEntries(distance);
    //second display
    if(minVal != 1)
    {
        std::swap(distance[minVal], distance[1]);
        std::swap(absolPos[minVal], absolPos[1]);
    }
    uint32_t maxVal = getMaxValueFromEntries(distance);
    //do not display
    if(maxVal != 3)
    {
        std::swap(distance[3], distance[maxVal]);
        std::swap(absolPos[3], absolPos[maxVal]);
    }

    float trigoAngleA =  getTrigoAngle(mapCompCamera->m_absoluteMapPositionPX, absolPos[0]),
            trigoAngleB =  getTrigoAngle(mapCompCamera->m_absoluteMapPositionPX, absolPos[1]),
            trigoAngleC =  getTrigoAngle(mapCompCamera->m_absoluteMapPositionPX, absolPos[2]);
    treatLimitAngle(trigoAngleA, trigoAngleC);
    treatLimitAngle(trigoAngleA, trigoAngleB);
    if(trigoAngleA < trigoAngleC)
    {
        std::swap(distance[0], distance[2]);
        std::swap(absolPos[0], absolPos[2]);
    }
    distanceToTreat = 3;
    if((trigoAngleA < trigoAngleB && trigoAngleB < trigoAngleC) ||
            (trigoAngleA > trigoAngleB && trigoAngleB > trigoAngleC))
    {
        return;
    }
    if(trigoAngleC >= trigoAngleB)
    {
        --distanceToTreat;
    }
    if(trigoAngleB >= trigoAngleA)
    {
        std::swap(distance[0], distance[1]);
        std::swap(absolPos[0], absolPos[1]);
        std::swap(distance[1], distance[2]);
        std::swap(absolPos[1], absolPos[2]);
        --distanceToTreat;
    }
}

//===================================================================
void treatLimitAngle(float &degreeAngleA, float &degreeAngleB)
{
    if(std::abs(degreeAngleA - degreeAngleB) > 90.0f)
    {
        if(degreeAngleB < degreeAngleA)
        {
            degreeAngleB += 360.0f;
        }
        else
        {
            degreeAngleA += 360.0f;
        }
    }
}

//===================================================================
void FirstPersonDisplaySystem::fillWallEntitiesData(uint32_t numEntity, pairFloat_t absolPos[], float depthGL[],
                                                    MapCoordComponent *mapCompCamera, MapCoordComponent *mapCompWall,
                                                    float observerAngle,
                                                    bool pointIn[], bool outLeft[], uint32_t &angleToTreat)
{
    float distanceReal[4];
    fillAbsolAndDistanceWall(absolPos, distanceReal, mapCompCamera, mapCompWall, numEntity, angleToTreat);
    if(angleToTreat < 2)
    {
        return;
    }
    float pointAngleVision[3];
    float anglePoint;
    for(uint32_t i = 0; i < 3; ++i)
    {
        anglePoint = getTrigoAngle(mapCompCamera->m_absoluteMapPositionPX, absolPos[i], false);
        if(std::abs(anglePoint - observerAngle) > PI)
        {
            if(anglePoint < observerAngle)
            {
                observerAngle -= PI_DOUBLE;
            }
            else
            {
                observerAngle += PI_DOUBLE;
            }
        }
        pointAngleVision[i] = anglePoint - observerAngle;
        pointIn[i] = std::abs(pointAngleVision[i]) < PI_QUARTER;

        //mem limit left or right
        if(!pointIn[i])
        {
            outLeft[i] = (anglePoint > observerAngle);
        }
    }
    SpriteTextureComponent *spriteComp = stairwayToComponentManager().
                searchComponentByType<SpriteTextureComponent>(numEntity, Components_e::SPRITE_TEXTURE_COMPONENT);
    assert(spriteComp);
    std::optional<uint32_t> j;
    for(uint32_t i = 0; i < angleToTreat; ++i)
    {
        if(!pointIn[i])
        {
            if((angleToTreat != 2 && i != 1 && !pointIn[1]) ||
                    (i == 2 && outLeft[2]) || (i == 0 && !outLeft[0]))
            {
                continue;
            }
        }
        //standard case
//        if(pointIn[i])
        {
            depthGL[i] = spriteComp->m_glFpsSize.second /
                    (getCameraDistance(mapCompCamera->m_absoluteMapPositionPX,
                                       absolPos[i], observerAngle) / LEVEL_TILE_SIZE_PX);
        }
        //out of screen limit case
//        else
//        {
//            if((angleToTreat != 2 && i != 1 && !pointIn[1]) ||
//                    (i == 2 && outLeft[2]) || (i == 0 && !outLeft[0]))
//            {
//                continue;
//            }
//            j = getLimitIndex(pointIn, distanceReal, i);
//            if(!j)
//            {
//                return;
//            }
//            depthGL[i] = calculateDepthGLAngleWallLimitDisplay(absolPos[i], absolPos[*j],
//                                                    observerAngle, mapCompCamera, spriteComp);
//        }
    }
}

//===================================================================
float calculateDepthGLAngleWallLimitDisplay(const pairFloat_t &outPoint, const pairFloat_t &linkPoint,
                                            float observerAngle, MapCoordComponent *mapCompCamera,
                                            SpriteTextureComponent *spriteComp)
{
    pairFloat_t limitPoint = linkPoint;
    //X case
    if(std::abs(outPoint.first - linkPoint.first) > EPSILON_FLOAT)
    {
        //if out > link
        limitPoint.first = (outPoint.first > linkPoint.first) ?
                    limitPoint.first + 1.0f : limitPoint.first - 1.0f;
    }
    //Y case
    else
    {
        //if out > link
        limitPoint.second = (outPoint.second > linkPoint.second) ?
                    limitPoint.second + 1.0f : limitPoint.second - 1.0f;
    }
    float lateralPosLink = getFPSLateralGLPosFromAngle(observerAngle, mapCompCamera->m_absoluteMapPositionPX, linkPoint);
    float lateralPosGLXLink = lateralPosLink / DOUBLE_CONE_VISION;
    float lateralPosLimit = getFPSLateralGLPosFromAngle(observerAngle, mapCompCamera->m_absoluteMapPositionPX, limitPoint);
    float lateralPosGLXLimit = lateralPosLimit / DOUBLE_CONE_VISION;
    float lateralPosOut = getFPSLateralGLPosFromAngle(observerAngle, mapCompCamera->m_absoluteMapPositionPX, outPoint);
    float lateralPosGLXOut = lateralPosOut / DOUBLE_CONE_VISION;
    float diffLateralPosTotal = std::abs(lateralPosGLXOut - lateralPosGLXLink);
    float diffLateralPosLimit = std::abs(lateralPosGLXLimit - lateralPosGLXLink);
    float distanceLink = spriteComp->m_glFpsSize.second /
            (getCameraDistance(mapCompCamera->m_absoluteMapPositionPX,
                               linkPoint, observerAngle) / LEVEL_TILE_SIZE_PX);
    //get Link point camera distance
    float distanceLimit = spriteComp->m_glFpsSize.second /
            (getCameraDistance(mapCompCamera->m_absoluteMapPositionPX,
                               limitPoint, observerAngle) / LEVEL_TILE_SIZE_PX);
    float diffDist = distanceLimit - distanceLink;
    diffDist = (diffDist * diffLateralPosTotal / diffLateralPosLimit);
    return (distanceLink + diffDist);
}

//===================================================================
std::optional<uint32_t> getLimitIndex(const bool pointIn[], const float distanceReal[], uint32_t i)
{
    if(i == 0 || i == 2)
    {
        return 1;
    }
    else
    {
        if(pointIn[0] && !pointIn[2])
        {
            return 0;
        }
        else if(!pointIn[0] && pointIn[2])
        {
            return 2;
        }
        else if(pointIn[0] && pointIn[2])
        {
            return (distanceReal[0] < distanceReal[2]) ? 0 : 2;
        }
        else
        {
            return {};
        }
    }
}

//===================================================================
std::optional<pairFloat_t> FirstPersonDisplaySystem::checkLimitWallCase(const pairFloat_t &pointObserver,
                                                                        float limitObserverAngle,
                                                                        const pairFloat_t &outPoint,
                                                                        const pairFloat_t &linkPoint,
                                                                        bool leftLimit, bool XCase,
                                                                        float correction, pairFloat_t &pointReturn)
{
    if(XCase)
    {
        //check if observer is between 2 wall points
        if((pointObserver.first > outPoint.first && pointObserver.first < linkPoint.first)
                || (pointObserver.first > linkPoint.first && pointObserver.first < outPoint.first))
        {

            if((leftLimit && limitObserverAngle > 0.0f && limitObserverAngle < 90.0f)
                    || (!leftLimit && limitObserverAngle > 270.0f && limitObserverAngle < 360.0f))

            {
                correction += std::abs(outPoint.first - pointObserver.first);
                pointReturn.first = outPoint.first + correction;
                return pointReturn;
            }
            else if((leftLimit && limitObserverAngle > 180.0f && limitObserverAngle < 270.0f)
                    || (!leftLimit && limitObserverAngle > 90.0f && limitObserverAngle < 180.0f))

            {
                correction += std::abs(outPoint.first - pointObserver.first);
                pointReturn.first = outPoint.first - correction;
                return pointReturn;
            }
        }
    }
    else
    {
        //check if observer is between 2 wall points
        if((pointObserver.second > outPoint.second && pointObserver.second < linkPoint.second)
                || (pointObserver.second > linkPoint.second && pointObserver.second < outPoint.second))
        {
            if((leftLimit && limitObserverAngle > 270.0f && limitObserverAngle < 360.0f)
                    || (!leftLimit && limitObserverAngle > 180.0f && limitObserverAngle < 270.0f))

            {
                correction += std::abs(outPoint.second - pointObserver.second);
                pointReturn.second = outPoint.second + correction;
                return pointReturn;
            }
            else if((leftLimit && limitObserverAngle > 90.0f && limitObserverAngle < 180.0f)
                    || (!leftLimit && limitObserverAngle > 0.0f && limitObserverAngle < 90.0f))

            {
                correction += std::abs(outPoint.second - pointObserver.second);
                pointReturn.second = outPoint.second - correction;
                return pointReturn;
            }
        }
    }
    return {};
}

//===================================================================
float getQuarterAngle(float angle)
{
    if(angle < 89.0f)
    {
        return angle;
    }
    else if(angle < 269.0f)
    {
        return std::abs(angle - 180.0f);
    }
    else
    {
        return std::abs(angle - 360.0f);
    }
}

//===================================================================
void FirstPersonDisplaySystem::fillVertexFromEntity(uint32_t numEntity, uint32_t numIteration,
                                                    float distance, bool wallTag)
{
    Shader_e shaderType = Shader_e::TEXTURE_S;
    //use 1 vertex for 1 sprite for beginning
    if(numIteration < m_vectVerticesData.size())
    {
        m_vectVerticesData[numIteration].clear();
        m_vectVerticesData[numIteration].setShaderType(shaderType);
    }
    else
    {
        do
        {
            m_vectVerticesData.push_back(VerticesData(shaderType));
        }
        while(numIteration >= m_vectVerticesData.size());
        assert(numIteration < m_vectVerticesData.size());
    }
    PositionVertexComponent *posComp = stairwayToComponentManager().
            searchComponentByType<PositionVertexComponent>(numEntity,
                                                           Components_e::POSITION_VERTEX_COMPONENT);
    SpriteTextureComponent *spriteComp = stairwayToComponentManager().
            searchComponentByType<SpriteTextureComponent>(numEntity,
                                                          Components_e::SPRITE_TEXTURE_COMPONENT);
    assert(posComp);
    assert(spriteComp);
    m_entitiesNumMem.insert(EntityData(distance,
                                       static_cast<Texture_e>(spriteComp->m_spriteData->m_textureNum),
                                       numIteration));
    if(!wallTag)
    {
        m_vectVerticesData[numIteration].
                loadVertexStandartTextureComponent(*posComp, *spriteComp);
    }
    else
    {
        m_vectVerticesData[numIteration].
                loadVertexTextureDrawByLineComponent(*posComp, *spriteComp,
                                                     m_textureLineDrawNumber);
    }
}

//===================================================================
void FirstPersonDisplaySystem::setVectTextures(std::vector<Texture> &vectTexture)
{
    m_ptrVectTexture = &vectTexture;
    m_vectVerticesData.reserve(50);
}

//===================================================================
void FirstPersonDisplaySystem::confNormalEntityVertex(uint32_t numEntity, VisionComponent *visionComp,
                                                      float lateralPosDegree, float depthGL)
{
    PositionVertexComponent *positionComp = stairwayToComponentManager().
            searchComponentByType<PositionVertexComponent>(numEntity, Components_e::POSITION_VERTEX_COMPONENT);
    SpriteTextureComponent *spriteComp = stairwayToComponentManager().
                searchComponentByType<SpriteTextureComponent>(numEntity, Components_e::SPRITE_TEXTURE_COMPONENT);
    assert(spriteComp);
    assert(positionComp);
    assert(visionComp);
    positionComp->m_vertex.resize(4);
    //convert to GL context
    float lateralPosGL = lateralPosDegree;
    //quickfix
    if(depthGL > 1.5f)
    {
        depthGL = 1.5f;
    }
    float halfLateralSize = depthGL / spriteComp->m_glFpsSize.first / 2.0f;
    float halfVerticalSize = depthGL / spriteComp->m_glFpsSize.second / 2.0f;
    positionComp->m_vertex[0].first = lateralPosGL - halfLateralSize;
    positionComp->m_vertex[0].second = halfVerticalSize;
    positionComp->m_vertex[1].first = lateralPosGL + halfLateralSize;
    positionComp->m_vertex[1].second = halfVerticalSize;
    positionComp->m_vertex[2].first = lateralPosGL + halfLateralSize;
    positionComp->m_vertex[2].second = -halfVerticalSize;
    positionComp->m_vertex[3].first = lateralPosGL - halfLateralSize;
    positionComp->m_vertex[3].second = -halfVerticalSize;
}


//===================================================================
void FirstPersonDisplaySystem::confWallEntityVertex(uint32_t numEntity, VisionComponent *visionComp,
                                                    float lateralPosDegree[], float depthGL[],
                                                    bool wallAllVisible)
{
    PositionVertexComponent *positionComp = stairwayToComponentManager().
            searchComponentByType<PositionVertexComponent>(numEntity, Components_e::POSITION_VERTEX_COMPONENT);
    assert(positionComp);
    assert(visionComp);
    bool excludeZero, excludeTwo;
    if(wallAllVisible)
    {
        excludeZero = (lateralPosDegree[0] > lateralPosDegree[1]) &&
                ((lateralPosDegree[0] < lateralPosDegree[2]) || (lateralPosDegree[0] < lateralPosDegree[1])) &&
                (lateralPosDegree[0] > lateralPosDegree[2]);
        excludeTwo = (lateralPosDegree[2] > lateralPosDegree[0]) &&
                ((lateralPosDegree[2] < lateralPosDegree[1]) || (lateralPosDegree[2] < lateralPosDegree[0])) &&
                (lateralPosDegree[2] > lateralPosDegree[1]);
        wallAllVisible = !(excludeZero || excludeTwo || (lateralPosDegree[1] > lateralPosDegree[2]));
    }
    if(!wallAllVisible)
    {
        positionComp->m_vertex.resize(4);
    }
    else
    {
        positionComp->m_vertex.resize(6);
    }
    //convert to GL context
    float halfVerticalSize = depthGL[0] / 2.0f;
    float lateralPosGL = lateralPosDegree[0];
    //convert to GL context
    float halfVerticalSizeMid = depthGL[1] / 2.0f;
    float lateralPosGLMid = lateralPosDegree[1];

    positionComp->m_vertex[0].first = lateralPosGL;
    positionComp->m_vertex[0].second = halfVerticalSize;
    positionComp->m_vertex[1].first = lateralPosGLMid;
    positionComp->m_vertex[1].second = halfVerticalSizeMid;

    positionComp->m_vertex[2].first = lateralPosGLMid;
    positionComp->m_vertex[2].second = -halfVerticalSizeMid;
    positionComp->m_vertex[3].first = lateralPosGL;
    positionComp->m_vertex[3].second = -halfVerticalSize;
    if(!wallAllVisible)
    {
        return;
    }
    float halfVerticalSizeMax = depthGL[2] / 2.0f;
    float lateralPosMaxGL = lateralPosDegree[2];
    positionComp->m_vertex[4].first = lateralPosMaxGL;
    positionComp->m_vertex[4].second = halfVerticalSizeMax;
    positionComp->m_vertex[5].first = lateralPosMaxGL;
    positionComp->m_vertex[5].second = -halfVerticalSizeMax;
}

//===================================================================
void FirstPersonDisplaySystem::drawVertex()
{
    m_shader->use();
    //DONT WORK for multiple player
    for(uint32_t i = 0; i < mVectNumEntity.size(); ++i)
    {
        uint32_t numEntity;
        for(std::multiset<EntityData>::const_iterator it = m_entitiesNumMem.begin();it != m_entitiesNumMem.end(); ++it)
        {
            numEntity = it->m_entityNum;
            assert(numEntity < m_vectVerticesData.size());
            m_ptrVectTexture->operator[](static_cast<uint32_t>(it->m_textureNum)).bind();
            m_vectVerticesData[numEntity].confVertexBuffer();
            m_vectVerticesData[numEntity].drawElement();
        }
    }
}

//===================================================================
void FirstPersonDisplaySystem::setShader(Shader &shader)
{
    m_shader = &shader;
}

//===================================================================
pairFloat_t FirstPersonDisplaySystem::getCenterPosition(const MapCoordComponent *mapComp,
                                                        GeneralCollisionComponent *genCollComp, float numEntity)
{
    assert(mapComp);
    assert(genCollComp);
    switch (genCollComp->m_shape)
    {
    case CollisionShape_e::CIRCLE_C:
    case CollisionShape_e::SEGMENT_C://TMP
        break;
    case CollisionShape_e::RECTANGLE_C:
        RectangleCollisionComponent *rectCollComp = stairwayToComponentManager().
                searchComponentByType<RectangleCollisionComponent>(numEntity, Components_e::RECTANGLE_COLLISION_COMPONENT);
        assert(rectCollComp);
        return {mapComp->m_absoluteMapPositionPX.first + rectCollComp->m_size.first / 2,
                    mapComp->m_absoluteMapPositionPX.second + rectCollComp->m_size.second / 2};
    }
    return mapComp->m_absoluteMapPositionPX;
}



uint32_t getMinValueFromEntries(const float distance[])
{
    uint32_t val = 0;
    val = (distance[0] < distance[1]) ? 0 : 1;
    val = (distance[val] < distance[2]) ? val : 2;
    val = (distance[val] < distance[3]) ? val : 3;
    return val;
}

uint32_t getMaxValueFromEntries(const float distance[])
{
    uint32_t val = 0;
    val = (distance[0] > distance[1]) ? 0 : 1;
    val = (distance[val] > distance[2]) ? val : 2;
    val = (distance[val] > distance[3]) ? val : 3;
    return val;
}
