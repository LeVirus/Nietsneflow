#include "MapDisplaySystem.hpp"
#include "Level.hpp"
#include "PictureData.hpp"
#include "CollisionUtils.hpp"
#include <ECS/Components/PositionVertexComponent.hpp>
#include <ECS/Components/MapCoordComponent.hpp>
#include <ECS/Components/SpriteTextureComponent.hpp>
#include <ECS/Components/ColorVertexComponent.hpp>
#include <ECS/Components/MoveableComponent.hpp>
#include <ECS/Components/GeneralCollisionComponent.hpp>
#include <ECS/Components/PlayerConfComponent.hpp>
#include <ECS/Components/CircleCollisionComponent.hpp>
#include <ECS/Components/RectangleCollisionComponent.hpp>
#include <ECS/Components/VisionComponent.hpp>
#include <ECS/Components/WallMultiSpriteConf.hpp>
#include <ECS/Systems/ColorDisplaySystem.hpp>
#include <constants.hpp>
#include <PhysicalEngine.hpp>

//===================================================================
//WARNING CONSIDER THAT LENGHT AND WEIGHT ARE THE SAME
MapDisplaySystem::MapDisplaySystem()
{
    setUsedComponents();
}

//===================================================================
void MapDisplaySystem::confLevelData()
{
    m_localLevelSizePX = Level::getRangeView() * 2;
    m_sizeLevelPX = {Level::getSize().first * LEVEL_TILE_SIZE_PX,
                    Level::getSize().second * LEVEL_TILE_SIZE_PX};
    m_miniMapTileSizeGL = (LEVEL_TILE_SIZE_PX * MAP_LOCAL_SIZE_GL) / m_localLevelSizePX;
    m_fullMapTileSizePX = {m_sizeLevelPX.first / Level::getSize().first,
                           m_sizeLevelPX.second / Level::getSize().second};
    m_fullMapTileSizeGL = {m_fullMapTileSizePX.first * FULL_MAP_SIZE_GL / m_sizeLevelPX.first,
                          m_fullMapTileSizePX.second * FULL_MAP_SIZE_GL / m_sizeLevelPX.second};
}

//===================================================================
void MapDisplaySystem::setUsedComponents()
{
    bAddComponentToSystem(Components_e::POSITION_VERTEX_COMPONENT);
    bAddComponentToSystem(Components_e::SPRITE_TEXTURE_COMPONENT);
    bAddComponentToSystem(Components_e::MAP_COORD_COMPONENT);
}

//===================================================================
void MapDisplaySystem::setShader(Shader &shader)
{
    m_shader = &shader;
}

//===================================================================
void MapDisplaySystem::execSystem()
{
    switch(m_playerComp.m_playerConfComp->m_mapMode)
    {
    case MapMode_e::NONE:
        return;
    case MapMode_e::MINI_MAP:
        drawMiniMap();
        break;
    case MapMode_e::FULL_MAP:
        drawFullMap();
        break;
    }
    updatePlayerArrow(*m_playerComp.m_moveableComp, *m_playerComp.m_posComp);
}

//===================================================================
void MapDisplaySystem::drawMiniMap()
{
    confMiniMapPositionVertexEntities();
    fillMiniMapVertexFromEntities();
    drawMapVertex();
    drawPlayerOnMap();
}

//===================================================================
void MapDisplaySystem::drawFullMap()
{
    confFullMapPositionVertexEntities();
    fillFullMapVertexFromEntities();
    drawMapVertex();
    confVertexPlayerOnFullMap();
    drawPlayerOnMap();
}

//===================================================================
void MapDisplaySystem::confFullMapPositionVertexEntities()
{
    PairFloat_t corner;
    for(std::map<uint32_t, PairUI_t>::const_iterator it = m_entitiesDetectedData.begin();
        it != m_entitiesDetectedData.end();)
    {
        MapCoordComponent *mapComp = stairwayToComponentManager().
                searchComponentByType<MapCoordComponent>(it->first,
                                                         Components_e::MAP_COORD_COMPONENT);
        if(!mapComp)
        {
            it = m_entitiesDetectedData.erase(it);
            continue;
        }
        //get absolute position corner
        corner = getUpLeftCorner(mapComp, it->first);
        //convert absolute position to relative
        confFullMapVertexElement(corner, it->first);
        ++it;
    }
}

//===================================================================
void MapDisplaySystem::confVertexPlayerOnFullMap()
{
    if(m_playerComp.m_posComp->m_vertex.empty())
    {
        m_playerComp.m_posComp->m_vertex.resize(3);
    }
    float angle = m_playerComp.m_moveableComp->m_degreeOrientation;
    float radiantAngle = getRadiantAngle(angle);
    PairFloat_t GLPos = {m_playerComp.m_mapCoordComp->m_absoluteMapPositionPX.first / m_sizeLevelPX.first * FULL_MAP_SIZE_GL,
                        m_playerComp.m_mapCoordComp->m_absoluteMapPositionPX.second / m_sizeLevelPX.second * FULL_MAP_SIZE_GL};
    // ((absolutePositionPX.first / m_sizeLevelPX.first) * FULL_MAP_SIZE_GL)
    m_playerComp.m_posComp->m_vertex[0].first = MAP_FULL_TOP_LEFT_X_GL + GLPos.first +
            cos(radiantAngle) * m_fullMapTileSizeGL.first;
    m_playerComp.m_posComp->m_vertex[0].second = MAP_FULL_TOP_LEFT_Y_GL - GLPos.second +
            sin(radiantAngle) * m_fullMapTileSizeGL.second;
    angle += 150.0f;
    radiantAngle = getRadiantAngle(angle);

    m_playerComp.m_posComp->m_vertex[1].first = MAP_FULL_TOP_LEFT_X_GL + GLPos.first +
            cos(radiantAngle) * m_fullMapTileSizeGL.first;
    m_playerComp.m_posComp->m_vertex[1].second = MAP_FULL_TOP_LEFT_Y_GL - GLPos.second +
            sin(radiantAngle) * m_fullMapTileSizeGL.second;
    angle += 60.0f;
    radiantAngle = getRadiantAngle(angle);

    m_playerComp.m_posComp->m_vertex[2].first = MAP_FULL_TOP_LEFT_X_GL + GLPos.first +
            cos(radiantAngle) * m_fullMapTileSizeGL.first;
    m_playerComp.m_posComp->m_vertex[2].second = MAP_FULL_TOP_LEFT_Y_GL - GLPos.second +
            sin(radiantAngle) * m_fullMapTileSizeGL.second;
}

//===================================================================
void MapDisplaySystem::confMiniMapPositionVertexEntities()
{
    assert(m_playerComp.m_mapCoordComp);
    PairFloat_t playerPos = m_playerComp.m_mapCoordComp->m_absoluteMapPositionPX;
    PairFloat_t corner, diffPosPX, relativePosMapGL;
    PairUI_t max, min;
    getMapDisplayLimit(playerPos, min, max);
    m_entitiesToDisplay.clear();
    m_entitiesToDisplay.reserve(mVectNumEntity.size());
    for(std::map<uint32_t, PairUI_t>::const_iterator it = m_entitiesDetectedData.begin();
         it != m_entitiesDetectedData.end(); ++it)
    {
        MapCoordComponent *mapComp = stairwayToComponentManager().
                searchComponentByType<MapCoordComponent>(it->first,
                                                         Components_e::MAP_COORD_COMPONENT);
        if(!mapComp)
        {
            it = m_entitiesDetectedData.erase(it);
            continue;
        }
        if(checkBoundEntityMap(*mapComp, min, max))
        {
            //get absolute position corner
            corner = getUpLeftCorner(mapComp, it->first);
            m_entitiesToDisplay.emplace_back(it->first);
            diffPosPX = corner - m_playerComp.m_mapCoordComp->m_absoluteMapPositionPX;
            //convert absolute position to relative
            relativePosMapGL = {diffPosPX.first * MAP_LOCAL_SIZE_GL / m_localLevelSizePX,
                                diffPosPX.second * MAP_LOCAL_SIZE_GL / m_localLevelSizePX};
            confMiniMapVertexElement(relativePosMapGL, it->first);
        }
    }
}

//===================================================================
void MapDisplaySystem::fillMiniMapVertexFromEntities()
{
    for(uint32_t h = 0; h < m_vectMapVerticesData.size(); ++h)
    {
        m_vectMapVerticesData[h].clear();
    }
    for(uint32_t i = 0; i < m_entitiesToDisplay.size(); ++i)
    {
        PositionVertexComponent *posComp = stairwayToComponentManager().
                searchComponentByType<PositionVertexComponent>(m_entitiesToDisplay[i],
                                                               Components_e::POSITION_VERTEX_COMPONENT);
        SpriteTextureComponent *spriteComp = stairwayToComponentManager().
                searchComponentByType<SpriteTextureComponent>(m_entitiesToDisplay[i],
                                                            Components_e::SPRITE_TEXTURE_COMPONENT);
        assert(posComp);
        if(!spriteComp)
        {
            GeneralCollisionComponent *genComp = stairwayToComponentManager().
                    searchComponentByType<GeneralCollisionComponent>(m_entitiesToDisplay[i],
                                                                Components_e::GENERAL_COLLISION_COMPONENT);
            assert(genComp);
            if(genComp->m_tagA == CollisionTag_e::TRIGGER_CT)
            {
                continue;
            }
        }
        assert(spriteComp);
        assert(spriteComp->m_spriteData->m_textureNum < m_vectMapVerticesData.size());
        m_vectMapVerticesData[spriteComp->m_spriteData->m_textureNum].
                loadVertexStandartTextureComponent(*posComp, *spriteComp);
    }
}

//===================================================================
void MapDisplaySystem::fillFullMapVertexFromEntities()
{
    for(uint32_t h = 0; h < m_vectMapVerticesData.size(); ++h)
    {
        m_vectMapVerticesData[h].clear();
    }
    for(std::map<uint32_t, PairUI_t>::const_iterator it = m_entitiesDetectedData.begin();
         it != m_entitiesDetectedData.end(); ++it)
    {
        PositionVertexComponent *posComp = stairwayToComponentManager().
                searchComponentByType<PositionVertexComponent>(it->first,
                                                               Components_e::POSITION_VERTEX_COMPONENT);
        SpriteTextureComponent *spriteComp = stairwayToComponentManager().
                searchComponentByType<SpriteTextureComponent>(it->first,
                                                              Components_e::SPRITE_TEXTURE_COMPONENT);
        assert(posComp);
        if(!spriteComp)
        {
            GeneralCollisionComponent *genComp = stairwayToComponentManager().
                    searchComponentByType<GeneralCollisionComponent>(it->first,
                                                                Components_e::GENERAL_COLLISION_COMPONENT);
            assert(genComp);
            if(genComp->m_tagA == CollisionTag_e::TRIGGER_CT)
            {
                continue;
            }
        }
        assert(spriteComp);
        assert(spriteComp->m_spriteData->m_textureNum < m_vectMapVerticesData.size());
        m_vectMapVerticesData[spriteComp->m_spriteData->m_textureNum].
                loadVertexStandartTextureComponent(*posComp, *spriteComp);
    }
}

//===================================================================
PairFloat_t MapDisplaySystem::getUpLeftCorner(const MapCoordComponent *mapCoordComp, uint32_t entityNum)
{
    GeneralCollisionComponent *genCollComp = stairwayToComponentManager().
            searchComponentByType<GeneralCollisionComponent>(entityNum, Components_e::GENERAL_COLLISION_COMPONENT);
    assert(genCollComp);
    assert(mapCoordComp);
    if(genCollComp->m_shape == CollisionShape_e::CIRCLE_C)
    {
        CircleCollisionComponent *circleCollComp = stairwayToComponentManager().
                searchComponentByType<CircleCollisionComponent>(entityNum, Components_e::CIRCLE_COLLISION_COMPONENT);
        assert(circleCollComp);
        return getCircleUpLeftCorner(mapCoordComp->m_absoluteMapPositionPX, circleCollComp->m_ray);
    }
    else
    {
        if(genCollComp->m_tagA == CollisionTag_e::DOOR_CT)
        {
            return {mapCoordComp->m_absoluteMapPositionPX.first - std::fmod(mapCoordComp->m_absoluteMapPositionPX.first, LEVEL_TILE_SIZE_PX),
                        mapCoordComp->m_absoluteMapPositionPX.second - std::fmod(mapCoordComp->m_absoluteMapPositionPX.second, LEVEL_TILE_SIZE_PX)};
        }
        else
        {
            return mapCoordComp->m_absoluteMapPositionPX;
        }
    }
}

//===================================================================
void MapDisplaySystem::getMapDisplayLimit(PairFloat_t &playerPos,
                                          PairUI_t &min, PairUI_t &max)
{
    assert(playerPos.first >= 0.0f || playerPos.second >= 0.0f);
    //getBound
    float rangeView = Level::getRangeView();
    playerPos.first += rangeView;
    playerPos.second += rangeView;
    max = *getLevelCoord(playerPos);
    playerPos.first -= rangeView * 2;
    if(playerPos.first < LEVEL_TILE_SIZE_PX)
    {
        min.first = 0;
    }
    else
    {
        min.first = static_cast<uint32_t>(playerPos.first / LEVEL_TILE_SIZE_PX);
    }
    playerPos.second -= rangeView * 2;
    if(playerPos.second < LEVEL_TILE_SIZE_PX)
    {
        min.second = 0;
    }
    else
    {
        min.second = static_cast<uint32_t>(playerPos.second / LEVEL_TILE_SIZE_PX);
    }
}


//===================================================================
void MapDisplaySystem::confMiniMapVertexElement(const PairFloat_t &glPosition,
                                         uint32_t entityNum)
{
    PositionVertexComponent *posComp = stairwayToComponentManager().
            searchComponentByType<PositionVertexComponent>(entityNum,
                                                           Components_e::POSITION_VERTEX_COMPONENT);
    assert(posComp);
    posComp->m_vertex.resize(4);
    //CONSIDER THAT MAP X AND Y ARE THE SAME
    if(posComp->m_vertex.empty())
    {
        posComp->m_vertex.resize(4);
    }
    posComp->m_vertex[0] = {MAP_LOCAL_CENTER_X_GL + glPosition.first,
                            MAP_LOCAL_CENTER_Y_GL + glPosition.second};
    posComp->m_vertex[1] = {MAP_LOCAL_CENTER_X_GL + glPosition.first + m_miniMapTileSizeGL,
                            MAP_LOCAL_CENTER_Y_GL + glPosition.second};
    posComp->m_vertex[2] = {MAP_LOCAL_CENTER_X_GL + glPosition.first + m_miniMapTileSizeGL,
                            MAP_LOCAL_CENTER_Y_GL + glPosition.second - m_miniMapTileSizeGL};
    posComp->m_vertex[3] = {MAP_LOCAL_CENTER_X_GL + glPosition.first,
                            MAP_LOCAL_CENTER_Y_GL + glPosition.second - m_miniMapTileSizeGL};
}

//===================================================================
void MapDisplaySystem::confFullMapVertexElement(const PairFloat_t &absolutePositionPX, uint32_t entityNum)
{
    PositionVertexComponent *posComp = stairwayToComponentManager().
            searchComponentByType<PositionVertexComponent>(entityNum,
                                                           Components_e::POSITION_VERTEX_COMPONENT);
    assert(posComp);
    posComp->m_vertex.resize(4);
    //CONSIDER THAT MAP X AND Y ARE THE SAME
    if(posComp->m_vertex.empty())
    {
        posComp->m_vertex.resize(4);
    }
    double leftPos = MAP_FULL_TOP_LEFT_X_GL +
            ((absolutePositionPX.first / m_sizeLevelPX.first) * FULL_MAP_SIZE_GL),
            rightPos = leftPos + m_fullMapTileSizeGL.first,
            topPos = MAP_FULL_TOP_LEFT_Y_GL -
            ((absolutePositionPX.second  / m_sizeLevelPX.second) * FULL_MAP_SIZE_GL),
            downPos = topPos - m_fullMapTileSizeGL.second;
    posComp->m_vertex[0] = {leftPos, topPos};
    posComp->m_vertex[1] = {rightPos, topPos};
    posComp->m_vertex[2] = {rightPos, downPos};
    posComp->m_vertex[3] = {leftPos, downPos};
}

//===================================================================
bool MapDisplaySystem::checkBoundEntityMap(const MapCoordComponent &mapCoordComp,
                                           const PairUI_t &minBound,
                                           const PairUI_t &maxBound)
{
    if(mapCoordComp.m_coord.first < minBound.first ||
            mapCoordComp.m_coord.second < minBound.second)
    {
        return false;
    }
    if(mapCoordComp.m_coord.first > maxBound.first ||
            mapCoordComp.m_coord.second > maxBound.second)
    {
        return false;
    }
    return true;
}

//===================================================================
void MapDisplaySystem::drawMapVertex()
{
    m_shader->use();
    for(uint32_t h = 0; h < m_vectMapVerticesData.size(); ++h)
    {
        m_ptrVectTexture->operator[](h).bind();
        m_vectMapVerticesData[h].confVertexBuffer();
        m_vectMapVerticesData[h].drawElement();
    }
}

//===================================================================
void MapDisplaySystem::drawPlayerVision()
{
    assert(m_playerComp.m_visionComp);
    mptrSystemManager->searchSystemByType<ColorDisplaySystem>(
                static_cast<uint32_t>(Systems_e::COLOR_DISPLAY_SYSTEM))->drawEntity(&m_playerComp.m_visionComp->m_positionVertexComp,
                                                             &m_playerComp.m_visionComp->m_colorVertexComp);
}

//===================================================================
void MapDisplaySystem::drawPlayerOnMap()
{
    assert(m_playerComp.m_posComp);
    assert(m_playerComp.m_colorComp);
    mptrSystemManager->searchSystemByType<ColorDisplaySystem>(
                static_cast<uint32_t>(Systems_e::COLOR_DISPLAY_SYSTEM))->drawEntity(m_playerComp.m_posComp,
                                                             m_playerComp.m_colorComp);
}

//===================================================================
void MapDisplaySystem::confPlayerComp(uint32_t playerNum)
{
    m_playerComp.m_mapCoordComp = stairwayToComponentManager().
            searchComponentByType<MapCoordComponent>(playerNum,
                                                     Components_e::MAP_COORD_COMPONENT);
    m_playerComp.m_posComp = stairwayToComponentManager().
            searchComponentByType<PositionVertexComponent>(playerNum,
                                                           Components_e::POSITION_VERTEX_COMPONENT);
    m_playerComp.m_colorComp = stairwayToComponentManager().
            searchComponentByType<ColorVertexComponent>(playerNum,
                                                        Components_e::COLOR_VERTEX_COMPONENT);
    m_playerComp.m_moveableComp = stairwayToComponentManager().
            searchComponentByType<MoveableComponent>(playerNum, Components_e::MOVEABLE_COMPONENT);
    m_playerComp.m_playerConfComp = stairwayToComponentManager().
            searchComponentByType<PlayerConfComponent>(playerNum, Components_e::PLAYER_CONF_COMPONENT);
    VisionComponent *visionComp = stairwayToComponentManager().
            searchComponentByType<VisionComponent>(playerNum,
                                                   Components_e::VISION_COMPONENT);
    assert(visionComp);
    assert(m_playerComp.m_posComp);
    assert(m_playerComp.m_colorComp);
    assert(m_playerComp.m_mapCoordComp);
    assert(m_playerComp.m_moveableComp);
    assert(m_playerComp.m_playerConfComp);
    visionComp->m_colorVertexComp.m_vertex.reserve(3);
    visionComp->m_colorVertexComp.m_vertex.emplace_back(0.00f, 100.00f, 0.00f, 1.0f);
    visionComp->m_colorVertexComp.m_vertex.emplace_back(0.00f, 10.00f, 0.00f, 1.0f);
    visionComp->m_colorVertexComp.m_vertex.emplace_back(0.00f, 10.00f, 0.00f, 1.0f);
    m_playerComp.m_visionComp = visionComp;
}

//===================================================================
void MapDisplaySystem::setVectTextures(std::vector<Texture> &vectTexture)
{
    m_ptrVectTexture = &vectTexture;
    m_vectMapVerticesData.reserve(vectTexture.size());
    for(uint32_t h = 0; h < vectTexture.size(); ++h)
    {
        m_vectMapVerticesData.emplace_back(VerticesData(Shader_e::TEXTURE_S));
    }
}
