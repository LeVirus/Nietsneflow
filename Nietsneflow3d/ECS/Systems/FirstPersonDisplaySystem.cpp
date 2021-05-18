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
#include <ECS/Systems/ColorDisplaySystem.hpp>
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
    drawPlayerDamage();
}

//===================================================================
void FirstPersonDisplaySystem::drawPlayerDamage()
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
    }
}

//===================================================================
void FirstPersonDisplaySystem::confCompVertexMemEntities()
{
    uint32_t vectEntitiesSize = mVectNumEntity.size();
    m_numVertexToDraw.resize(vectEntitiesSize);
    //treat one player
    uint32_t toRemove = 0;
    VisionComponent *visionComp;
    MoveableComponent *moveComp;
    MapCoordComponent *mapCompA;
    GeneralCollisionComponent *genCollComp;
    MapCoordComponent *mapCompB;
    uint32_t numIteration;
    for(uint32_t i = 0; i < vectEntitiesSize; ++i)
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
            if(genCollComp->m_active)
            {
                treatDisplayEntity(genCollComp, mapCompA, mapCompB, visionComp,
                                   toRemove, moveComp->m_degreeOrientation, numIteration);
            }
        }
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
    m_entitiesNumMem.insert(EntityData(distance, spriteComp->m_spriteData->m_textureNum,
                                       numIteration));
}


//===================================================================
void FirstPersonDisplaySystem::treatDisplayEntity(GeneralCollisionComponent *genCollComp, MapCoordComponent *mapCompA,
                                                  MapCoordComponent *mapCompB, VisionComponent *visionComp,
                                                  uint32_t &toRemove, float degreeObserverAngle, uint32_t numIteration)
{
    float radiantObserverAngle = getRadiantAngle(degreeObserverAngle);
    uint32_t numEntity = visionComp->m_vectVisibleEntities[numIteration];
    pairFloat_t centerPosB = getCenterPosition(mapCompB, genCollComp, numEntity);
    float distance = getCameraDistance(mapCompA->m_absoluteMapPositionPX,
                                       mapCompB->m_absoluteMapPositionPX, radiantObserverAngle);
    float simpleDistance = getDistance(mapCompA->m_absoluteMapPositionPX,
                                       mapCompB->m_absoluteMapPositionPX);
    if(distance > visionComp->m_distanceVisibility || distance < 1.0f)
    {
        ++toRemove;
        return;
    }
    float trigoAngle = getTrigoAngle(mapCompA->m_absoluteMapPositionPX, centerPosB);
    //get lateral pos from angle
    float lateralPos = getLateralAngle(degreeObserverAngle, trigoAngle);
    confNormalEntityVertex(numEntity, visionComp, genCollComp->m_tag, lateralPos, distance);
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
    if(m_entitiesNumMem.size() == 1)
    {
        EntityData(distance - LEVEL_TILE_SIZE_PX, spriteComp->m_spriteData->m_textureNum,
                                               numIteration);
    }
    m_entitiesNumMem.insert(EntityData(distance - LEVEL_TILE_SIZE_PX, spriteComp->m_spriteData->m_textureNum,
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
                                                      CollisionTag_e tag,
                                                      float lateralPosDegree, float distance)
{
    PositionVertexComponent *positionComp = stairwayToComponentManager().
            searchComponentByType<PositionVertexComponent>(numEntity, Components_e::POSITION_VERTEX_COMPONENT);
    FPSVisibleStaticElementComponent *fpsStaticComp = stairwayToComponentManager().
            searchComponentByType<FPSVisibleStaticElementComponent>(
                numEntity, Components_e::FPS_VISIBLE_STATIC_ELEMENT_COMPONENT);
    assert(fpsStaticComp);
    assert(positionComp);
    assert(visionComp);
    positionComp->m_vertex.resize(4);
    //convert to GL context
    float lateralPosGL = lateralPosDegree;
    //quickfix
    if(distance < 1.5f)
    {
        distance = 1.5f;
    }
    float halfLateralSize = fpsStaticComp->m_inGameSpriteSize.first  / (distance / LEVEL_TILE_SIZE_PX),
            downPos, upPos;
    if(tag == CollisionTag_e::BULLET_ENEMY_CT || tag == CollisionTag_e::BULLET_PLAYER_CT)
    {
        downPos = -0.3f / (distance / LEVEL_TILE_SIZE_PX);
        upPos = downPos + fpsStaticComp->m_inGameSpriteSize.second / (distance / LEVEL_TILE_SIZE_PX);
    }
    else
    {
        downPos = -RAYCAST_VERTICAL_SIZE / (distance / LEVEL_TILE_SIZE_PX);
        upPos = downPos + fpsStaticComp->m_inGameSpriteSize.second / (distance / LEVEL_TILE_SIZE_PX);
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
    bool lateral = false, textLateral, textFace;
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
                        textPos = lateral ? currentPoint.first : currentPoint.second;
                        memDistance(element->m_numEntity, j, getCameraDistance(mapCompCamera->m_absoluteMapPositionPX,
                                                                               currentPoint, cameraRadiantAngle), textPos);
                        break;
                    }
                    else if(element->m_type == LevelCaseType_e::DOOR_LC)
                    {
                        std::optional<float> textPos = treatDoorRaycast(element->m_numEntity, radiantAngle,
                                                                        currentPoint, lateralLeadCoef,
                                                                        verticalLeadCoef, textLateral, textFace);
                        if(textPos)
                        {
                            if(!textFace)
                            {
                                textPos = 3.0f;
                            }
                            else
                            {
                                textPos = textLateral == textFace ? std::fmod(currentPoint.first, LEVEL_TILE_SIZE_PX) + *textPos :
                                                        std::fmod(currentPoint.second, LEVEL_TILE_SIZE_PX) + *textPos;
                            }
                            memDistance(element->m_numEntity, j,
                                        getCameraDistance(mapCompCamera->m_absoluteMapPositionPX,
                                                          currentPoint, cameraRadiantAngle), *textPos);
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
std::optional<pairUI_t> getCorrectedCoord(const pairFloat_t &currentPoint, bool lateral,
                                          float radiantAngle)
{
    pairFloat_t point = currentPoint;
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
    return getLevelCoord(point);
}

//===================================================================
std::optional<float> FirstPersonDisplaySystem::treatDoorRaycast(uint32_t numEntity, float radiantAngle,
                                                                pairFloat_t &currentPoint,
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
    pairFloat_t doorPos[2] = {{mapComp->m_absoluteMapPositionPX.first,
                               mapComp->m_absoluteMapPositionPX.first +
                               rectComp->m_size.first},
                              {mapComp->m_absoluteMapPositionPX.second,
                               mapComp->m_absoluteMapPositionPX.second +
                               rectComp->m_size.second}};
    ok = treatDisplayDoor(radiantAngle, doorComp->m_vertical, currentPoint,
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
bool treatDisplayDoor(float radiantAngle, bool doorVertical, pairFloat_t &currentPoint,
                      pairFloat_t doorPos[], std::optional<float> verticalLeadCoef,
                      std::optional<float> lateralLeadCoef,
                      bool &textLateral, bool &textFace)
{
    //exclude case
    if((std::cos(radiantAngle) < 0.0f && currentPoint.first < doorPos[0].first ) ||
            (std::cos(radiantAngle) > 0.0f && currentPoint.first > doorPos[0].second))
    {
        return false;
    }
    if((std::sin(radiantAngle) > 0.0f && currentPoint.second < doorPos[1].first ) ||
            (std::sin(radiantAngle) < 0.0f && currentPoint.second > doorPos[1].second))
    {
        return false;
    }
    if(doorVertical)
    {
        if(treatLateralIntersectDoor(currentPoint, doorPos, *lateralLeadCoef))
        {
            //determine if this is face of the door
            textFace = false;
            return true;
        }
        if(!verticalLeadCoef)
        {
            return false;
        }
        if(treatVerticalIntersectDoor(currentPoint, doorPos, *verticalLeadCoef))
        {
            textLateral = false;
            textFace = true;
            return true;
        }
    }
    else
    {
        if(treatVerticalIntersectDoor(currentPoint, doorPos, *verticalLeadCoef))
        {
            textFace = false;
            return true;
        }
        if(!lateralLeadCoef)
        {
            return false;
        }
        if(treatLateralIntersectDoor(currentPoint, doorPos, *lateralLeadCoef))
        {
            textLateral = true;
            textFace = true;
            return true;
        }
    }
    return false;
}

//===================================================================
bool treatLateralIntersectDoor(pairFloat_t &currentPoint, const pairFloat_t doorPos[],
                               float lateralLeadCoef)
{
    bool upCase;
    if(currentPoint.second <= doorPos[1].first)
    {
        upCase = true;
    }
    else if(currentPoint.second >= doorPos[1].second)
    {
        upCase = false;
    }
    else
    {
        return false;
    }
    float diffLat;
    pairFloat_t tmpPos = currentPoint;
    tmpPos.second = (upCase) ? doorPos[1].first : doorPos[1].second;
    diffLat = lateralLeadCoef * std::abs(tmpPos.second - currentPoint.second) /
            LEVEL_TILE_SIZE_PX;
    tmpPos.first += diffLat;
    if(tmpPos.first >= doorPos[0].first && tmpPos.first <= doorPos[0].second)
    {
        currentPoint = tmpPos;
        return true;
    }
    return false;
}

//===================================================================
bool treatVerticalIntersectDoor(pairFloat_t &currentPoint, const pairFloat_t doorPos[],
                                float verticalLeadCoef)
{
    bool leftCase;
    if(currentPoint.first <= doorPos[0].first)
    {
        leftCase = true;
    }
    else if(currentPoint.first >= doorPos[0].second)
    {
        leftCase = false;
    }
    else
    {
        return false;
    }
    float diffVert;
    pairFloat_t tmpPos = currentPoint;
    tmpPos.first = (leftCase) ? doorPos[0].first : doorPos[0].second;
    diffVert = verticalLeadCoef * std::abs(tmpPos.first - currentPoint.first) /
            LEVEL_TILE_SIZE_PX;
    tmpPos.second += diffVert;
    if(tmpPos.second >= doorPos[1].first && tmpPos.second <= doorPos[1].second)
    {
        currentPoint = tmpPos;
        return true;
    }
    return false;
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
