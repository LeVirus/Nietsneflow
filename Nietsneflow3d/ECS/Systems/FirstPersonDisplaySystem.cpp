#include "FirstPersonDisplaySystem.hpp"
#include "Level.hpp"
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
#include <ECS/Components/DoorComponent.hpp>
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
        //TEST back camera
        pairFloat_t memPreviousCameraPos = mapCompA->m_absoluteMapPositionPX;
        {
            float radiantAngle = getRadiantAngle(moveComp->m_degreeOrientation + 180.0f);
            mapCompA->m_absoluteMapPositionPX.first +=
                    std::cos(radiantAngle) * 19.0f;
            mapCompA->m_absoluteMapPositionPX.second -=
                    std::sin(radiantAngle) * 19.0f;
        }
        uint32_t numIteration;
        //draw dynamic element
        for(numIteration = 0; numIteration < m_numVertexToDraw[i]; ++numIteration)
        {
            genCollComp = stairwayToComponentManager().
                    searchComponentByType<GeneralCollisionComponent>(visionComp->m_vectVisibleEntities[numIteration],
                                                                     Components_e::GENERAL_COLLISION_COMPONENT);
            mapCompB = stairwayToComponentManager().
                    searchComponentByType<MapCoordComponent>(visionComp->m_vectVisibleEntities[numIteration],
                                                             Components_e::MAP_COORD_COMPONENT);
            assert(mapCompB);
            assert(genCollComp);
            treatDisplayEntity(genCollComp, mapCompA, mapCompB, visionComp,
                               toRemove, moveComp->m_degreeOrientation, numIteration);
        }
        mapCompA->m_absoluteMapPositionPX = memPreviousCameraPos;
        m_numVertexToDraw[i] -= toRemove;
        ++numIteration;
        rayCasting();
        //draw wall and door
        for(mapRayCastingData_t::const_iterator it = m_raycastingData.begin();
            it != m_raycastingData.end(); ++it, ++numIteration)
        {
            writeVertexRaycasting(*it, numIteration);
        }
    }
}

//===================================================================
void FirstPersonDisplaySystem::writeVertexRaycasting(const pairRaycastingData_t &entityData,
                                                     uint32_t numIteration)
{
    VerticesData &vertex = getClearedVertice(numIteration);
    SpriteTextureComponent *spriteComp = stairwayToComponentManager().
            searchComponentByType<SpriteTextureComponent>(entityData.first,
                                                          Components_e::SPRITE_TEXTURE_COMPONENT);
    assert(spriteComp);
    float distance = vertex.loadRaycastingEntity(*spriteComp, entityData.second, m_textureLineDrawNumber);
    m_entitiesNumMem.insert(EntityData(distance,
                                       static_cast<Texture_e>(spriteComp->m_spriteData->m_textureNum),
                                       numIteration));
}


//===================================================================
void FirstPersonDisplaySystem::treatDisplayEntity(GeneralCollisionComponent *genCollComp, MapCoordComponent *mapCompA,
                                                  MapCoordComponent *mapCompB, VisionComponent *visionComp,
                                                  uint32_t &toRemove, float degreeObserverAngle, uint32_t numIteration)
{
    float radiantObserverAngle = getRadiantAngle(degreeObserverAngle);
    uint32_t numEntity = visionComp->m_vectVisibleEntities[numIteration];
    SpriteTextureComponent *spriteComp = stairwayToComponentManager().
            searchComponentByType<SpriteTextureComponent>(
                numEntity,
                Components_e::SPRITE_TEXTURE_COMPONENT);
    assert(spriteComp);
    pairFloat_t centerPosB = getCenterPosition(mapCompB, genCollComp, numEntity);
    assert(spriteComp);
    float distance = getCameraDistance(mapCompA->m_absoluteMapPositionPX,
                                       mapCompB->m_absoluteMapPositionPX, radiantObserverAngle) / LEVEL_TILE_SIZE_PX;
    float simpleDistance = getDistance(mapCompA->m_absoluteMapPositionPX,
                                       mapCompB->m_absoluteMapPositionPX);
    float depthSimpleGL;
    if(distance > visionComp->m_distanceVisibility)
    {
        ++toRemove;
        return;
    }
    depthSimpleGL = spriteComp->m_glFpsSize.second / distance;
    float trigoAngle = getTrigoAngle(mapCompA->m_absoluteMapPositionPX, centerPosB);
    //get lateral pos from angle
    float lateralPos = getLateralAngle(degreeObserverAngle, trigoAngle);
    confNormalEntityVertex(numEntity, visionComp, lateralPos, depthSimpleGL);
    fillVertexFromEntity(numEntity, numIteration, simpleDistance, DisplayMode_e::STANDART_DM);
}

//===================================================================
float getDoorDistance(const MapCoordComponent *mapCompCamera, const MapCoordComponent *mapCompDoor,
                      const DoorComponent *doorComp)
{
    pairFloat_t refPointA = getAbsolutePosition(mapCompDoor->m_coord);
    float latPosDoor = refPointA.first,
            vertPosDoor = refPointA.second,
            latPosCamera = mapCompCamera->m_absoluteMapPositionPX.first,
            vertPosCamera = mapCompCamera->m_absoluteMapPositionPX.second;
    if(doorComp->m_vertical)
    {
        //RIGHT
        if(latPosCamera > latPosDoor + LEVEL_TILE_SIZE_PX)
        {
            refPointA.first += LEVEL_TILE_SIZE_PX;
            //UP
            if(vertPosCamera < refPointA.second)
            {
                refPointA.second -= LEVEL_TILE_SIZE_PX;
            }
            //DOWN
            else if(vertPosCamera > refPointA.second + LEVEL_TILE_SIZE_PX)
            {
                refPointA.second += LEVEL_TILE_SIZE_PX;
            }
            //MID
            else
            {
                return getMiddleDoorDistance(mapCompCamera->m_absoluteMapPositionPX,
                                             refPointA, doorComp->m_vertical);
            }
            return getDistance(mapCompCamera->m_absoluteMapPositionPX, refPointA) + 0.1f;
        }
        //LEFT
        else if(latPosCamera < latPosDoor)
        {
            //DOWN
            if(vertPosCamera > refPointA.second + LEVEL_TILE_SIZE_PX)
            {
                refPointA.second += LEVEL_TILE_SIZE_PX * 2.0f;
            }
            //MID
            else if(!(vertPosCamera < refPointA.second))
            {
                return getMiddleDoorDistance(mapCompCamera->m_absoluteMapPositionPX,
                                             refPointA, doorComp->m_vertical);
            }
            return getDistance(mapCompCamera->m_absoluteMapPositionPX, refPointA) + 0.1f;
        }
    }
    //LATERAL
    else
    {
        //UP
        if(vertPosCamera < vertPosDoor)
        {
            //LEFT
            if(latPosCamera < latPosDoor)
            {
                refPointA.first -= LEVEL_TILE_SIZE_PX;
            }
            //RIGHT
            else if(latPosCamera > latPosDoor + LEVEL_TILE_SIZE_PX)
            {
                refPointA.first += LEVEL_TILE_SIZE_PX;
            }
            //MID
            else
            {
                return getMiddleDoorDistance(mapCompCamera->m_absoluteMapPositionPX,
                                             refPointA, doorComp->m_vertical);
            }
            return getDistance(mapCompCamera->m_absoluteMapPositionPX, refPointA) + 0.1f;
        }
        //DOWN
        else if(vertPosCamera > vertPosDoor + LEVEL_TILE_SIZE_PX)
        {
            refPointA.second += LEVEL_TILE_SIZE_PX;
            //RIGHT
            if(latPosCamera > latPosDoor + LEVEL_TILE_SIZE_PX)
            {
                refPointA.first += LEVEL_TILE_SIZE_PX * 2.0f;
            }
            //MID
            else if(!(latPosCamera < latPosDoor))
            {
                return getMiddleDoorDistance(mapCompCamera->m_absoluteMapPositionPX,
                                             refPointA, doorComp->m_vertical);
            }
            return getDistance(mapCompCamera->m_absoluteMapPositionPX, refPointA) + 0.1f;
        }
    }
    return getDistance(mapCompCamera->m_absoluteMapPositionPX, refPointA);
}

//===================================================================
float getMiddleDoorDistance(const pairFloat_t &camera, const pairFloat_t &element, bool vertical)
{
    pairFloat_t refPointB = element;
    if(vertical)
    {
        refPointB.second += LEVEL_TILE_SIZE_PX;
    }
    else
    {
        refPointB.first += LEVEL_TILE_SIZE_PX;
    }
    float distA = getDistance(camera, element),
    distB = getDistance(camera, refPointB);
    return std::min(distA, distB) - 0.1f;
}

//===================================================================
void FirstPersonDisplaySystem::adaptTextureDoorDisplay(DoorComponent *doorComp,
                                                       RectangleCollisionComponent *rectComp,
                                                       MapCoordComponent *mapCompCamera,
                                                       MapCoordComponent *mapCompDoor, pairFloat_t absolPos[])
{


    //first rect is vertical?
    doorComp->m_verticalPosDisplay.first = (absolPos[0].first == absolPos[1].first);
    //second rect is vertical?
    doorComp->m_verticalPosDisplay.second = (absolPos[1].first == absolPos[2].first);

    if(doorComp->m_currentState == DoorState_e::STATIC_CLOSED ||
            doorComp->m_currentState == DoorState_e::STATIC_OPEN)
    {
        doorComp->m_boundActive = false;
        return;
    }

    if(doorComp->m_vertical)
    {
        //left
        if(mapCompCamera->m_absoluteMapPositionPX.first < mapCompDoor->m_absoluteMapPositionPX.first)
        {
            doorComp->m_spriteLateralBound.first = 1.0f - rectComp->m_size.second / LEVEL_TILE_SIZE_PX;
            doorComp->m_spriteLateralBound.second = 1.0f;
        }
        //right
        else
        {
            doorComp->m_spriteLateralBound.second = rectComp->m_size.second / LEVEL_TILE_SIZE_PX;
            doorComp->m_spriteLateralBound.first = 0.0f;
        }
    }
    else
    {
        //up
        if(mapCompCamera->m_absoluteMapPositionPX.second < mapCompDoor->m_absoluteMapPositionPX.second)
        {
            doorComp->m_spriteLateralBound.first = 0.0f;
            doorComp->m_spriteLateralBound.second = rectComp->m_size.first / LEVEL_TILE_SIZE_PX;
        }
        //down
        else
        {
            doorComp->m_spriteLateralBound.first = 1.0f - rectComp->m_size.first / LEVEL_TILE_SIZE_PX;
            doorComp->m_spriteLateralBound.second = 1.0f;
        }
    }
    doorComp->m_boundActive = true;
}

//===================================================================
void FirstPersonDisplaySystem::calculateDepthWallEntitiesData(uint32_t numEntity, uint32_t angleToTreat,
                                                              const bool pointIn[], const bool outLeft[], float depthGL[],
                                                              float radiantObserverAngle, const pairFloat_t absolPos[],
                                                              const MapCoordComponent *mapCompCamera)
{
    SpriteTextureComponent *spriteComp = stairwayToComponentManager().
                searchComponentByType<SpriteTextureComponent>(numEntity, Components_e::SPRITE_TEXTURE_COMPONENT);
    assert(spriteComp);
    bool YIntersect = false;
    int outPoint;
    int inPoint = -1;
    float degreeObserverAngle = getDegreeAngle(radiantObserverAngle);
    pairFloat_t intersectPoint;
    for(uint32_t i = 1; i < angleToTreat; ++i)
    {
        outPoint = -1;
        if(!pointIn[i])
        {
            outPoint = i;
            inPoint = i - 1;
        }
        else if(!pointIn[i - 1])
        {
            outPoint = i - 1;
            inPoint = i;
        }
        if(outPoint != -1)
        {
            YIntersect = (static_cast<int>(absolPos[i - 1].first) ==
                          static_cast<int>(absolPos[i].first));
            intersectPoint = getIntersectCoord(mapCompCamera->m_absoluteMapPositionPX, absolPos[outPoint],
                                               degreeObserverAngle, outLeft[outPoint], YIntersect);
            break;
        }
    }

    for(uint32_t i = 0; i < angleToTreat; ++i)
    {
        //standard case
        if(pointIn[i])
        {
            depthGL[i] = spriteComp->m_glFpsSize.second /
                    (getCameraDistance(mapCompCamera->m_absoluteMapPositionPX,
                                       absolPos[i], radiantObserverAngle) / LEVEL_TILE_SIZE_PX);
        }
        //out of screen limit case
        else
        {
            if((angleToTreat != 2 && i != 1 && !pointIn[1]) ||
                    (i == 2 && outLeft[2]) || (i == 0 && !outLeft[0]))
            {
                continue;
            }
            bool increment = false;
            if(outLeft[i])
            {
                depthGL[inPoint] = spriteComp->m_glFpsSize.second /
                        (getCameraDistance(mapCompCamera->m_absoluteMapPositionPX,
                                           absolPos[inPoint], radiantObserverAngle) / LEVEL_TILE_SIZE_PX);
                increment = true;
            }
            float diffDist, diffCamDist,
            distCamIntersect = spriteComp->m_glFpsSize.second /
                    (getCameraDistance(mapCompCamera->m_absoluteMapPositionPX,
                                       intersectPoint, radiantObserverAngle) / LEVEL_TILE_SIZE_PX);
            diffCamDist = std::abs(distCamIntersect - depthGL[inPoint]);
            if(YIntersect)
            {
                diffDist = std::abs(intersectPoint.second - absolPos[inPoint].second);
            }
            else
            {
                diffDist = std::abs(intersectPoint.first - absolPos[inPoint].first);
            }
            float correctDepth = (LEVEL_TILE_SIZE_PX * diffCamDist) / diffDist;
            if(depthGL[inPoint] < distCamIntersect)
            {
                depthGL[i] = depthGL[inPoint] + correctDepth;
            }
            else
            {
                depthGL[i] = depthGL[inPoint] - correctDepth;
            }
            if(increment)
            {
                ++i;
            }
        }
    }
}

//===================================================================
float getLateralAngle(float centerAngleVision, float trigoAngle)
{
    float result = centerAngleVision - trigoAngle;
    if(result > 180.0f)
    {
        result -= 360.0f;
    }
    if(result < -180.0f)
    {
        result += 360.0f;
    }
    return result / HALF_CONE_VISION;
}

//===================================================================
pairFloat_t getIntersectCoord(const pairFloat_t &observerPoint, const pairFloat_t &targetPoint,
                              float centerAngleVision, bool outLeft, bool YIntersect)
{
    float angle, adj, diffAngle;
    //X case
    if(!YIntersect)
    {
        //look up
        if(observerPoint.second > targetPoint.second)
        {
            angle = 90.0f;
        }
        //look down
        else
        {
            angle = 270.0f;
        }
        adj = observerPoint.second - targetPoint.second;
    }
    //Y case
    else
    {
        //look left
        if(observerPoint.first > targetPoint.first)
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
        adj = observerPoint.first - targetPoint.first;
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
        return {targetPoint.first, (observerPoint.second - diff)};
    }
    else
    {
        return {(observerPoint.first + diff), targetPoint.second};
    }
}

//===================================================================
void FirstPersonDisplaySystem::fillAbsolAndDistanceWall(pairFloat_t absolPos[], float distance[], VisionComponent *visionComp,
                                                        MapCoordComponent *mapCompCamera, MapCoordComponent *mapCompWall,
                                                        uint32_t numEntity, uint32_t &distanceToTreat, float degreeObserverAngle)
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
        if(!angleWallVisible(mapCompCamera->m_absoluteMapPositionPX, absolPos[2],
                             visionComp->m_vectVisibleEntities, numEntity))
        {
            --distanceToTreat;
        }
        if(!angleWallVisible(mapCompCamera->m_absoluteMapPositionPX, absolPos[0],
                             visionComp->m_vectVisibleEntities, numEntity))
        {
            removeSecondRect(absolPos, distance, distanceToTreat);
        }
        treatLimitAngle(trigoAngleB, degreeObserverAngle);
        float currentElementAngle = degreeObserverAngle - trigoAngleB;
        if(currentElementAngle < -45.0f)
        {
            removeSecondRect(absolPos, distance, distanceToTreat);
        }
        else if(currentElementAngle > 45.0f)
        {
            --distanceToTreat;
        }
        return;
    }
    if(trigoAngleC >= trigoAngleB)
    {
        --distanceToTreat;
    }
    else if(trigoAngleB >= trigoAngleA)
    {
        removeSecondRect(absolPos, distance, distanceToTreat);
    }
}

//===================================================================
bool FirstPersonDisplaySystem::angleWallVisible(const pairFloat_t &observerPoint, const pairFloat_t &angleWall,
                                                const std::vector<uint32_t> &vectEntities, uint32_t numEntity)
{
    RectangleCollisionComponent *rectComp;
    MapCoordComponent *mapCompA, *mapCompB;
    GeneralCollisionComponent *genCollComp;
    for(uint32_t i = 0; i < vectEntities.size(); ++i)
    {
        if(vectEntities[i] == numEntity)
        {
            continue;
        }
        genCollComp = stairwayToComponentManager().
                searchComponentByType<GeneralCollisionComponent>(vectEntities[i], Components_e::GENERAL_COLLISION_COMPONENT);
        assert(genCollComp);
        if(genCollComp->m_tag != CollisionTag_e::WALL_CT)
        {
            continue;
        }
        mapCompA = stairwayToComponentManager().
                searchComponentByType<MapCoordComponent>(vectEntities[i], Components_e::MAP_COORD_COMPONENT);
        mapCompB = stairwayToComponentManager().
                searchComponentByType<MapCoordComponent>(numEntity, Components_e::MAP_COORD_COMPONENT);
        rectComp = stairwayToComponentManager().
                searchComponentByType<RectangleCollisionComponent>(vectEntities[i],
                                                                   Components_e::RECTANGLE_COLLISION_COMPONENT);
        assert(mapCompA);
        assert(mapCompB);
        assert(rectComp);
        if(getDistance(mapCompA->m_absoluteMapPositionPX, mapCompB->m_absoluteMapPositionPX) <= LEVEL_TILE_SIZE_PX)
        {
            if(checkSegmentRectCollision(observerPoint, angleWall,
                                         mapCompA->m_absoluteMapPositionPX, rectComp->m_size))
            {
                return false;
            }
        }
    }
    return true;
}

//===================================================================
void removeSecondRect(pairFloat_t absolPos[], float distance[], uint32_t &distanceToTreat)
{
    std::swap(distance[0], distance[1]);
    std::swap(absolPos[0], absolPos[1]);
    std::swap(distance[1], distance[2]);
    std::swap(absolPos[1], absolPos[2]);
    --distanceToTreat;
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
void FirstPersonDisplaySystem::fillWallEntitiesData(uint32_t numEntity, pairFloat_t absolPos[],
                                                    VisionComponent *visionComp,
                                                    MapCoordComponent *mapCompCamera, MapCoordComponent *mapCompWall,
                                                    float radiantObserverAngle,
                                                    bool pointIn[], bool outLeft[], uint32_t &angleToTreat)
{
    float distanceReal[4];
    fillAbsolAndDistanceWall(absolPos, distanceReal, visionComp, mapCompCamera, mapCompWall, numEntity,
                             angleToTreat, getDegreeAngle(radiantObserverAngle));
    if(angleToTreat < 2)
    {
        return;
    }
    float pointAngleVision[3];
    float anglePoint;
    for(uint32_t i = 0; i < 3; ++i)
    {
        anglePoint = getTrigoAngle(mapCompCamera->m_absoluteMapPositionPX, absolPos[i], false);
        if(std::abs(anglePoint - radiantObserverAngle) > PI)
        {
            if(anglePoint < radiantObserverAngle)
            {
                radiantObserverAngle -= PI_DOUBLE;
            }
            else
            {
                radiantObserverAngle += PI_DOUBLE;
            }
        }
        pointAngleVision[i] = anglePoint - radiantObserverAngle;
        pointIn[i] = std::abs(pointAngleVision[i]) < PI_QUARTER;
        //mem limit left or right
        if(!pointIn[i])
        {
            outLeft[i] = (anglePoint > radiantObserverAngle);
        }
    }
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
                                                    float distance, DisplayMode_e displayMode)
{
    VerticesData &vertex = getClearedVertice(numIteration);
    PositionVertexComponent *posComp = stairwayToComponentManager().
            searchComponentByType<PositionVertexComponent>(numEntity,
                                                           Components_e::POSITION_VERTEX_COMPONENT);
    SpriteTextureComponent *spriteComp = stairwayToComponentManager().
            searchComponentByType<SpriteTextureComponent>(numEntity,
                                                          Components_e::SPRITE_TEXTURE_COMPONENT);
    assert(posComp);
    assert(spriteComp);
    m_entitiesNumMem.insert(EntityData(distance, static_cast<Texture_e>(spriteComp->m_spriteData->m_textureNum),
                                       numIteration));
    if(displayMode == DisplayMode_e::STANDART_DM)
    {
        vertex.loadVertexStandartTextureComponent(*posComp, *spriteComp);
    }
    else
    {
        DoorComponent *doorComp = nullptr;
        doorComp = stairwayToComponentManager().
                searchComponentByType<DoorComponent>(numEntity, Components_e::DOOR_COMPONENT);
        vertex.loadVertexTextureDrawByLineComponent(*posComp, *spriteComp, m_textureLineDrawNumber, doorComp);
    }
}

//===================================================================
VerticesData &FirstPersonDisplaySystem::getClearedVertice(uint32_t index)
{
    //use 1 vertex for 1 sprite for beginning
    if(index < m_vectVerticesData.size())
    {
        m_vectVerticesData[index].clear();
        m_vectVerticesData[index].setShaderType(Shader_e::TEXTURE_S);
    }
    else
    {
        do
        {
            m_vectVerticesData.push_back(VerticesData(Shader_e::TEXTURE_S));
        }
        while(index >= m_vectVerticesData.size());
        assert(index < m_vectVerticesData.size());
    }
    return m_vectVerticesData[index];
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
        uint32_t numIteration;
        for(std::multiset<EntityData>::const_iterator it = m_entitiesNumMem.begin();
            it != m_entitiesNumMem.end(); ++it)
        {
            numIteration = it->m_iterationNum;
            assert(numIteration < m_vectVerticesData.size());
            m_ptrVectTexture->operator[](static_cast<uint32_t>(it->m_textureNum)).bind();
            m_vectVerticesData[numIteration].confVertexBuffer();
            m_vectVerticesData[numIteration].drawElement();
        }
    }
}

//===================================================================
void FirstPersonDisplaySystem::setShader(Shader &shader)
{
    m_shader = &shader;
}

//===================================================================
void FirstPersonDisplaySystem::rayCasting()
{
    MapCoordComponent *mapCompCamera;
    MoveableComponent *moveComp;
    std::optional<float> lateralLeadCoef, verticalLeadCoef;
    float radiantAngle, textPos;
    pairFloat_t currentPoint;
    std::optional<pairUI_t> currentCoord;
    std::optional<ElementRaycast> element;
    bool lateral, textLateral;
    for(uint32_t i = 0; i < mVectNumEntity.size(); ++i)
    {
        //WORK FOR ONE PLAYER ONLY
        m_raycastingData.clear();
        mapCompCamera = stairwayToComponentManager().
                searchComponentByType<MapCoordComponent>(mVectNumEntity[i], Components_e::MAP_COORD_COMPONENT);
        assert(mapCompCamera);
        moveComp = stairwayToComponentManager().
                searchComponentByType<MoveableComponent>(mVectNumEntity[i], Components_e::MOVEABLE_COMPONENT);
        assert(moveComp);
        float leftAngle = moveComp->m_degreeOrientation + HALF_CONE_VISION;
        float currentAngle = leftAngle, currentLateralScreen = -1.0f;
        float cameraRadiantAngle = getRadiantAngle(moveComp->m_degreeOrientation);
        pairFloat_t point;
        //mem entity num & distances
        for(uint32_t j = 0; j < m_textureLineDrawNumber; ++j)
        {
            radiantAngle = getRadiantAngle(currentAngle);
            verticalLeadCoef = getLeadCoef(radiantAngle, false);
            lateralLeadCoef = getLeadCoef(radiantAngle, true);
            currentPoint = mapCompCamera->m_absoluteMapPositionPX;
            for(uint32_t k = 0; k < 20; ++k)//limit distance
            {
                currentPoint = getLimitPointRayCasting(currentPoint, radiantAngle,
                                                       lateralLeadCoef, verticalLeadCoef, lateral);
                point = currentPoint;
                //treat limit angle cube case
                if(!treatLimitIntersect(point, lateral))
                {
                    if(lateral && std::sin(radiantAngle) > 0.0f)
                    {
                        --point.second;
                    }
                    else if(!lateral && std::cos(radiantAngle) < 0.0f)
                    {
                        --point.first;
                    }
                }
                currentCoord = getLevelCoord(point);
                if(!currentCoord)
                {
                    break;
                }
                element = Level::getElementCase(*currentCoord);
                if(element)
                {
                    if(element->m_type == LevelCaseType_e::WALL_LC)
                    {
                        textPos = lateral ? currentPoint.first : currentPoint.second;
                        memDistance(element->m_numEntity, j, getCameraDistance(mapCompCamera->m_absoluteMapPositionPX,
                                                                               currentPoint, cameraRadiantAngle), textPos);
                        break;
                    }
                    else if(element->m_type == LevelCaseType_e::DOOR_LC)
                    {
                        if(treatDoorRaycast(element->m_numEntity, radiantAngle,
                                            currentPoint, *currentCoord, lateral,
                                            lateralLeadCoef, verticalLeadCoef, textLateral))
                        {
                            //TMP TEST================
                            textPos = textLateral ? std::fmod(currentPoint.first, LEVEL_TILE_SIZE_PX) :
                                                    std::fmod(currentPoint.second, LEVEL_TILE_SIZE_PX);
                            memDistance(element->m_numEntity, j,
                                        getCameraDistance(mapCompCamera->m_absoluteMapPositionPX,
                                                          currentPoint, cameraRadiantAngle), textPos);
                            break;
                        }
                    }
                }
            }
            currentLateralScreen += m_stepDrawLateralScreen;
            currentAngle -= m_stepAngle;
            if(currentAngle < 0.0f)
            {
                currentAngle += 360.0f;
            }
        }
    }
}

//===================================================================
bool FirstPersonDisplaySystem::treatDoorRaycast(uint32_t numEntity, uint32_t radiantAngle,
                                                pairFloat_t &currentPoint,
                                                const pairUI_t &coord, bool lateral,
                                                std::optional<float> lateralLeadCoef,
                                                std::optional<float> verticalLeadCoef,
                                                bool &textLateral)
{
    DoorComponent *doorComp = stairwayToComponentManager().
            searchComponentByType<DoorComponent>(numEntity, Components_e::DOOR_COMPONENT);
    assert(doorComp);
    RectangleCollisionComponent *rectComp = stairwayToComponentManager().
            searchComponentByType<RectangleCollisionComponent>(numEntity, Components_e::RECTANGLE_COLLISION_COMPONENT);
    assert(rectComp);
    MapCoordComponent *mapComp = stairwayToComponentManager().
            searchComponentByType<MapCoordComponent>(numEntity, Components_e::MAP_COORD_COMPONENT);
    assert(mapComp);
    pairFloat_t doorPos[4] = {mapComp->m_absoluteMapPositionPX,
                              {mapComp->m_absoluteMapPositionPX.first + rectComp->m_size.first,
                               mapComp->m_absoluteMapPositionPX.second},
                             {mapComp->m_absoluteMapPositionPX.first + rectComp->m_size.first,
                              mapComp->m_absoluteMapPositionPX.second + rectComp->m_size.second},
                             {mapComp->m_absoluteMapPositionPX.first + rectComp->m_size.first,
                              mapComp->m_absoluteMapPositionPX.second}};
    if(!doorComp->m_boundActive)
    {
        if(doorComp->m_vertical)
        {
            return treatVerticalDoor(radiantAngle, lateral, currentPoint,
                                     doorPos, verticalLeadCoef, lateralLeadCoef,
                                     coord, textLateral);
        }
        else if(!doorComp->m_vertical)
        {
            return treatLateralDoor(radiantAngle, lateral, currentPoint,
                                    doorPos, lateralLeadCoef, verticalLeadCoef,
                                    coord, textLateral);
        }
    }
    return false;//TMP
}

//===================================================================
bool treatVerticalDoor(float radiantAngle, bool lateral, pairFloat_t &currentPoint,
                       pairFloat_t doorPos[], std::optional<float> verticalLeadCoef,
                       std::optional<float> lateralLeadCoef, const pairUI_t &coord, bool &textLateral)
{
    float diffLat, diffVert;
    pairFloat_t tmpPos = currentPoint;
    bool leftCase = (currentPoint.first < doorPos[0].first);
    if(lateral)
    {
        //exclude case
        if((std::cos(radiantAngle) < 0.0f && currentPoint.first < doorPos[0].first ) ||
                (std::cos(radiantAngle) > 0.0f && currentPoint.first > doorPos[1].first))
        {
            return false;
        }
        else if(currentPoint.first >= doorPos[0].first &&
                currentPoint.first <= doorPos[1].first)
        {
            textLateral = true;
            return true;
        }
        else if(!verticalLeadCoef)
        {
            return false;
        }
        else
        {
            if(leftCase)
            {
                diffLat = doorPos[0].first - currentPoint.first;
            }
            else
            {
                diffLat = doorPos[1].first - currentPoint.first;
            }
            diffVert = *verticalLeadCoef * std::abs(diffLat) / LEVEL_TILE_SIZE_PX;
            tmpPos.first += diffLat;
            tmpPos.second += diffVert;
            if(static_cast<uint32_t>(tmpPos.second / LEVEL_TILE_SIZE_PX) == coord.second)
            {
                textLateral = false;
                currentPoint = tmpPos;
                return true;
            }
            return false;
        }
    }
    //vertical
    else
    {
        textLateral = false;
        if(!lateralLeadCoef)
        {
            currentPoint.first = leftCase ? doorPos[0].first : doorPos[1].first;
            return true;
        }
        diffLat = (std::cos(radiantAngle) < 0.0f) ? -DOOR_CASE_POS_PX : DOOR_CASE_POS_PX;
        diffVert = *verticalLeadCoef * std::abs(diffLat) / LEVEL_TILE_SIZE_PX;
        tmpPos.first += diffLat;
        tmpPos.second += diffVert;
        if(static_cast<uint32_t>(tmpPos.second / LEVEL_TILE_SIZE_PX) == coord.second)
        {
            currentPoint = tmpPos;
            return true;
        }
        return false;
    }
}

//===================================================================
bool treatLateralDoor(float radiantAngle, bool lateral, pairFloat_t &currentPoint,
                      pairFloat_t doorPos[], std::optional<float> lateralLeadCoef,
                      std::optional<float> verticalLeadCoef, const pairUI_t &coord, bool &textLateral)
{
    float diffLat, diffVert;
    pairFloat_t tmpPos = currentPoint;
    bool upCase = (currentPoint.second < doorPos[0].second);
    if(!lateral)
    {
        //exclude case
        if((std::sin(radiantAngle) > 0.0f && currentPoint.second < doorPos[0].second ) ||
                (std::sin(radiantAngle) < 0.0f && currentPoint.second > doorPos[2].second))
        {
            return false;
        }
        else if(currentPoint.second >= doorPos[0].second &&
                currentPoint.second <= doorPos[2].second)
        {
            textLateral = false;
            return true;
        }
        else if (!lateralLeadCoef)
        {
            return false;
        }
        else
        {
            if(upCase)
            {
                diffVert = doorPos[0].second - currentPoint.second;
            }
            else
            {
                diffVert = doorPos[2].second - currentPoint.second;
            }
            diffLat = *lateralLeadCoef * std::abs(diffVert) / LEVEL_TILE_SIZE_PX;
            tmpPos.first += diffLat;
            tmpPos.second += diffVert;
            if(static_cast<uint32_t>(tmpPos.second / LEVEL_TILE_SIZE_PX) == coord.second)
            {
                textLateral = true;
                currentPoint = tmpPos;
                return true;
            }
            return false;
        }
    }
    //lateral
    else
    {
        textLateral = true;
        if(!verticalLeadCoef)
        {
            currentPoint.second = upCase ? doorPos[0].second : doorPos[2].second;
            return true;
        }
        diffVert = (std::sin(radiantAngle) > 0.0f) ? -DOOR_CASE_POS_PX : DOOR_CASE_POS_PX;
        diffLat = *lateralLeadCoef * std::abs(diffVert) / LEVEL_TILE_SIZE_PX;
        tmpPos.first += diffLat;
        tmpPos.second += diffVert;
        if(static_cast<uint32_t>(tmpPos.first / LEVEL_TILE_SIZE_PX) == coord.first)
        {
            currentPoint = tmpPos;
            return true;
        }
        return false;
    }
}

//===================================================================
bool treatLimitIntersect(pairFloat_t &point, bool lateral)
{
    std::optional<pairUI_t> currentCoord;
    if(std::fmod(point.second, LEVEL_TILE_SIZE_PX) <= 0.01f &&
            std::fmod(point.first, LEVEL_TILE_SIZE_PX) <= 0.01f)
    {
        if(lateral)
        {
            currentCoord = getLevelCoord({point.first - 1.0f, point.second});
            if(currentCoord && (*Level::getElementCase(*currentCoord)).m_type ==
                    LevelCaseType_e::WALL_LC)
            {
                --point.first;
                return true;
            }
        }
        else if(!lateral)
        {
            currentCoord = getLevelCoord({point.first, point.second - 1.0f});
            if(currentCoord && (*Level::getElementCase(*currentCoord)).m_type ==
                    LevelCaseType_e::WALL_LC)
            {
                --point.second;
                return true;
            }
        }
        currentCoord = getLevelCoord({point.first - 1.0f, point.second - 1.0f});
        if(currentCoord && (*Level::getElementCase(*currentCoord)).m_type ==
                LevelCaseType_e::WALL_LC)
        {
            --point.first;
            --point.second;
            return true;
        }
    }
    return false;
}

//===================================================================
void FirstPersonDisplaySystem::memDistance(uint32_t numEntity, uint32_t lateralScreenPos,
                                           float distance, float texturePos)
{
    mapRayCastingData_t::iterator it = m_raycastingData.find(numEntity);
    if(it == m_raycastingData.end())
    {
        m_raycastingData.insert({numEntity, {{distance, texturePos, lateralScreenPos}}});
    }
    else
    {
        it->second.push_back({distance, texturePos, lateralScreenPos});
    }
}

//===================================================================
std::optional<float> getModulo(float sinCosAngle, float position, float modulo, bool lateral)
{
    float result = std::fmod(position, modulo);
    if(result <= EPSILON_FLOAT)
    {
        //first cos second sin
        if((lateral && sinCosAngle < 0.0f) || (!lateral && sinCosAngle > 0.0f))
        {
            return {};
        }
    }
    return result;
}

//===================================================================
pairFloat_t getLimitPointRayCasting(const pairFloat_t &cameraPoint, float radiantAngle,
                                    std::optional<float> lateralLeadCoef,
                                    std::optional<float> verticalLeadCoef, bool &lateral)
{
    float currentCos = std::cos(radiantAngle),
            currentSin = std::sin(radiantAngle);
    std::optional<float> modulo;
    pairFloat_t prevLimitPoint = cameraPoint;
    int32_t coordX;
    if(currentCos < 0.0f)
    {
        coordX = getCoord(prevLimitPoint.first, LEVEL_TILE_SIZE_PX);
    }
    else
    {
        coordX = static_cast<int32_t>(prevLimitPoint.first / LEVEL_TILE_SIZE_PX);
    }
    //limit case
    if(!verticalLeadCoef)
    {
        modulo = getModulo(currentSin, cameraPoint.second, LEVEL_TILE_SIZE_PX, false);
        if(!modulo)
        {
            modulo = LEVEL_TILE_SIZE_PX;
        }
        //down
        if(currentSin < 0.0f)
        {
            return {cameraPoint.first, cameraPoint.second + (LEVEL_TILE_SIZE_PX - *modulo)};
        }
        //up
        else
        {
            return {cameraPoint.first, cameraPoint.second - *modulo};
        }
    }
    //second limit case
    else if(!lateralLeadCoef)
    {
        modulo = getModulo(currentCos, cameraPoint.first, LEVEL_TILE_SIZE_PX, true);
        if(!modulo)
        {
            modulo = LEVEL_TILE_SIZE_PX;
        }
        //left
        if(currentCos < 0.0f)
        {
            return {cameraPoint.first - *modulo, cameraPoint.second};
        }
        //right
        else
        {
            return {cameraPoint.first + (LEVEL_TILE_SIZE_PX - *modulo), cameraPoint.second};
        }
    }
    float diffVert, diffLat;
    //check if lateral diff is out of case=================================
    modulo = getModulo(currentSin, cameraPoint.second, LEVEL_TILE_SIZE_PX, false);
    if(!modulo)
    {
        diffVert = LEVEL_TILE_SIZE_PX;
        if(currentSin > 0.0f)
        {
            diffVert = -diffVert;
        }
    }
    else
    {
        diffVert = (currentSin < 0.0f) ? LEVEL_TILE_SIZE_PX - *modulo : -(*modulo);
    }
    diffLat = *lateralLeadCoef * std::abs(diffVert) / LEVEL_TILE_SIZE_PX;
    //if lateral diff is in the same case
    prevLimitPoint.first += diffLat;
    if(static_cast<int32_t>(prevLimitPoint.first / LEVEL_TILE_SIZE_PX) == coordX)
    {
        prevLimitPoint.second += diffVert;
        lateral = true;
        return prevLimitPoint;
    }
    prevLimitPoint = cameraPoint;
    //check if vertical diff is out of case=================================
    modulo = getModulo(currentCos, cameraPoint.first, LEVEL_TILE_SIZE_PX, true);
    if(!modulo)
    {
        diffLat = (currentCos < 0.0f) ? -LEVEL_TILE_SIZE_PX : LEVEL_TILE_SIZE_PX;
    }
    else
    {
        diffLat = (currentCos < 0.0f) ? -(*modulo) : (LEVEL_TILE_SIZE_PX - *modulo);
    }
    diffVert = *verticalLeadCoef * std::abs(diffLat) / LEVEL_TILE_SIZE_PX;
    prevLimitPoint.first += diffLat;
    prevLimitPoint.second += diffVert;
    lateral = false;
    return prevLimitPoint;
}

//===================================================================
int32_t getCoord(float value, float tileSize)
{
    uint32_t coord = static_cast<uint32_t>(value / tileSize);
    if(std::fmod(value, tileSize) < 0.01f)
    {
        --coord;
    }
    return coord;
}

//===================================================================
std::optional<float> getLeadCoef(float radiantAngle, bool lateral)
{
    float tanRadiantAngleQuarter = std::tan(std::fmod(radiantAngle, PI_HALF)),
            sinus = std::sin(radiantAngle),
            cosinus = std::cos(radiantAngle), result;
    if((lateral && std::abs(sinus) < 0.0001f) || (!lateral && std::abs(cosinus) < 0.0001f))
    {
        return {};
    }
    bool sinusPos = (sinus > 0.0f),
    cosinusPos = (cosinus > 0.0f);
    if((lateral && sinusPos == cosinusPos) || (!lateral && cosinusPos != sinusPos))
    {
        result = LEVEL_TILE_SIZE_PX / tanRadiantAngleQuarter;
    }
    else
    {
        result = tanRadiantAngleQuarter * LEVEL_TILE_SIZE_PX;
    }
    if((lateral && cosinus < 0.0f) || (!lateral && sinus > 0.0f))
    {
        result *= -1.0f;
    }
    return result;
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
