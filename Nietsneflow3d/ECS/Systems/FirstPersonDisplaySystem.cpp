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
#include <ECS/Components/FPSVisibleStaticElementComponent.hpp>
#include <ECS/Components/CircleCollisionComponent.hpp>
#include <ECS/Components/DoorComponent.hpp>
#include <ECS/Components/PlayerConfComponent.hpp>
#include <ECS/Components/ImpactShotComponent.hpp>
#include <ECS/Components/ShotConfComponent.hpp>
#include <ECS/Components/MapCoordComponent.hpp>
#include <ECS/Components/MemFPSGLSizeComponent.hpp>
#include <ECS/Components/EnemyConfComponent.hpp>
#include <ECS/Systems/ColorDisplaySystem.hpp>
#include <PictureData.hpp>
#include <cmath>
#include <chrono>

//===================================================================
FirstPersonDisplaySystem::FirstPersonDisplaySystem() :
    m_groundTiledTextVertice(Shader_e::COLORED_TEXTURE_S),
    m_ceilingTiledVertice(Shader_e::COLORED_TEXTURE_S)
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
    drawPlayerColorEffects();
}

//===================================================================
void FirstPersonDisplaySystem::drawPlayerColorEffects()
{
    for(uint32_t i = 0; i < mVectNumEntity.size(); ++i)
    {
        PlayerConfComponent *playerComp = stairwayToComponentManager().
                searchComponentByType<PlayerConfComponent>(mVectNumEntity[i],
                                                           Components_e::PLAYER_CONF_COMPONENT);
        assert(playerComp);
        if(playerComp->m_takeDamage)
        {
            mptrSystemManager->searchSystemByType<ColorDisplaySystem>(
                        static_cast<uint32_t>(Systems_e::COLOR_DISPLAY_SYSTEM))->drawVisibleDamage();
            playerComp->m_takeDamage = false;
        }
        if(playerComp->m_pickItem)
        {
            mptrSystemManager->searchSystemByType<ColorDisplaySystem>(
                        static_cast<uint32_t>(Systems_e::COLOR_DISPLAY_SYSTEM))->drawVisiblePickUpObject();
            playerComp->m_pickItem = false;
        }
        if(playerComp->m_insideWall)
        {
            mptrSystemManager->searchSystemByType<ColorDisplaySystem>(
                        static_cast<uint32_t>(Systems_e::COLOR_DISPLAY_SYSTEM))->drawScratchWall();
        }
    }
}

//===================================================================
void FirstPersonDisplaySystem::confCompVertexMemEntities()
{
    m_numVertexToDraw.resize(mVectNumEntity.size());
    //treat one player
    uint32_t toRemove = 0;
    VisionComponent *visionComp;
    MoveableComponent *moveComp;
    MapCoordComponent *mapCompA;
    GeneralCollisionComponent *genCollComp;
    MapCoordComponent *mapCompB;
    uint32_t numIteration;
    for(uint32_t i = 0; i < mVectNumEntity.size(); ++i)
    {
        numIteration = 0;
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
        m_memDoorDistance.clear();
        //if scratch continue
        if(rayCasting(mVectNumEntity[i]))
        {
            continue;
        }
        if(m_groundCeilingSimpleTextureActive)
        {
            confSimpleTextVertexGroundCeiling(moveComp->m_degreeOrientation);
            writeSimpleTextVertexGroundCeiling();
        }
        m_memWallEntityDistances.clear();
        //draw wall and door
        for(MapRayCastingData_t::const_iterator it = m_raycastingData.begin(); it != m_raycastingData.end(); ++it, ++numIteration)
        {
            writeVertexWallDoorRaycasting(*it, numIteration);
        }
        ++numIteration;
        //Draw simple element
        for(uint32_t j = 0; j < m_numVertexToDraw[i]; ++j, ++numIteration)
        {
            genCollComp = stairwayToComponentManager().
                    searchComponentByType<GeneralCollisionComponent>(visionComp->m_vectVisibleEntities[j],
                                                                     Components_e::GENERAL_COLLISION_COMPONENT);
            mapCompB = stairwayToComponentManager().
                    searchComponentByType<MapCoordComponent>(visionComp->m_vectVisibleEntities[j], Components_e::MAP_COORD_COMPONENT);
            assert(mapCompB);
            assert(genCollComp);
            if(genCollComp->m_active)
            {
                treatDisplayEntity(genCollComp, mapCompA, mapCompB, visionComp, toRemove, moveComp->m_degreeOrientation, numIteration, j);
            }
        }
        //draw dynamic element
        m_numVertexToDraw[i] -= toRemove;
    }
}

//===================================================================
void FirstPersonDisplaySystem::confSimpleTextVertexGroundCeiling(float observerAngle)
{
    float midPos = std::fmod(observerAngle, 90.0f) / 45.0f - 1.0f, leftPos = midPos - 2.0f, rightPos = midPos + 2.0f;
    if(m_groundSimpleTextBackground)
    {
        PositionVertexComponent *posComp = stairwayToComponentManager().
                searchComponentByType<PositionVertexComponent>(*m_groundSimpleTextBackground, Components_e::POSITION_VERTEX_COMPONENT);
        assert(posComp);
        posComp->m_vertex[0].first = leftPos;
        posComp->m_vertex[3].first = leftPos;
        posComp->m_vertex[1].first = midPos;
        posComp->m_vertex[2].first = midPos;
        posComp->m_vertex[4].first = rightPos;
        posComp->m_vertex[5].first = rightPos;
    }
    if(m_ceilingSimpleTextBackground)
    {
        PositionVertexComponent *posComp = stairwayToComponentManager().
                searchComponentByType<PositionVertexComponent>(*m_ceilingSimpleTextBackground, Components_e::POSITION_VERTEX_COMPONENT);
        assert(posComp);
        posComp->m_vertex[0].first = leftPos;
        posComp->m_vertex[3].first = leftPos;
        posComp->m_vertex[1].first = midPos;
        posComp->m_vertex[2].first = midPos;
        posComp->m_vertex[4].first = rightPos;
        posComp->m_vertex[5].first = rightPos;
    }
}

//===================================================================
void FirstPersonDisplaySystem::writeSimpleTextVertexGroundCeiling()
{
    if(m_groundSimpleTextBackground)
    {
        PositionVertexComponent *posComp = stairwayToComponentManager().
                searchComponentByType<PositionVertexComponent>(*m_groundSimpleTextBackground, Components_e::POSITION_VERTEX_COMPONENT);
        assert(posComp);
        SpriteTextureComponent *spriteComp = stairwayToComponentManager().
                searchComponentByType<SpriteTextureComponent>(*m_groundSimpleTextBackground, Components_e::SPRITE_TEXTURE_COMPONENT);
        assert(spriteComp);
        m_groundSimpleTextVertice.clear();
        m_groundSimpleTextVertice.loadVertexStandartTextureComponent(*posComp, *spriteComp);
    }
    if(m_ceilingSimpleTextBackground)
    {
        PositionVertexComponent *posComp = stairwayToComponentManager().
                searchComponentByType<PositionVertexComponent>(*m_ceilingSimpleTextBackground, Components_e::POSITION_VERTEX_COMPONENT);
        assert(posComp);
        SpriteTextureComponent *spriteComp = stairwayToComponentManager().
                searchComponentByType<SpriteTextureComponent>(*m_ceilingSimpleTextBackground, Components_e::SPRITE_TEXTURE_COMPONENT);
        assert(spriteComp);
        m_ceilingSimpleVertice.clear();
        m_ceilingSimpleVertice.loadVertexStandartTextureComponent(*posComp, *spriteComp);
    }
}

//===================================================================
void FirstPersonDisplaySystem::writeVertexWallDoorRaycasting(const pairRaycastingData_t &entityData, uint32_t numIteration)
{
    VerticesData &vertex = getClearedVertice(numIteration);
    SpriteTextureComponent *spriteComp = stairwayToComponentManager().
            searchComponentByType<SpriteTextureComponent>(entityData.first,
                                                          Components_e::SPRITE_TEXTURE_COMPONENT);
    assert(spriteComp);
    float distance = vertex.loadRaycastingEntity(*spriteComp, entityData.second);
    m_memWallEntityDistances.insert({entityData.first, distance});
    DoorComponent *doorComp = stairwayToComponentManager().searchComponentByType<DoorComponent>(entityData.first, Components_e::DOOR_COMPONENT);
    if(doorComp)
    {
        m_memDoorDistance.insert({entityData.first, distance});
    }
    m_entitiesNumMem.insert(EntityData(distance, spriteComp->m_spriteData->m_textureNum, numIteration));
}


//===================================================================
void FirstPersonDisplaySystem::treatDisplayEntity(GeneralCollisionComponent *genCollComp, MapCoordComponent *mapCompA,
                                                  MapCoordComponent *mapCompB, VisionComponent *visionComp,
                                                  uint32_t &toRemove, float degreeObserverAngle, uint32_t numIteration, uint32_t currentNormal)
{
    uint32_t numEntity = visionComp->m_vectVisibleEntities[currentNormal];
    assert(visionComp->m_vectVisibleEntities.size() > currentNormal);
    PairFloat_t centerPosB = getCenterPosition(mapCompB, genCollComp, numEntity);
    float radiantObserverAngle = getRadiantAngle(degreeObserverAngle),
            cameraDistance = getCameraDistance(mapCompA->m_absoluteMapPositionPX, mapCompB->m_absoluteMapPositionPX, radiantObserverAngle);
    float displayDistance = cameraDistance;
    if(cameraDistance > visionComp->m_distanceVisibility)
    {
        ++toRemove;
        return;
    }
    float trigoAngle = getTrigoAngle(mapCompA->m_absoluteMapPositionPX, centerPosB);
    //get lateral pos from angle
    float lateralPos = getLateralAngle(degreeObserverAngle, trigoAngle);
    //quickfix
    if(genCollComp->m_tagA != CollisionTag_e::ENEMY_CT &&
            genCollComp->m_tagA != CollisionTag_e::BULLET_ENEMY_CT &&
            genCollComp->m_tagA != CollisionTag_e::BULLET_PLAYER_CT &&
            genCollComp->m_tagA != CollisionTag_e::IMPACT_CT &&
            genCollComp->m_tagA != CollisionTag_e::STATIC_SET_CT && cameraDistance < 15.0f)
    {
        return;
    }
    confNormalEntityVertex(numEntity, visionComp, genCollComp->m_tagA, lateralPos, cameraDistance);
    fillVertexFromEntity(numEntity, numIteration, displayDistance);
}

//===================================================================
bool FirstPersonDisplaySystem::elementBehindDoor(const ElementRaycast &elementCase, float radiantObserverAngle, const MapCoordComponent *mapComp)
{
    if(m_memDoorDistance.find(elementCase.m_numEntity) != m_memDoorDistance.end())
    {
        DoorComponent *doorComp = stairwayToComponentManager().
                searchComponentByType<DoorComponent>(elementCase.m_numEntity, Components_e::DOOR_COMPONENT);
        assert(doorComp);
        float m_lateralMod = std::fmod(mapComp->m_absoluteMapPositionPX.second, LEVEL_TILE_SIZE_PX);
        if(!doorComp->m_vertical)
        {
            if((std::sin(radiantObserverAngle) > EPSILON_FLOAT) == (m_lateralMod < LEVEL_HALF_TILE_SIZE_PX))
            {
                return true;
            }
        }
        else
        {
            float m_verticalMod = std::fmod(mapComp->m_absoluteMapPositionPX.first, LEVEL_TILE_SIZE_PX);
            if((std::cos(radiantObserverAngle) > EPSILON_FLOAT) == (m_verticalMod > LEVEL_HALF_TILE_SIZE_PX))
            {
                return true;
            }
        }
    }
    return false;
}

//===================================================================
float getDoorDistance(const MapCoordComponent *mapCompCamera, const MapCoordComponent *mapCompDoor, const DoorComponent *doorComp)
{
    PairFloat_t refPointA = getAbsolutePosition(mapCompDoor->m_coord);
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
float getMiddleDoorDistance(const PairFloat_t &camera, const PairFloat_t &element, bool vertical)
{
    PairFloat_t refPointB = element;
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
PairFloat_t getIntersectCoord(const PairFloat_t &observerPoint, const PairFloat_t &targetPoint,
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
void removeSecondRect(PairFloat_t absolPos[], float distance[], uint32_t &distanceToTreat)
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
                                                    float distance)
{
    VerticesData &vertex = getClearedVertice(numIteration);
    PositionVertexComponent *posComp = stairwayToComponentManager().
            searchComponentByType<PositionVertexComponent>(numEntity, Components_e::POSITION_VERTEX_COMPONENT);
    SpriteTextureComponent *spriteComp = stairwayToComponentManager().
            searchComponentByType<SpriteTextureComponent>(numEntity, Components_e::SPRITE_TEXTURE_COMPONENT);
    assert(posComp);
    assert(spriteComp);
    m_entitiesNumMem.insert(EntityData(distance, spriteComp->m_spriteData->m_textureNum, numIteration));
    vertex.loadVertexStandardEntityByLine(*posComp, *spriteComp, distance, m_memRaycastDist);
}

//===================================================================
VerticesData &FirstPersonDisplaySystem::getClearedVertice(uint32_t index)
{
    //use 1 vertex for 1 sprite for beginning
    if(index < m_vectWallDoorVerticesData.size())
    {
        m_vectWallDoorVerticesData[index].clear();
        m_vectWallDoorVerticesData[index].setShaderType(Shader_e::COLORED_TEXTURE_S);
    }
    else
    {
        do
        {
            m_vectWallDoorVerticesData.push_back(VerticesData(Shader_e::COLORED_TEXTURE_S));
        }
        while(index >= m_vectWallDoorVerticesData.size());
        assert(index < m_vectWallDoorVerticesData.size());
    }
    return m_vectWallDoorVerticesData[index];
}

//===================================================================
void FirstPersonDisplaySystem::setVectTextures(std::vector<Texture> &vectTexture)
{
    m_ptrVectTexture = &vectTexture;
    m_vectWallDoorVerticesData.reserve(50);
}

//===================================================================
void FirstPersonDisplaySystem::confNormalEntityVertex(uint32_t numEntity, VisionComponent *visionComp,
                                                      CollisionTag_e tag, float lateralPosGL, float distance)
{
    PositionVertexComponent *positionComp = stairwayToComponentManager().
            searchComponentByType<PositionVertexComponent>(numEntity, Components_e::POSITION_VERTEX_COMPONENT);
    FPSVisibleStaticElementComponent *fpsStaticComp = stairwayToComponentManager().
            searchComponentByType<FPSVisibleStaticElementComponent>(
                numEntity, Components_e::FPS_VISIBLE_STATIC_ELEMENT_COMPONENT);
    assert(fpsStaticComp);
    assert(positionComp);
    assert(visionComp);
    //quickfix
    if(fpsStaticComp->m_inGameSpriteSize.first > 1.5f)
    {
        if(distance < 15.0f)
        {
            return;
        }
        else if(distance < 40.0f)
        {
            distance = 40.0f;
        }
    }
    else if(distance < 15.0f)
    {
        distance = 15.0f;
    }
    SpriteTextureComponent *spriteComp = stairwayToComponentManager().
            searchComponentByType<SpriteTextureComponent>(numEntity, Components_e::SPRITE_TEXTURE_COMPONENT);
    assert(spriteComp);
    spriteComp->m_reverseVisibilityRate = getFogIntensity(distance);
    positionComp->m_vertex.resize(4);
    //convert to GL context
    float distanceFactor = distance / LEVEL_TILE_SIZE_PX;
    float halfLateralSize = fpsStaticComp->m_inGameSpriteSize.first  / distanceFactor,
            downPos, upPos;
    if(tag == CollisionTag_e::BULLET_ENEMY_CT || tag == CollisionTag_e::BULLET_PLAYER_CT)
    {
        ShotConfComponent *shotComp = stairwayToComponentManager().
                searchComponentByType<ShotConfComponent>(numEntity, Components_e::SHOT_CONF_COMPONENT);
        assert(shotComp);
        if(!shotComp->m_destructPhase)
        {
            downPos = -0.3f / distanceFactor;
        }
        else
        {
            downPos = -(fpsStaticComp->m_inGameSpriteSize.second / distanceFactor) / 1.6f;
        }
        upPos = downPos + fpsStaticComp->m_inGameSpriteSize.second / distanceFactor;
    }
    else if((tag == CollisionTag_e::IMPACT_CT || tag == CollisionTag_e::GHOST_CT) &&
            fpsStaticComp->m_levelElementType == LevelStaticElementType_e::IMPACT)
    {
        ImpactShotComponent *impactComp = stairwayToComponentManager().
            searchComponentByType<ImpactShotComponent>(numEntity, Components_e::IMPACT_CONF_COMPONENT);
        assert(impactComp);
        downPos = (impactComp->m_currentVerticalPos + impactComp->m_moveUp) / distanceFactor;
        upPos = downPos + fpsStaticComp->m_inGameSpriteSize.second / distanceFactor;
    }
    else if(fpsStaticComp->m_levelElementType == LevelStaticElementType_e::CEILING)
    {
        upPos = RAYCAST_VERTICAL_SIZE / distanceFactor;
        downPos = upPos - fpsStaticComp->m_inGameSpriteSize.second / distanceFactor;
    }
    else
    {
        downPos = -RAYCAST_VERTICAL_SIZE / distanceFactor;
        upPos = downPos + fpsStaticComp->m_inGameSpriteSize.second / distanceFactor;
    }
    positionComp->m_vertex[0].first = lateralPosGL - halfLateralSize;
    positionComp->m_vertex[0].second = upPos;
    positionComp->m_vertex[1].first = lateralPosGL + halfLateralSize;
    positionComp->m_vertex[1].second = upPos;
    positionComp->m_vertex[2].first = lateralPosGL + halfLateralSize;
    positionComp->m_vertex[2].second = downPos;
    positionComp->m_vertex[3].first = lateralPosGL - halfLateralSize;
    positionComp->m_vertex[3].second = downPos;
}

//===================================================================
void FirstPersonDisplaySystem::drawVertex()
{
    m_shader->use();
    drawTextureBackground();
    //DONT WORK for multiple player
    for(uint32_t i = 0; i < mVectNumEntity.size(); ++i)
    {
        uint32_t numIteration;
        for(std::multiset<EntityData>::const_iterator it = m_entitiesNumMem.begin(); it != m_entitiesNumMem.end(); ++it)
        {
            numIteration = it->m_iterationNum;
            if(m_vectWallDoorVerticesData[numIteration].getNumberOfIndex() == 0)
            {
                continue;
            }
            assert(numIteration < m_vectWallDoorVerticesData.size());
            m_ptrVectTexture->operator[](static_cast<uint32_t>(it->m_textureNum)).bind();
            m_vectWallDoorVerticesData[numIteration].confVertexBuffer();
            m_vectWallDoorVerticesData[numIteration].drawElement();
        }
    }
}

//===================================================================
void FirstPersonDisplaySystem::drawTextureBackground()
{
    if(m_groundSimpleTextBackground)
    {
        SpriteTextureComponent *spriteComp = stairwayToComponentManager().
                searchComponentByType<SpriteTextureComponent>(*m_groundSimpleTextBackground, Components_e::SPRITE_TEXTURE_COMPONENT);
        assert(spriteComp);
        setShader(*m_memShaders.second);
        m_ptrVectTexture->operator[](static_cast<uint32_t>(spriteComp->m_spriteData->m_textureNum)).bind();
        m_groundSimpleTextVertice.confVertexBuffer();
        m_groundSimpleTextVertice.drawElement();
    }
    if(m_ceilingSimpleTextBackground)
    {
        SpriteTextureComponent *spriteComp = stairwayToComponentManager().
                searchComponentByType<SpriteTextureComponent>(*m_ceilingSimpleTextBackground, Components_e::SPRITE_TEXTURE_COMPONENT);
        assert(spriteComp);
        setShader(*m_memShaders.second);
        m_ptrVectTexture->operator[](static_cast<uint32_t>(spriteComp->m_spriteData->m_textureNum)).bind();
        m_ceilingSimpleVertice.confVertexBuffer();
        m_ceilingSimpleVertice.drawElement();
    }
    setShader(*m_memShaders.first);
    if(m_groundTiledTextBackground)
    {
        SpriteTextureComponent *spriteComp = stairwayToComponentManager().
                searchComponentByType<SpriteTextureComponent>(*m_groundTiledTextBackground, Components_e::SPRITE_TEXTURE_COMPONENT);
        assert(spriteComp);
        m_ptrVectTexture->operator[](static_cast<uint32_t>(spriteComp->m_spriteData->m_textureNum)).bind();
        m_groundTiledTextVertice.confVertexBuffer();
        m_groundTiledTextVertice.drawElement();
    }
    if(m_ceilingTiledTextBackground)
    {
        SpriteTextureComponent *spriteComp = stairwayToComponentManager().
                searchComponentByType<SpriteTextureComponent>(*m_ceilingTiledTextBackground, Components_e::SPRITE_TEXTURE_COMPONENT);
        assert(spriteComp);
        m_ptrVectTexture->operator[](static_cast<uint32_t>(spriteComp->m_spriteData->m_textureNum)).bind();
        m_ceilingTiledVertice.confVertexBuffer();
        m_ceilingTiledVertice.drawElement();
    }
}

//===================================================================
void FirstPersonDisplaySystem::addShaders(Shader &shaderTextColor, Shader &shaderText)
{
    m_memShaders = {&shaderTextColor, &shaderText};
    setShader(*m_memShaders.first);
}

//===================================================================
void FirstPersonDisplaySystem::setShader(Shader &shader)
{
    m_shader = &shader;
    m_shader->use();
}

//===================================================================
void FirstPersonDisplaySystem::memGroundBackgroundEntity(uint32_t entity, bool simpleTexture)
{
    if(simpleTexture)
    {
        m_groundSimpleTextBackground = entity;
        m_groundCeilingSimpleTextureActive = true;
    }
    else
    {
        m_groundTiledTextBackground = entity;
        m_backgroundRaycastActive = true;
    }
}

//===================================================================
void FirstPersonDisplaySystem::memCeilingBackgroundEntity(uint32_t entity, bool simpleTexture)
{
    if(simpleTexture)
    {
        m_ceilingSimpleTextBackground = entity;
        m_groundCeilingSimpleTextureActive = true;
    }
    else
    {
        m_ceilingTiledTextBackground = entity;
        m_backgroundRaycastActive = true;
    }
}

//===================================================================
void FirstPersonDisplaySystem::clearBackgroundData()
{
    m_ceilingTiledTextBackground = std::nullopt;
    m_groundTiledTextBackground = std::nullopt;
    m_groundSimpleTextBackground = std::nullopt;
    m_ceilingSimpleTextBackground = std::nullopt;
    m_groundCeilingSimpleTextureActive = false;
    m_backgroundRaycastActive = false;
    m_ceilingTextureSize = {};
    m_groundTextureSize = {};
}

//===================================================================
bool FirstPersonDisplaySystem::rayCasting(uint32_t observerEntity)
{
    optionalTargetRaycast_t targetPoint;
    //WORK FOR ONE PLAYER ONLY
    m_raycastingData.clear();
    if(m_groundTiledTextBackground)
    {
        m_groundTiledTextVertice.clear();
    }
    if(m_ceilingTiledTextBackground)
    {
        m_ceilingTiledVertice.clear();
    }
    MapCoordComponent *mapCompCamera = stairwayToComponentManager().
            searchComponentByType<MapCoordComponent>(observerEntity, Components_e::MAP_COORD_COMPONENT);
    assert(mapCompCamera);
    PlayerConfComponent *playerComp = stairwayToComponentManager().
            searchComponentByType<PlayerConfComponent>(observerEntity, Components_e::PLAYER_CONF_COMPONENT);
    assert(playerComp);
    if(isInsideWall(mapCompCamera->m_absoluteMapPositionPX))
    {
        playerComp->m_insideWall = true;
        return true;
    }
    else
    {
        playerComp->m_insideWall = false;
    }
    MoveableComponent *moveComp = stairwayToComponentManager().
            searchComponentByType<MoveableComponent>(observerEntity, Components_e::MOVEABLE_COMPONENT);
    assert(moveComp);
    float leftAngle = moveComp->m_degreeOrientation + HALF_CONE_VISION;
    float radiantObserverAngle = getRadiantAngle(moveComp->m_degreeOrientation);
    float currentRadiantAngle = getRadiantAngle(leftAngle), currentLateralScreen = -1.0f;
    float cameraRadiantAngle = getRadiantAngle(moveComp->m_degreeOrientation);
    if(m_groundTiledTextBackground)
    {
        m_groundTiledTextVertice.reserveVertex(RAYCAST_LINE_NUMBER *
                                               RAYCAST_GROUND_CEILING_NUMBER * 16);
        m_groundTiledTextVertice.reserveIndices(RAYCAST_LINE_NUMBER * RAYCAST_GROUND_CEILING_NUMBER * 6);
    }
    if(m_ceilingTiledTextBackground)
    {
        m_ceilingTiledVertice.reserveVertex(RAYCAST_LINE_NUMBER *
                                            RAYCAST_GROUND_CEILING_NUMBER * 16);
        m_ceilingTiledVertice.reserveIndices(RAYCAST_LINE_NUMBER * RAYCAST_GROUND_CEILING_NUMBER * 6);
    }
    //mem entity num & distances
    for(uint32_t j = 0; j < RAYCAST_LINE_NUMBER; ++j)
    {
        targetPoint = calcLineSegmentRaycast(currentRadiantAngle, mapCompCamera->m_absoluteMapPositionPX, true, playerComp->m_frozen);
        if(targetPoint)
        {
            m_memRaycastDist[j] = getCameraDistance(mapCompCamera->m_absoluteMapPositionPX, std::get<0>(*targetPoint), cameraRadiantAngle);
            memRaycastDistance(*std::get<2>(*targetPoint), j, m_memRaycastDist[j], std::get<1>(*targetPoint));
        }
        else
        {
            m_memRaycastDist[j] = -1.0f;
        }
        if(m_backgroundRaycastActive)
        {
            calcVerticalBackgroundLineRaycast(mapCompCamera->m_absoluteMapPositionPX, currentRadiantAngle, currentLateralScreen,
                                              radiantObserverAngle);
        }
        currentLateralScreen += SCREEN_HORIZ_BACKGROUND_GL_STEP;
        currentRadiantAngle -= m_stepAngle;
        if(currentRadiantAngle < EPSILON_FLOAT)
        {
            currentRadiantAngle += PI_DOUBLE;
        }
    }
    return false;
}

//===================================================================
bool FirstPersonDisplaySystem::isInsideWall(const PairFloat_t &pos)
{
    std::optional<ElementRaycast> element = Level::getElementCase({static_cast<uint32_t>(pos.first / LEVEL_TILE_SIZE_PX),
                                                                   static_cast<uint32_t>(pos.second / LEVEL_TILE_SIZE_PX)});
    if(element)
    {
        MapCoordComponent *mapComp;
        RectangleCollisionComponent *rectComp;
        if(element->m_typeStd == LevelCaseType_e::WALL_LC)
        {
            mapComp = stairwayToComponentManager().
                    searchComponentByType<MapCoordComponent>(element->m_numEntity, Components_e::MAP_COORD_COMPONENT);
            assert(mapComp);
            rectComp = stairwayToComponentManager().
                    searchComponentByType<RectangleCollisionComponent>(element->m_numEntity, Components_e::RECTANGLE_COLLISION_COMPONENT);
            assert(rectComp);
            if(checkPointRectCollision(pos, mapComp->m_absoluteMapPositionPX, rectComp->m_size))
            {
                return true;
            }
        }
        if(element->m_memMoveWall)
        {
            for(std::set<uint32_t>::const_iterator it = element->m_memMoveWall->begin(); it != element->m_memMoveWall->end(); ++it)
            {
                mapComp = stairwayToComponentManager().
                        searchComponentByType<MapCoordComponent>(*it, Components_e::MAP_COORD_COMPONENT);
                assert(mapComp);
                rectComp = stairwayToComponentManager().
                        searchComponentByType<RectangleCollisionComponent>(*it, Components_e::RECTANGLE_COLLISION_COMPONENT);
                assert(rectComp);
                if(checkPointRectCollision(pos, mapComp->m_absoluteMapPositionPX, rectComp->m_size))
                {
                    return true;
                }
            }
        }
        if(element->m_memStaticMoveableWall)
        {
            for(std::set<uint32_t>::const_iterator it = element->m_memStaticMoveableWall->begin(); it != element->m_memMoveWall->end(); ++it)
            {
                mapComp = stairwayToComponentManager().
                        searchComponentByType<MapCoordComponent>(*it, Components_e::MAP_COORD_COMPONENT);
                assert(mapComp);
                rectComp = stairwayToComponentManager().
                        searchComponentByType<RectangleCollisionComponent>(*it, Components_e::RECTANGLE_COLLISION_COMPONENT);
                assert(rectComp);
                if(checkPointRectCollision(pos, mapComp->m_absoluteMapPositionPX, rectComp->m_size))
                {
                    return true;
                }
            }
        }
    }
    return false;
}

//===================================================================
void FirstPersonDisplaySystem::calcVerticalBackgroundLineRaycast(const PairFloat_t &observerPos, float currentRadiantAngle,
                                                                 float currentGLLatPos, float radiantObserverAngle)
{
    SpriteTextureComponent *spriteGroundComp = nullptr, *spriteCeilingComp = nullptr;
    PairFloat_t currentGroundGLA = {currentGLLatPos, -1.0f},
            currentGroundGLB = {currentGLLatPos + SCREEN_HORIZ_BACKGROUND_GL_STEP,
                                -1.0f + SCREEN_VERT_BACKGROUND_GL_STEP},
            currentCeilingGLA = {currentGLLatPos, 1.0f},
            currentCeilingGLB = {currentGLLatPos + SCREEN_HORIZ_BACKGROUND_GL_STEP,
                                 1.0f - SCREEN_VERT_BACKGROUND_GL_STEP};
    PairFloat_t currentPoint, pairMod;;
    float totalDistanceTarget;
    float calcAngle = std::abs(radiantObserverAngle - currentRadiantAngle);
    if(m_groundTiledTextBackground)
    {
        spriteGroundComp = stairwayToComponentManager().
                searchComponentByType<SpriteTextureComponent>(*m_groundTiledTextBackground, Components_e::SPRITE_TEXTURE_COMPONENT);
        assert(spriteGroundComp);
        if(!m_groundTextureSize)
        {
            m_groundTextureSize = {spriteGroundComp->m_spriteData->m_texturePosVertex[1].first -
                                   spriteGroundComp->m_spriteData->m_texturePosVertex[0].first,
                                   spriteGroundComp->m_spriteData->m_texturePosVertex[3].second -
                                   spriteGroundComp->m_spriteData->m_texturePosVertex[0].second};
        }
    }
    if(m_ceilingTiledTextBackground)
    {
        spriteCeilingComp = stairwayToComponentManager().
                searchComponentByType<SpriteTextureComponent>(*m_ceilingTiledTextBackground, Components_e::SPRITE_TEXTURE_COMPONENT);
        assert(spriteCeilingComp);
        if(!m_ceilingTextureSize)
        {
            m_ceilingTextureSize = {spriteCeilingComp->m_spriteData->m_texturePosVertex[1].first -
                                    spriteCeilingComp->m_spriteData->m_texturePosVertex[0].first,
                                    spriteCeilingComp->m_spriteData->m_texturePosVertex[3].second -
                                    spriteCeilingComp->m_spriteData->m_texturePosVertex[0].second};
        }
    }
    if(!m_memBackgroundDistance)
    {
        m_memBackgroundDistance = std::array<float, RAYCAST_GROUND_CEILING_NUMBER>();
        for(uint32_t i = 0; i < RAYCAST_GROUND_CEILING_NUMBER; ++i, currentGroundGLA.second += SCREEN_VERT_BACKGROUND_GL_STEP)
        {
            totalDistanceTarget = 30.0f / currentGroundGLA.second;
            currentPoint = observerPos;
            moveElementFromAngle(totalDistanceTarget, currentRadiantAngle, currentPoint);
            (*m_memBackgroundDistance)[i] = getCameraDistance(observerPos, currentPoint, currentRadiantAngle);
        }
        currentGroundGLA = {currentGLLatPos, -1.0f};
    }
    for(uint32_t i = 0; i < RAYCAST_GROUND_CEILING_NUMBER; ++i)
    {
        totalDistanceTarget = (*m_memBackgroundDistance)[i] / std::cos(calcAngle);
        currentPoint = observerPos;
        moveElementFromAngle(totalDistanceTarget, currentRadiantAngle, currentPoint);
        if(spriteGroundComp || spriteCeilingComp)
        {
            pairMod = {std::abs(std::fmod(currentPoint.first, LEVEL_TILE_SIZE_PX)),
                       std::abs(std::fmod(currentPoint.second, LEVEL_TILE_SIZE_PX))};
        }
        if(spriteGroundComp)
        {
            m_groundTiledTextVertice.loadPointBackgroundRaycasting(spriteGroundComp, currentGroundGLA, currentGroundGLB,
                                                                   *m_groundTextureSize, pairMod);
            currentGroundGLA.second += SCREEN_VERT_BACKGROUND_GL_STEP;
            currentGroundGLB.second += SCREEN_VERT_BACKGROUND_GL_STEP;
        }
        if(spriteCeilingComp)
        {
            m_ceilingTiledVertice.loadPointBackgroundRaycasting(spriteCeilingComp, currentCeilingGLA, currentCeilingGLB,
                                                                *m_ceilingTextureSize, pairMod);
            currentCeilingGLA.second -= SCREEN_VERT_BACKGROUND_GL_STEP;
            currentCeilingGLB.second -= SCREEN_VERT_BACKGROUND_GL_STEP;
        }
    }
}

//===================================================================
optionalTargetRaycast_t FirstPersonDisplaySystem::calcLineSegmentRaycast(float radiantAngle,
                                                                         const PairFloat_t &originPoint,
                                                                         bool visual, bool scratchMode)
{
    std::optional<ElementRaycast> element;
    float textPos;
    bool lateral;
    std::optional<PairUI_t> currentCoord;
    std::optional<float> lateralLeadCoef, verticalLeadCoef;
    verticalLeadCoef = getLeadCoef(radiantAngle, false);
    lateralLeadCoef = getLeadCoef(radiantAngle, true);
    PairFloat_t currentPoint = scratchMode ? getCorrectedPosition(originPoint, radiantAngle) : originPoint;
    optionalTargetRaycast_t result;
    lateral = raycastPointLateral(radiantAngle, originPoint);
    currentCoord = getCorrectedCoord(currentPoint, lateral, radiantAngle);
    element = Level::getElementCase(*currentCoord);
    if(element && element->m_type == LevelCaseType_e::DOOR_LC)
    {
        result = calcDoorSegmentRaycast(radiantAngle, lateralLeadCoef,
                                        verticalLeadCoef, currentPoint, *element);
        if(result)
        {
            return result;
        }
    }
    else if(element && element->m_type == LevelCaseType_e::WALL_MOVE_LC)
    {
        result = calcMovingWallSegmentRaycast(radiantAngle, lateralLeadCoef,
                                              verticalLeadCoef, currentPoint, *element);
        if(result)
        {
            return result;
        }
    }
    for(uint32_t k = 0; k < RAYCAST_DEPTH; ++k)//limit distance
    {
        //OOOOK get lateral dirty
        currentPoint = getLimitPointRayCasting(currentPoint, radiantAngle,
                                               lateralLeadCoef, verticalLeadCoef, lateral);
        currentCoord = getCorrectedCoord(currentPoint, lateral, radiantAngle);
        if(!currentCoord)
        {
            break;
        }
        element = Level::getElementCase(*currentCoord);
        if(element)
        {
            if(element->m_type == LevelCaseType_e::WALL_LC)
            {
                textPos = getRaycastTexturePos(radiantAngle, lateral, currentPoint);
                return tupleTargetRaycast_t{currentPoint, textPos, element->m_numEntity};
            }
            else if(element->m_type == LevelCaseType_e::DOOR_LC)
            {
                result = calcDoorSegmentRaycast(radiantAngle, lateralLeadCoef,
                                                verticalLeadCoef, currentPoint, *element);
                if(result)
                {
                    return result;
                }
            }
            else if(element->m_type == LevelCaseType_e::WALL_MOVE_LC)
            {
                result = calcMovingWallSegmentRaycast(radiantAngle, lateralLeadCoef,
                                                      verticalLeadCoef, currentPoint, *element);
                if(result)
                {
                    return result;
                }
            }
            if(visual && element->m_type == LevelCaseType_e::EMPTY_LC)
            {
                if(std::fmod(currentPoint.first, LEVEL_TILE_SIZE_PX) < 0.01f &&
                        std::fmod(currentPoint.second, LEVEL_TILE_SIZE_PX) < 0.01f)
                {
                    optionalTargetRaycast_t result =
                            getTextureLimitCase(radiantAngle, *lateralLeadCoef, *verticalLeadCoef, *currentCoord, currentPoint, lateral);
                    if(result)
                    {
                        return result;
                    }
                }
            }
        }
    }
    if(visual)
    {
        return {};
    }
    else
    {
        return tupleTargetRaycast_t{currentPoint, EPSILON_FLOAT, {}};
    }
}

//===================================================================
float getRaycastTexturePos(float radiantObserverAngle, bool lateral, const PairFloat_t &pos)
{
    if(lateral)
    {
        if(std::sin(radiantObserverAngle) >= 0.0f)
        {
            return std::fmod(pos.first, LEVEL_TILE_SIZE_PX);
        }
        else
        {
            return std::abs(std::fmod(pos.first, LEVEL_TILE_SIZE_PX) - LEVEL_TILE_SIZE_PX);
        }
    }
    else
    {
        if(std::cos(radiantObserverAngle) >= 0.0f)
        {
            return std::fmod(pos.second, LEVEL_TILE_SIZE_PX);
        }
        else
        {
            return std::abs(std::fmod(pos.second, LEVEL_TILE_SIZE_PX) - LEVEL_TILE_SIZE_PX);
        }
    }
}

//===================================================================
optionalTargetRaycast_t FirstPersonDisplaySystem::getTextureLimitCase(float radiantAngle, float lateralLeadCoef, float verticalLeadCoef,
                                                                      const PairUI_t &currentCoord,
                                                                      const PairFloat_t &currentPoint, bool lateral)
{
    PairUI_t pointA, pointB;
    float textPos;
    std::optional<ElementRaycast> elementA, elementB;
    pointA = (lateralLeadCoef < EPSILON_FLOAT) ? PairUI_t{currentCoord.first + 1, currentCoord.second} :
                                                   PairUI_t{currentCoord.first - 1, currentCoord.second};
    pointB = (verticalLeadCoef < EPSILON_FLOAT) ? PairUI_t{currentCoord.first, currentCoord.second + 1} :
                                                   PairUI_t{currentCoord.first, currentCoord.second - 1};
    elementA = (lateralLeadCoef > EPSILON_FLOAT) ? Level::getElementCase(pointA) : Level::getElementCase(pointB);
    elementB = (verticalLeadCoef > EPSILON_FLOAT) ? Level::getElementCase(pointA) : Level::getElementCase(pointB);
    if((elementA && (elementA->m_type == LevelCaseType_e::WALL_LC ||
                     (elementA->m_type == LevelCaseType_e::WALL_MOVE_LC && elementA->m_moveableWallStopped))) &&
            (elementB && (elementB->m_type == LevelCaseType_e::WALL_LC ||
                          (elementB->m_type == LevelCaseType_e::WALL_MOVE_LC && elementB->m_moveableWallStopped))))
    {
        textPos = getRaycastTexturePos(radiantAngle, lateral, {currentPoint.first + 1.0f, currentPoint.second + 1.0f});
        return tupleTargetRaycast_t{currentPoint, textPos, elementA->m_numEntity};
    }
    return {};
}

//===================================================================
optionalTargetRaycast_t FirstPersonDisplaySystem::calcMovingWallSegmentRaycast(float radiantAngle,
                                                                               std::optional<float> lateralLeadCoef,
                                                                               std::optional<float> verticalLeadCoef,
                                                                               PairFloat_t &currentPoint,
                                                                               const ElementRaycast &element)
{
    assert(element.m_memMoveWall);
    PairFloat_t memBase = currentPoint;
    float memDistance;
    //first raycast result second distance
    std::optional<std::pair<tupleTargetRaycast_t, float>> resultMem;
    std::set<uint32_t>::const_iterator it = element.m_memMoveWall->begin();
    for(; it != element.m_memMoveWall->end(); ++it)
    {
        bool lateralColl, textPosWall = false;
        RectangleCollisionComponent *rectComp = stairwayToComponentManager().
                searchComponentByType<RectangleCollisionComponent>(*it, Components_e::RECTANGLE_COLLISION_COMPONENT);
        assert(rectComp);
        MapCoordComponent *mapComp = stairwayToComponentManager().
                searchComponentByType<MapCoordComponent>(*it, Components_e::MAP_COORD_COMPONENT);
        assert(mapComp);
        //first case x pos limit second y pos limit
        PairFloat_t wallPos[2] = {{mapComp->m_absoluteMapPositionPX.first,
                                   mapComp->m_absoluteMapPositionPX.first +
                                   rectComp->m_size.first},
                                  {mapComp->m_absoluteMapPositionPX.second,
                                   mapComp->m_absoluteMapPositionPX.second +
                                   rectComp->m_size.second}};
        //exclude case
        if((std::cos(radiantAngle) < 0.0f && currentPoint.first < wallPos[0].first) ||
                (std::cos(radiantAngle) > 0.0f && currentPoint.first > wallPos[0].second))
        {
            textPosWall = false;
        }
        else if((std::sin(radiantAngle) > 0.0f && currentPoint.second < wallPos[1].first) ||
                (std::sin(radiantAngle) < 0.0f && currentPoint.second > wallPos[1].second))
        {
            textPosWall = false;
        }
        else if(lateralLeadCoef && treatLateralIntersectRect(currentPoint, wallPos, *lateralLeadCoef, radiantAngle))
        {
            lateralColl = true;
            textPosWall = true;
        }
        else if(verticalLeadCoef && treatVerticalIntersectRect(currentPoint, wallPos, *verticalLeadCoef, radiantAngle))
        {
            lateralColl = false;
            textPosWall = true;
        }
        if(textPosWall)
        {
            float textPosWall = getRaycastTexturePos(radiantAngle, lateralColl,
            {std::abs(wallPos[0].first - currentPoint.first),
             std::abs(wallPos[1].first - currentPoint.second)});
            if(element.m_memMoveWall->size() > 1)
            {
                memDistance = getDistance(currentPoint, memBase);
                if(!resultMem || memDistance < resultMem->second)
                {
                    resultMem = {{currentPoint, textPosWall, *it}, memDistance};
                }
            }
            else
            {
                return tupleTargetRaycast_t{currentPoint, textPosWall, *it};
            }
        }
        currentPoint = memBase;
    }
    if(resultMem)
    {
        return resultMem->first;
    }
    return {};
}

//===================================================================
optionalTargetRaycast_t FirstPersonDisplaySystem::calcDoorSegmentRaycast(float radiantAngle,
                                                                         std::optional<float> lateralLeadCoef,
                                                                         std::optional<float> verticalLeadCoef,
                                                                         PairFloat_t &currentPoint,
                                                                         const ElementRaycast &element)
{
    bool textFace, textLateral;
    std::optional<float> textPosDoor = treatDoorRaycast(element.m_numEntity, radiantAngle,
                                                        currentPoint, lateralLeadCoef,
                                                        verticalLeadCoef, textLateral,
                                                        textFace);
    if(textPosDoor)
    {
        if(!textFace)
        {
            textPosDoor = 3.0f;
        }
        else
        {
            textPosDoor = getDoorRaycastTexturePos(*textPosDoor, radiantAngle, textLateral, currentPoint);
        }
        return tupleTargetRaycast_t{currentPoint, *textPosDoor, element.m_numEntity};
    }
    return {};
}

//===================================================================
std::optional<float> FirstPersonDisplaySystem::treatDoorRaycast(uint32_t numEntity, float currentRadiantAngle,
                                                                PairFloat_t &currentPoint,
                                                                std::optional<float> lateralLeadCoef,
                                                                std::optional<float> verticalLeadCoef,
                                                                bool &textLateral, bool &textFace)
{
    bool ok;
    DoorComponent *doorComp = stairwayToComponentManager().
            searchComponentByType<DoorComponent>(numEntity, Components_e::DOOR_COMPONENT);
    assert(doorComp);
    RectangleCollisionComponent *rectComp = stairwayToComponentManager().
            searchComponentByType<RectangleCollisionComponent>(numEntity, Components_e::RECTANGLE_COLLISION_COMPONENT);
    assert(rectComp);
    MapCoordComponent *mapComp = stairwayToComponentManager().
            searchComponentByType<MapCoordComponent>(numEntity, Components_e::MAP_COORD_COMPONENT);
    assert(mapComp);
    //first case x pos limit second y pos limit
    PairFloat_t doorPos[2] = {{mapComp->m_absoluteMapPositionPX.first,
                               mapComp->m_absoluteMapPositionPX.first +
                               rectComp->m_size.first},
                              {mapComp->m_absoluteMapPositionPX.second,
                               mapComp->m_absoluteMapPositionPX.second +
                               rectComp->m_size.second}};
    ok = treatDisplayDoor(currentRadiantAngle, doorComp->m_vertical,currentPoint,
                          doorPos, verticalLeadCoef, lateralLeadCoef,
                          textLateral, textFace);
    if(ok)
    {
        //return diff display in case of door move
        if(!textFace || doorComp->m_currentState == DoorState_e::STATIC_CLOSED)
        {
            return 0.0f;
        }
        if(textLateral)
        {
            return LEVEL_TILE_SIZE_PX - rectComp->m_size.first;
        }
        else
        {
            return LEVEL_TILE_SIZE_PX - rectComp->m_size.second;
        }
    }
    return {};
}

//===================================================================
bool treatDisplayDoor(float currentRadiantAngle, bool doorVertical, PairFloat_t &currentPoint,
                      const PairFloat_t doorPos[], std::optional<float> verticalLeadCoef,
                      std::optional<float> lateralLeadCoef, bool &textLateral, bool &textFace,
                      bool bull)
{
    //exclude case
    if((!bull && std::cos(currentRadiantAngle) < 0.0f && currentPoint.first < doorPos[0].first) ||
            (std::cos(currentRadiantAngle) > 0.0f && currentPoint.first > doorPos[0].second))
    {
        return false;
    }
    if((std::sin(currentRadiantAngle) > 0.0f && currentPoint.second < doorPos[1].first) ||
            (std::sin(currentRadiantAngle) < 0.0f && currentPoint.second > doorPos[1].second))
    {
        return false;
    }
    if(doorVertical)
    {
        if(lateralLeadCoef && treatLateralIntersectRect(currentPoint, doorPos, *lateralLeadCoef, currentRadiantAngle))
        {
            //determine if this is face of the door
            textFace = false;
            return true;
        }
        if(verticalLeadCoef && treatVerticalIntersectRect(currentPoint, doorPos, *verticalLeadCoef, currentRadiantAngle))
        {
            textLateral = false;
            textFace = true;
            return true;
        }
    }
    else
    {
        if(verticalLeadCoef && treatVerticalIntersectRect(currentPoint, doorPos, *verticalLeadCoef, currentRadiantAngle))
        {
            textFace = false;
            return true;
        }
        if(lateralLeadCoef && treatLateralIntersectRect(currentPoint, doorPos, *lateralLeadCoef, currentRadiantAngle))
        {
            textLateral = true;
            textFace = true;
            return true;
        }
    }
    return false;
}

//===================================================================
bool treatLateralIntersectRect(PairFloat_t &currentPoint, const PairFloat_t rectPos[],
                               float lateralLeadCoef, float radiantAngle)
{
    bool upCase;
    if(currentPoint.second <= rectPos[1].first)
    {
        upCase = true;
    }
    else if(currentPoint.second >= rectPos[1].second)
    {
        upCase = false;
    }
    else
    {
        return false;
    }
    float diffLat;
    PairFloat_t tmpPos = currentPoint;
    tmpPos.second = (upCase) ? rectPos[1].first : rectPos[1].second;
    if(std::abs(std::cos(radiantAngle)) > 0.0001f)
    {
        diffLat = lateralLeadCoef * std::abs(tmpPos.second - currentPoint.second) /
                LEVEL_TILE_SIZE_PX;
        tmpPos.first += diffLat;
    }
    if(tmpPos.first >= rectPos[0].first && tmpPos.first <= rectPos[0].second)
    {
        currentPoint = tmpPos;
        return true;
    }
    return false;
}

//===================================================================
bool treatVerticalIntersectRect(PairFloat_t &currentPoint, const PairFloat_t rectPos[],
                                float verticalLeadCoef, float radiantAngle)
{
    bool leftCase;
    if(currentPoint.first <= rectPos[0].first)
    {
        leftCase = true;
    }
    else if(currentPoint.first >= rectPos[0].second)
    {
        leftCase = false;
    }
    else
    {
        return false;
    }
    float diffVert;
    PairFloat_t tmpPos = currentPoint;
    tmpPos.first = (leftCase) ? rectPos[0].first : rectPos[0].second;
    if(std::abs(std::sin(radiantAngle)) > 0.001f)
    {
        diffVert = verticalLeadCoef * std::abs(tmpPos.first - currentPoint.first) /
                LEVEL_TILE_SIZE_PX;
        tmpPos.second += diffVert;
    }
    if(tmpPos.second >= rectPos[1].first && tmpPos.second <= rectPos[1].second)
    {
        currentPoint = tmpPos;
        return true;
    }
    return false;
}

//===================================================================
std::optional<PairUI_t> getCorrectedCoord(const PairFloat_t &currentPoint,
                                          bool lateral, float radiantAngle)
{
    PairFloat_t point = currentPoint;
    bool fmodX = (std::fmod(point.first, LEVEL_TILE_SIZE_PX) <= 0.01f),
            fmodY = (std::fmod(point.second, LEVEL_TILE_SIZE_PX) <= 0.01f);
    //treat limit angle cube case
    //raycast on angle case
    if(fmodX && fmodY)
    {
        bool treat = false;
        std::optional<ElementRaycast> elementA, elementB;
        if(std::cos(radiantAngle) < EPSILON_FLOAT)
        {
            //NORTH WEST
            if(std::sin(radiantAngle) > EPSILON_FLOAT)
            {
                elementA = Level::getElementCase(*getLevelCoord({point.first - 1, point.second}));
                elementB = Level::getElementCase(*getLevelCoord({point.first, point.second - 1}));
                if(elementA && elementB && elementA->m_type == LevelCaseType_e::WALL_LC &&
                        elementB->m_type == LevelCaseType_e::WALL_LC)
                {
                    --point.first;
                    treat = true;
                }
            }
            //SOUTH WEST
            else
            {
                elementA = Level::getElementCase(*getLevelCoord(point));
                elementB = Level::getElementCase(*getLevelCoord({point.first - 1, point.second - 1}));
                if(elementA && elementB && elementA->m_type == LevelCaseType_e::WALL_LC &&
                        elementB->m_type == LevelCaseType_e::WALL_LC)
                {
                    treat = true;
                }
            }
        }
        else
        {
            //NORTH EAST
            if(std::sin(radiantAngle) > EPSILON_FLOAT)
            {
                elementA = Level::getElementCase(*getLevelCoord(point));
                elementB = Level::getElementCase(*getLevelCoord({point.first - 1, point.second - 1}));
                if(elementA && elementB && elementA->m_type == LevelCaseType_e::WALL_LC &&
                        elementB->m_type == LevelCaseType_e::WALL_LC)
                {
                    --point.second;
                    --point.first;
                    treat = true;
                }
            }
            //SOUTH EAST
            else
            {
                elementA = Level::getElementCase(*getLevelCoord({point.first - 1, point.second}));
                elementB = Level::getElementCase(*getLevelCoord({point.first, point.second - 1}));
                if(elementA && elementB && elementA->m_type == LevelCaseType_e::WALL_LC &&
                        elementB->m_type == LevelCaseType_e::WALL_LC)
                {
                    --point.first;
                    treat = true;
                }
            }
        }
        if(!treat)
        {
            if(std::cos(radiantAngle) < EPSILON_FLOAT)
            {
                --point.first;
            }
            if(std::sin(radiantAngle) > EPSILON_FLOAT)
            {
                --point.second;
            }
        }

    }
    else if(fmodY && std::sin(radiantAngle) > EPSILON_FLOAT &&
            (lateral || std::abs(std::cos(radiantAngle)) < 0.0001f))
    {
        --point.second;
    }
    else if(fmodX && !lateral && std::cos(radiantAngle) < EPSILON_FLOAT)
    {
        --point.first;
    }
    return getLevelCoord(point);
}

//===================================================================
void FirstPersonDisplaySystem::memRaycastDistance(uint32_t numEntity, uint32_t lateralScreenPos,
                                                  float distance, float texturePos)
{
    MapRayCastingData_t::iterator it = m_raycastingData.find(numEntity);
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
bool raycastPointLateral(float radiantAngle, const PairFloat_t &cameraPoint)
{
    bool lateral;
    if(!getLeadCoef(radiantAngle, false))
    {
        lateral = true;
    }
    else if(!getLeadCoef(radiantAngle, true))
    {
        lateral = false;
    }
    else
    {
        lateral = (static_cast<int32_t>(std::fmod(cameraPoint.first, LEVEL_TILE_SIZE_PX)) != 0);
    }
    return lateral;
}

//===================================================================
PairFloat_t getLimitPointRayCasting(const PairFloat_t &cameraPoint, float radiantAngle,
                                    std::optional<float> lateralLeadCoef,
                                    std::optional<float> verticalLeadCoef, bool &lateral)
{
    float currentCos = std::cos(radiantAngle),
            currentSin = std::sin(radiantAngle);
    std::optional<float> modulo;
    PairFloat_t prevLimitPoint = cameraPoint;
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
        lateral = true;
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
        lateral = false;
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
PairFloat_t FirstPersonDisplaySystem::getCenterPosition(const MapCoordComponent *mapComp,
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

//===================================================================
uint32_t getMinValueFromEntries(const float distance[])
{
    uint32_t val = 0;
    val = (distance[0] < distance[1]) ? 0 : 1;
    val = (distance[val] < distance[2]) ? val : 2;
    val = (distance[val] < distance[3]) ? val : 3;
    return val;
}

//===================================================================
uint32_t getMaxValueFromEntries(const float distance[])
{
    uint32_t val = 0;
    val = (distance[0] > distance[1]) ? 0 : 1;
    val = (distance[val] > distance[2]) ? val : 2;
    val = (distance[val] > distance[3]) ? val : 3;
    return val;
}

//===================================================================
PairFloat_t getCorrectedPosition(const PairFloat_t &initPos, float radiantAngle)
{
    PairFloat_t finalPos = initPos;
    float cosVal = std::cos(radiantAngle), sinVal = std::sin(radiantAngle);
    if(std::abs(cosVal) > 0.1f && static_cast<uint32_t>(std::fmod(initPos.first, LEVEL_TILE_SIZE_PX)) == 0)
    {
        if(cosVal > EPSILON_FLOAT)
        {
            --finalPos.first;
        }
        else
        {
            ++finalPos.first;
        }
    }
    if(std::abs(sinVal) > 0.1f && static_cast<uint32_t>(std::fmod(initPos.second, LEVEL_TILE_SIZE_PX)) == 0)
    {
        if(sinVal < EPSILON_FLOAT)
        {
            --finalPos.second;
        }
        else
        {
            ++finalPos.second;
        }
    }
    return finalPos;
}

//===================================================================
float getDoorRaycastTexturePos(float textDoor, float radiantObserverAngle, bool lateral, const PairFloat_t &pos)
{
    float textPos = getRaycastTexturePos(radiantObserverAngle, lateral, pos);
    if((lateral && std::sin(radiantObserverAngle) >= 0.0f) ||
            (!lateral && std::cos(radiantObserverAngle) >= 0.0f))
    {
        return textPos + textDoor;
    }
    else
    {
        return textPos - textDoor;
    }
}

//===================================================================
float randFloat(float min, float max)
{
    return std::fmod(static_cast<float>(std::rand()), max) + min;
}

//===================================================================
float getFogIntensity(float distance)
{
    if(distance < MIN_DISTANCE_FOG)
    {
        return 1.0f;
    }
    else
    {
        float distFog = distance - MIN_DISTANCE_FOG;
        if(distFog >= TOTAL_DISTANCE_FOG)
        {
            return 0.0f;
        }
        return std::abs((distFog / TOTAL_DISTANCE_FOG) - 1.0f);
    }
}
