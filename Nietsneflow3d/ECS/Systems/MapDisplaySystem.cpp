#include "MapDisplaySystem.hpp"
#include "Level.hpp"
#include "PictureData.hpp"
#include <ECS/Components/PositionVertexComponent.hpp>
#include <ECS/Components/MapCoordComponent.hpp>
#include <ECS/Components/SpriteTextureComponent.hpp>
#include <ECS/Components/ColorVertexComponent.hpp>
#include <ECS/Components/MoveableComponent.hpp>
#include <ECS/Systems/ColorDisplaySystem.hpp>
#include <constants.hpp>

//===================================================================
MapDisplaySystem::MapDisplaySystem()
//WARNING CONSIDER THAT LENGHT AND WEIGHT ARE THE SAME
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
    MapCoordComponent *playerMapCoordComp = stairwayToComponentManager().
            searchComponentByType<MapCoordComponent>(m_playerNum,
                                                     Components_e::MAP_COORD_COMPONENT);
    assert(playerMapCoordComp);
    pairFloat_t playerPos = playerMapCoordComp->m_absoluteMapPositionPX;
    pairUI_t max, min;
    getMapDisplayLimit(playerPos, min, max);
    m_entitiesToDisplay.clear();
    m_entitiesToDisplay.reserve(20);
    for(uint32_t i = 0; i < mVectNumEntity.size(); ++i)
    {
        MapCoordComponent *mapComp = stairwayToComponentManager().
                searchComponentByType<MapCoordComponent>(mVectNumEntity[i],
                                                         Components_e::MAP_COORD_COMPONENT);
        assert(mapComp);

        if(checkBoundEntityMap(*mapComp, min, max))
        {
            m_entitiesToDisplay.emplace_back(mVectNumEntity[i]);

            pairFloat_t diffPosPX = mapComp->m_absoluteMapPositionPX -
                    playerMapCoordComp->m_absoluteMapPositionPX;
            pairFloat_t relativePosMapGL = {diffPosPX.first * MAP_LOCAL_SIZE_GL / m_localLevelSizePX,
                                            diffPosPX.second * MAP_LOCAL_SIZE_GL / m_localLevelSizePX};
            confVertexElement(relativePosMapGL, mVectNumEntity[i]);

        }
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
    max = Level::getLevelCoord(playerPos);
    playerPos.first -= rangeView * 2;
    playerPos.second -= rangeView * 2;
    min = Level::getLevelCoord(playerPos);
}

//===================================================================
void MapDisplaySystem::confVertexElement(const pairFloat_t &glPosition,
                                         uint32_t entityNum)
{
    PositionVertexComponent *posComp = stairwayToComponentManager().
            searchComponentByType<PositionVertexComponent>(entityNum,
                                                           Components_e::POSITION_VERTEX_COMPONENT);
    assert(posComp);
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
    MapCoordComponent *mapComp = stairwayToComponentManager().
            searchComponentByType<MapCoordComponent>(m_playerNum,
                                                     Components_e::MAP_COORD_COMPONENT);
    assert(mapComp);
    return true;
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
        assert(spriteComp->m_spriteData->m_textureNum < m_vectVerticesData.size());
        m_vectVerticesData[spriteComp->m_spriteData->m_textureNum].loadVertexTextureComponent(*posComp, *spriteComp);
    }
}

//===================================================================
void MapDisplaySystem::drawVertex()
{
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
void MapDisplaySystem::drawPlayerOnMap()
{
    PositionVertexComponent *posComp = stairwayToComponentManager().
            searchComponentByType<PositionVertexComponent>(m_playerNum,
                                                           Components_e::POSITION_VERTEX_COMPONENT);
    ColorVertexComponent *colorComp = stairwayToComponentManager().
            searchComponentByType<ColorVertexComponent>(m_playerNum,
                                                           Components_e::COLOR_VERTEX_COMPONENT);
    assert(posComp);
    assert(colorComp);
    mptrSystemManager->searchSystemByType<ColorDisplaySystem>(
                Systems_e::COLOR_DISPLAY_SYSTEM)->drawEntity(posComp, colorComp);
}
