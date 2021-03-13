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
#include <ECS/Components/CircleCollisionComponent.hpp>
#include <ECS/Components/RectangleCollisionComponent.hpp>
#include <ECS/Components/VisionComponent.hpp>
#include <ECS/Systems/ColorDisplaySystem.hpp>
#include <constants.hpp>

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
    m_levelSizePX = Level::getSize().first * LEVEL_TILE_SIZE_PX;
    m_tileSizeGL = (LEVEL_TILE_SIZE_PX * MAP_LOCAL_SIZE_GL) / m_localLevelSizePX;
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
    System::execSystem();
    confPositionVertexEntities();
    fillVertexFromEntities();
    drawVertex();
    drawPlayerOnMap();
}

//===================================================================
void MapDisplaySystem::confPositionVertexEntities()
{
    assert(m_playerComp.m_mapCoordComp);
    pairFloat_t playerPos = m_playerComp.m_mapCoordComp->m_absoluteMapPositionPX;
    pairUI_t max, min;
    getMapDisplayLimit(playerPos, min, max);
    m_entitiesToDisplay.clear();
    m_entitiesToDisplay.reserve(mVectNumEntity.size());
    for(uint32_t i = 0; i < mVectNumEntity.size(); ++i)
    {
        MapCoordComponent *mapComp = stairwayToComponentManager().
                searchComponentByType<MapCoordComponent>(mVectNumEntity[i],
                                                         Components_e::MAP_COORD_COMPONENT);
        assert(mapComp);
        //get absolute position corner
        if(checkBoundEntityMap(*mapComp, min, max))
        {
            pairFloat_t corner = getUpLeftCorner(mapComp, mVectNumEntity[i]);
            m_entitiesToDisplay.emplace_back(mVectNumEntity[i]);
            pairFloat_t diffPosPX = corner - m_playerComp.m_mapCoordComp->m_absoluteMapPositionPX;
            //convert absolute position to relative
            pairFloat_t relativePosMapGL = {diffPosPX.first * MAP_LOCAL_SIZE_GL / m_localLevelSizePX,
                                            diffPosPX.second * MAP_LOCAL_SIZE_GL / m_localLevelSizePX};
            confVertexElement(relativePosMapGL, mVectNumEntity[i]);
        }
    }
}

//===================================================================
void MapDisplaySystem::fillVertexFromEntities()
{
    for(uint32_t h = 0; h < m_vectVerticesData.size(); ++h)
    {
        m_vectVerticesData[h].clear();
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
        assert(spriteComp);
        assert(static_cast<size_t>(spriteComp->m_spriteData->m_textureNum) < m_vectVerticesData.size());
        m_vectVerticesData[static_cast<size_t>(spriteComp->m_spriteData->m_textureNum)].
                loadVertexStandartTextureComponent(*posComp, *spriteComp);
    }
}

//===================================================================
pairFloat_t MapDisplaySystem::getUpLeftCorner(const MapCoordComponent *mapCoordComp, uint32_t entityNum)
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
        return mapCoordComp->m_absoluteMapPositionPX;//tmp
    }
}

//===================================================================
void MapDisplaySystem::getMapDisplayLimit(pairFloat_t &playerPos,
                                          pairUI_t &min, pairUI_t &max)
{
    //getBound
    float rangeView = Level::getRangeView();
    playerPos.first += rangeView;
    playerPos.second += rangeView;
    max = *getLevelCoord(playerPos);
    playerPos.first -= rangeView * 2;
    playerPos.second -= rangeView * 2;
    min = *getLevelCoord(playerPos);
}


//===================================================================
void MapDisplaySystem::confVertexElement(const pairFloat_t &glPosition,
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
    posComp->m_vertex[1] = {MAP_LOCAL_CENTER_X_GL + glPosition.first + m_tileSizeGL,
                            MAP_LOCAL_CENTER_Y_GL + glPosition.second};
    posComp->m_vertex[2] = {MAP_LOCAL_CENTER_X_GL + glPosition.first + m_tileSizeGL,
                            MAP_LOCAL_CENTER_Y_GL + glPosition.second - m_tileSizeGL};
    posComp->m_vertex[3] = {MAP_LOCAL_CENTER_X_GL + glPosition.first,
                            MAP_LOCAL_CENTER_Y_GL + glPosition.second - m_tileSizeGL};
}

//===================================================================
bool MapDisplaySystem::checkBoundEntityMap(const MapCoordComponent &mapCoordComp,
                                           const pairUI_t &minBound,
                                           const pairUI_t &maxBound)
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
void MapDisplaySystem::drawVertex()
{
    drawPlayerVision();
    m_shader->use();
    for(uint32_t h = 0; h < m_vectVerticesData.size(); ++h)
    {
        m_ptrVectTexture->operator[](h).bind();
        m_vectVerticesData[h].confVertexBuffer();
        m_vectVerticesData[h].drawElement();
    }
    drawPlayerOnMap();
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
    VisionComponent *visionComp = stairwayToComponentManager().
            searchComponentByType<VisionComponent>(playerNum,
                                                   Components_e::VISION_COMPONENT);
    assert(visionComp);
    visionComp->m_colorVertexComp.m_vertex.reserve(3);
    visionComp->m_colorVertexComp.m_vertex.emplace_back(0.00f, 100.00f, 0.00f);
    visionComp->m_colorVertexComp.m_vertex.emplace_back(0.00f, 10.00f, 0.00f);
    visionComp->m_colorVertexComp.m_vertex.emplace_back(0.00f, 10.00f, 0.00f);
    m_playerComp.m_visionComp = visionComp;
    assert(m_playerComp.m_posComp);
    assert(m_playerComp.m_colorComp);
    assert(m_playerComp.m_mapCoordComp);
}

//===================================================================
void MapDisplaySystem::setVectTextures(std::vector<Texture> &vectTexture)
{
    m_ptrVectTexture = &vectTexture;
    m_vectVerticesData.reserve(vectTexture.size());
    for(uint32_t h = 0; h < vectTexture.size(); ++h)
    {
        m_vectVerticesData.emplace_back(VerticesData(Shader_e::TEXTURE_S));
    }
}
