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
MapDisplaySystem::MapDisplaySystem(NewComponentManager &newComponentManager) : m_newComponentManager(newComponentManager),
    m_componentsContainer(m_newComponentManager.getComponentsContainer())
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
    PlayerConfComponent &playerConfComp = m_componentsContainer.m_playerConfComp;
    switch(playerConfComp.m_mapMode)
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
    OptUint_t compNum = m_newComponentManager.getComponentEmplacement(m_playerNum, Components_e::POSITION_VERTEX_COMPONENT);
    assert(compNum);
    PositionVertexComponent &posComp = m_componentsContainer.m_vectPositionVertexComp[*compNum];

    compNum = m_newComponentManager.getComponentEmplacement(m_playerNum, Components_e::MOVEABLE_COMPONENT);
    assert(compNum);
    MoveableComponent &moveComp = m_componentsContainer.m_vectMoveableComp[*compNum];
    updatePlayerArrow(moveComp, posComp);
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
    OptUint_t compNum;
    PairFloat_t corner;
    for(std::map<uint32_t, PairUI_t>::const_iterator it = m_entitiesDetectedData.begin();
        it != m_entitiesDetectedData.end();)
    {
        compNum = m_newComponentManager.getComponentEmplacement(it->first, Components_e::MAP_COORD_COMPONENT);
        if(!compNum)
        {
            it = m_entitiesDetectedData.erase(it);
            continue;
        }
        MapCoordComponent &mapComp = m_componentsContainer.m_vectMapCoordComp[*compNum];
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
    OptUint_t compNum = m_newComponentManager.getComponentEmplacement(m_playerNum, Components_e::POSITION_VERTEX_COMPONENT);
    assert(compNum);
    PositionVertexComponent &posComp = m_componentsContainer.m_vectPositionVertexComp[*compNum];
    compNum = m_newComponentManager.getComponentEmplacement(m_playerNum, Components_e::MAP_COORD_COMPONENT);
    assert(compNum);
    MapCoordComponent &mapComp = m_componentsContainer.m_vectMapCoordComp[*compNum];
    compNum = m_newComponentManager.getComponentEmplacement(m_playerNum, Components_e::MOVEABLE_COMPONENT);
    assert(compNum);
    MoveableComponent &moveComp = m_componentsContainer.m_vectMoveableComp[*compNum];
    if(posComp.m_vertex.empty())
    {
        posComp.m_vertex.resize(3);
    }
    float angle = moveComp.m_degreeOrientation;
    float radiantAngle = getRadiantAngle(angle);
    PairFloat_t GLPos = {mapComp.m_absoluteMapPositionPX.first / m_sizeLevelPX.first * FULL_MAP_SIZE_GL,
                        mapComp.m_absoluteMapPositionPX.second / m_sizeLevelPX.second * FULL_MAP_SIZE_GL};
    // ((absolutePositionPX.first / m_sizeLevelPX.first) * FULL_MAP_SIZE_GL)
    posComp.m_vertex[0].first = MAP_FULL_TOP_LEFT_X_GL + GLPos.first +
            cos(radiantAngle) * m_fullMapTileSizeGL.first;
    posComp.m_vertex[0].second = MAP_FULL_TOP_LEFT_Y_GL - GLPos.second +
            sin(radiantAngle) * m_fullMapTileSizeGL.second;
    angle += 150.0f;
    radiantAngle = getRadiantAngle(angle);

    posComp.m_vertex[1].first = MAP_FULL_TOP_LEFT_X_GL + GLPos.first +
            cos(radiantAngle) * m_fullMapTileSizeGL.first;
    posComp.m_vertex[1].second = MAP_FULL_TOP_LEFT_Y_GL - GLPos.second +
            sin(radiantAngle) * m_fullMapTileSizeGL.second;
    angle += 60.0f;
    radiantAngle = getRadiantAngle(angle);

    posComp.m_vertex[2].first = MAP_FULL_TOP_LEFT_X_GL + GLPos.first +
            cos(radiantAngle) * m_fullMapTileSizeGL.first;
    posComp.m_vertex[2].second = MAP_FULL_TOP_LEFT_Y_GL - GLPos.second +
            sin(radiantAngle) * m_fullMapTileSizeGL.second;
}

//===================================================================
void MapDisplaySystem::confMiniMapPositionVertexEntities()
{
    OptUint_t compNum = m_newComponentManager.getComponentEmplacement(m_playerNum, Components_e::MAP_COORD_COMPONENT);
    assert(compNum);
    MapCoordComponent &mapCompPlayer = m_componentsContainer.m_vectMapCoordComp[*compNum];
    PairFloat_t playerPos = mapCompPlayer.m_absoluteMapPositionPX;
    PairFloat_t corner, diffPosPX, relativePosMapGL;
    PairUI_t max, min;
    getMapDisplayLimit(playerPos, min, max);
    m_entitiesToDisplay.clear();
    m_entitiesToDisplay.reserve(mVectNumEntity.size());
    for(std::map<uint32_t, PairUI_t>::const_iterator it = m_entitiesDetectedData.begin();
         it != m_entitiesDetectedData.end(); ++it)
    {
        compNum = m_newComponentManager.getComponentEmplacement(it->first, Components_e::MAP_COORD_COMPONENT);
        if(!compNum)
        {
            it = m_entitiesDetectedData.erase(it);
            continue;
        }
        MapCoordComponent &mapComp = m_componentsContainer.m_vectMapCoordComp[*compNum];
        if(checkBoundEntityMap(mapComp, min, max))
        {
            //get absolute position corner
            corner = getUpLeftCorner(mapComp, it->first);
            m_entitiesToDisplay.emplace_back(it->first);
            diffPosPX = corner - mapCompPlayer.m_absoluteMapPositionPX;
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
    OptUint_t numCom;
    for(uint32_t i = 0; i < m_entitiesToDisplay.size(); ++i)
    {
        numCom = m_newComponentManager.getComponentEmplacement(m_entitiesToDisplay[i], Components_e::POSITION_VERTEX_COMPONENT);
        assert(numCom);
        PositionVertexComponent &posComp = m_componentsContainer.m_vectPositionVertexComp[*numCom];
        numCom = m_newComponentManager.getComponentEmplacement(m_entitiesToDisplay[i], Components_e::SPRITE_TEXTURE_COMPONENT);
        if(!numCom)
        {
            numCom = m_newComponentManager.getComponentEmplacement(m_entitiesToDisplay[i], Components_e::GENERAL_COLLISION_COMPONENT);
            assert(numCom);
            GeneralCollisionComponent &genComp = m_componentsContainer.m_vectGeneralCollisionComp[*numCom];
            if(genComp.m_tagA == CollisionTag_e::TRIGGER_CT)
            {
                continue;
            }
        }
        SpriteTextureComponent &spriteComp = m_componentsContainer.m_vectSpriteTextureComp[*numCom];
        assert(spriteComp.m_spriteData->m_textureNum < m_vectMapVerticesData.size());
        m_vectMapVerticesData[spriteComp.m_spriteData->m_textureNum].
                loadVertexStandartTextureComponent(posComp, spriteComp);
    }
}

//===================================================================
void MapDisplaySystem::fillFullMapVertexFromEntities()
{
    OptUint_t numCom;
    for(uint32_t h = 0; h < m_vectMapVerticesData.size(); ++h)
    {
        m_vectMapVerticesData[h].clear();
    }
    for(std::map<uint32_t, PairUI_t>::const_iterator it = m_entitiesDetectedData.begin();
         it != m_entitiesDetectedData.end(); ++it)
    {
        numCom = m_newComponentManager.getComponentEmplacement(it->first, Components_e::POSITION_VERTEX_COMPONENT);
        assert(numCom);
        PositionVertexComponent &posComp = m_componentsContainer.m_vectPositionVertexComp[*numCom];
        numCom = m_newComponentManager.getComponentEmplacement(it->first, Components_e::SPRITE_TEXTURE_COMPONENT);
        if(!numCom)
        {
            numCom = m_newComponentManager.getComponentEmplacement(it->first, Components_e::GENERAL_COLLISION_COMPONENT);
            assert(numCom);
            GeneralCollisionComponent &genComp = m_componentsContainer.m_vectGeneralCollisionComp[*numCom];
            if(genComp.m_tagA == CollisionTag_e::TRIGGER_CT)
            {
                continue;
            }
        }
        SpriteTextureComponent &spriteComp = m_componentsContainer.m_vectSpriteTextureComp[*numCom];
        assert(spriteComp.m_spriteData->m_textureNum < m_vectMapVerticesData.size());
        m_vectMapVerticesData[spriteComp.m_spriteData->m_textureNum].
                loadVertexStandartTextureComponent(posComp, spriteComp);
    }
}

//===================================================================
PairFloat_t MapDisplaySystem::getUpLeftCorner(const MapCoordComponent &mapCoordComp, uint32_t entityNum)
{
    OptUint_t numCom = m_newComponentManager.getComponentEmplacement(entityNum, Components_e::GENERAL_COLLISION_COMPONENT);
    assert(numCom);
    GeneralCollisionComponent &genCollComp = m_componentsContainer.m_vectGeneralCollisionComp[*numCom];
    if(genCollComp.m_shape == CollisionShape_e::CIRCLE_C)
    {
        numCom = m_newComponentManager.getComponentEmplacement(entityNum, Components_e::CIRCLE_COLLISION_COMPONENT);
        assert(numCom);
        CircleCollisionComponent &circleCollComp = m_componentsContainer.m_vectCircleCollisionComp[*numCom];
        return getCircleUpLeftCorner(mapCoordComp.m_absoluteMapPositionPX, circleCollComp.m_ray);
    }
    else
    {
        if(genCollComp.m_tagA == CollisionTag_e::DOOR_CT)
        {
            return {mapCoordComp.m_absoluteMapPositionPX.first - std::fmod(mapCoordComp.m_absoluteMapPositionPX.first, LEVEL_TILE_SIZE_PX),
                        mapCoordComp.m_absoluteMapPositionPX.second - std::fmod(mapCoordComp.m_absoluteMapPositionPX.second, LEVEL_TILE_SIZE_PX)};
        }
        else
        {
            return mapCoordComp.m_absoluteMapPositionPX;
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
    OptUint_t numCom = m_newComponentManager.getComponentEmplacement(entityNum, Components_e::POSITION_VERTEX_COMPONENT);
    assert(numCom);
    PositionVertexComponent &posComp = m_componentsContainer.m_vectPositionVertexComp[*numCom];
    posComp.m_vertex.resize(4);
    //CONSIDER THAT MAP X AND Y ARE THE SAME
    if(posComp.m_vertex.empty())
    {
        posComp.m_vertex.resize(4);
    }
    posComp.m_vertex[0] = {MAP_LOCAL_CENTER_X_GL + glPosition.first,
                            MAP_LOCAL_CENTER_Y_GL + glPosition.second};
    posComp.m_vertex[1] = {MAP_LOCAL_CENTER_X_GL + glPosition.first + m_miniMapTileSizeGL,
                            MAP_LOCAL_CENTER_Y_GL + glPosition.second};
    posComp.m_vertex[2] = {MAP_LOCAL_CENTER_X_GL + glPosition.first + m_miniMapTileSizeGL,
                            MAP_LOCAL_CENTER_Y_GL + glPosition.second - m_miniMapTileSizeGL};
    posComp.m_vertex[3] = {MAP_LOCAL_CENTER_X_GL + glPosition.first,
                            MAP_LOCAL_CENTER_Y_GL + glPosition.second - m_miniMapTileSizeGL};
}

//===================================================================
void MapDisplaySystem::confFullMapVertexElement(const PairFloat_t &absolutePositionPX, uint32_t entityNum)
{
    OptUint_t numCom = m_newComponentManager.getComponentEmplacement(entityNum, Components_e::POSITION_VERTEX_COMPONENT);
    assert(numCom);
    PositionVertexComponent &posComp = m_componentsContainer.m_vectPositionVertexComp[*numCom];
    posComp.m_vertex.resize(4);
    //CONSIDER THAT MAP X AND Y ARE THE SAME
    if(posComp.m_vertex.empty())
    {
        posComp.m_vertex.resize(4);
    }
    double leftPos = MAP_FULL_TOP_LEFT_X_GL +
            ((absolutePositionPX.first / m_sizeLevelPX.first) * FULL_MAP_SIZE_GL),
            rightPos = leftPos + m_fullMapTileSizeGL.first,
            topPos = MAP_FULL_TOP_LEFT_Y_GL -
            ((absolutePositionPX.second  / m_sizeLevelPX.second) * FULL_MAP_SIZE_GL),
            downPos = topPos - m_fullMapTileSizeGL.second;
    posComp.m_vertex[0] = {leftPos, topPos};
    posComp.m_vertex[1] = {rightPos, topPos};
    posComp.m_vertex[2] = {rightPos, downPos};
    posComp.m_vertex[3] = {leftPos, downPos};
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
    OptUint_t compNum = m_newComponentManager.getComponentEmplacement(m_playerNum, Components_e::VISION_COMPONENT);
    assert(compNum);
    VisionComponent &visionComp = m_componentsContainer.m_vectVisionComp[*compNum];
    mptrSystemManager->searchSystemByType<ColorDisplaySystem>(
                static_cast<uint32_t>(Systems_e::COLOR_DISPLAY_SYSTEM))->drawEntity(&visionComp.m_positionVertexComp,
                                                             &visionComp.m_colorVertexComp);
}

//===================================================================
void MapDisplaySystem::drawPlayerOnMap()
{
    OptUint_t compNum = m_newComponentManager.getComponentEmplacement(m_playerNum, Components_e::POSITION_VERTEX_COMPONENT);
    assert(compNum);
    PositionVertexComponent &posComp = m_componentsContainer.m_vectPositionVertexComp[*compNum];
    compNum = m_newComponentManager.getComponentEmplacement(m_playerNum, Components_e::COLOR_VERTEX_COMPONENT);
    assert(compNum);
    ColorVertexComponent &colorComp = m_componentsContainer.m_vectColorVertexComp[*compNum];
    mptrSystemManager->searchSystemByType<ColorDisplaySystem>(
                static_cast<uint32_t>(Systems_e::COLOR_DISPLAY_SYSTEM))->drawEntity(&posComp, &colorComp);
}

//===================================================================
void MapDisplaySystem::confPlayerComp(uint32_t playerNum)
{
    m_playerNum = playerNum;
    OptUint_t compNum = m_newComponentManager.getComponentEmplacement(m_playerNum, Components_e::VISION_COMPONENT);
    assert(compNum);
    VisionComponent &visionComp = m_componentsContainer.m_vectVisionComp[*compNum];
    visionComp.m_colorVertexComp.m_vertex.reserve(3);
    visionComp.m_colorVertexComp.m_vertex.emplace_back(0.00f, 100.00f, 0.00f, 1.0f);
    visionComp.m_colorVertexComp.m_vertex.emplace_back(0.00f, 10.00f, 0.00f, 1.0f);
    visionComp.m_colorVertexComp.m_vertex.emplace_back(0.00f, 10.00f, 0.00f, 1.0f);
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
