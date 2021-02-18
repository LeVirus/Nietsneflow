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
                                                  uint32_t &toRemove, float degreeObserverAngle, uint32_t numIteration)
{
    float radiantObserverAngle = getRadiantAngle(degreeObserverAngle);
    uint32_t numEntity = visionComp->m_vectVisibleEntities[numIteration];
    SpriteTextureComponent *spriteComp = stairwayToComponentManager().
            searchComponentByType<SpriteTextureComponent>(
                numEntity,
                Components_e::SPRITE_TEXTURE_COMPONENT);
    assert(spriteComp);
    if(genCollComp->m_tag == CollisionTag_e::WALL_CT || genCollComp->m_tag == CollisionTag_e::DOOR_CT)
    {
        float depthGL[4];
        pairFloat_t absolPos[4];
        float lateralPos[3];
        bool pointIn[3];
        bool outLeft[3];
        float trigoAngle;
        uint32_t angleToTreat;
        DisplayMode_e displayMode;
        float distance;
        //calculate distance
        fillWallEntitiesData(numEntity, absolPos, visionComp, mapCompA, mapCompB,
                             radiantObserverAngle, pointIn, outLeft, angleToTreat);
        if(angleToTreat < 2)
        {
            return;
        }
        if(genCollComp->m_tag == CollisionTag_e::DOOR_CT)
        {
            DoorComponent *doorComp = stairwayToComponentManager().
                    searchComponentByType<DoorComponent>(numEntity, Components_e::DOOR_COMPONENT);
            assert(doorComp);
            RectangleCollisionComponent *rectComp = stairwayToComponentManager().
                    searchComponentByType<RectangleCollisionComponent>(numEntity,
                                                                       Components_e::RECTANGLE_COLLISION_COMPONENT);
            assert(rectComp);
            adaptTextureDoorDisplay(doorComp, rectComp, mapCompA, mapCompB, absolPos);
            distance = getDoorDistance(mapCompA, mapCompB, doorComp);
            displayMode = DisplayMode_e::DOOR_DM;
        }
        else
        {
            displayMode = DisplayMode_e::WALL_DM;
        }
        //calculate all 3 display position
        for(uint32_t i = 0; i < angleToTreat; ++i)
        {
            trigoAngle = getTrigoAngle(mapCompA->m_absoluteMapPositionPX, absolPos[i]);
            lateralPos[i] = getLateralAngle(degreeObserverAngle, trigoAngle);
        }
        calculateDepthWallEntitiesData(numEntity, angleToTreat, pointIn, outLeft, depthGL,
                                       radiantObserverAngle, absolPos, mapCompA);
        //conf screen position
        confWallEntityVertex(numEntity, visionComp, lateralPos, depthGL, (angleToTreat == 3));
        if(displayMode != DisplayMode_e::DOOR_DM)
        {
            distance = (getDistance(mapCompA->m_absoluteMapPositionPX, absolPos[1]));
        }
        fillVertexFromEntity(numEntity, numIteration, distance, displayMode);
    }
    else
    {
        pairFloat_t centerPosB = getCenterPosition(mapCompB, genCollComp, numEntity);
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
        float trigoAngle = getTrigoAngle(mapCompA->m_absoluteMapPositionPX, centerPosB);
        //get lateral pos from angle
        float lateralPos = getLateralAngle(degreeObserverAngle, trigoAngle);
        confNormalEntityVertex(numEntity, visionComp, lateralPos, depthSimpleGL);
        fillVertexFromEntity(numEntity, numIteration, distance, DisplayMode_e::STANDART_DM);
    }
}

//===================================================================
float getDoorDistance(const MapCoordComponent *mapCompCamera, const MapCoordComponent *mapCompDoor,
                      const DoorComponent *doorComp)
{
    float latPosDoor = mapCompDoor->m_absoluteMapPositionPX.first,
            vertPosDoor = mapCompDoor->m_absoluteMapPositionPX.second,
            latPosCamera = mapCompCamera->m_absoluteMapPositionPX.first,
            vertPosCamera = mapCompCamera->m_absoluteMapPositionPX.second;
    pairFloat_t absolPosDoor = getAbsolutePosition(mapCompDoor->m_coord);
    if(doorComp->m_vertical)
    {
        absolPosDoor.second += LEVEL_HALF_TILE_SIZE_PX;
        //RIGHT
        if(latPosCamera > latPosDoor)
        {
            absolPosDoor.first += LEVEL_TILE_SIZE_PX;
        }
    }
    else
    {
        absolPosDoor.first += LEVEL_HALF_TILE_SIZE_PX;
        //DOWN
        if(vertPosCamera > vertPosDoor)
        {
            absolPosDoor.second += LEVEL_TILE_SIZE_PX;
        }
    }
    return getDistance(mapCompCamera->m_absoluteMapPositionPX, absolPosDoor);
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
    if(displayMode == DisplayMode_e::STANDART_DM)
    {
        m_vectVerticesData[numIteration].
                loadVertexStandartTextureComponent(*posComp, *spriteComp);
    }
    else
    {
        DoorComponent *doorComp = nullptr;
        doorComp = stairwayToComponentManager().
                searchComponentByType<DoorComponent>(numEntity, Components_e::DOOR_COMPONENT);
        m_vectVerticesData[numIteration].
                loadVertexTextureDrawByLineComponent(*posComp, *spriteComp,
                                                     m_textureLineDrawNumber, doorComp);
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
