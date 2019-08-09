#include "MapDisplaySystem.hpp"
#include "Level.hpp"
#include <ECS/Components/PositionVertexComponent.hpp>
#include <ECS/Components/MapCoordComponent.hpp>
#include <ECS/Components/SpriteTextureComponent.hpp>
#include <ECS/Components/ColorVertexComponent.hpp>
#include <ECS/Components/MoveableComponent.hpp>
#include <ECS/Systems/ColorDisplaySystem.hpp>
#include <constants.hpp>

//===================================================================
MapDisplaySystem::MapDisplaySystem():m_verticesData(Shader_e::TEXTURE_S)
//WARNING CONSIDER THAT LENGHT AND WEIGHT ARE THE SAME
{
    setUsedComponents();
}

//===================================================================
void MapDisplaySystem::confLevelData()
{
    m_levelSizePX = Level::getSize().first * LEVEL_TILE_SIZE_PX;
    m_halfTileSizeGL = (LEVEL_TILE_SIZE_PX / 2) * MAP_SIZE_GL / m_levelSizePX;
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
    confEntity();
    fillVertexFromEntities();
    drawVertex();
    drawPlayerOnMap();
}

//===================================================================
void MapDisplaySystem::confEntity()
{
    MoveableComponent *playerMoveableComp = stairwayToComponentManager().
            searchComponentByType<MoveableComponent>(m_playerNum,
                                                           Components_e::MOVEABLE_COMPONENT);
    assert(playerMoveableComp);
    //getBound
    float rangeView = Level::getRangeView();
    pairFloat_t playerPos = playerMoveableComp->m_absoluteMapPosition;
    playerPos.first += rangeView;
    playerPos.second += rangeView;
    pairUint_t max = Level::getLevelCoord(playerPos);
    playerPos.first -= rangeView * 2;
    playerPos.second -= rangeView * 2;
    pairUint_t min = Level::getLevelCoord(playerPos);

    m_entitiesToDisplay.clear();
    m_entitiesToDisplay.reserve(20);
    for(uint32_t i = 0; i < mVectNumEntity.size(); ++i)
    {

        MapCoordComponent *mapComp = stairwayToComponentManager().
                searchComponentByType<MapCoordComponent>(mVectNumEntity[i],
                                                               Components_e::MAP_COORD_COMPONENT);
        assert(mapComp);
//        if(checkBoundEntityMap(*mapComp, min, max))
//        {
        //DEBUG
//        if(i == 9){
            m_entitiesToDisplay.emplace_back(mVectNumEntity[i]);
            PositionVertexComponent *posComp = stairwayToComponentManager().
                    searchComponentByType<PositionVertexComponent>(mVectNumEntity[i],
                                                                   Components_e::POSITION_VERTEX_COMPONENT);
            assert(posComp);
            pairFloat_t absolutePositionElement =
                    Level::getAbsolutePosition(mapComp->m_coord);
            std::cerr << absolutePositionElement.first << " dddzze " << mapComp->m_coord.first << "\n";
            std::cerr << absolutePositionElement.second << " dddzze " << mapComp->m_coord.second << "\n\n";

            pairFloat_t diffPos = playerMoveableComp->m_absoluteMapPosition -
                    absolutePositionElement;
            std::cerr << diffPos.first << " aaaa " <<playerMoveableComp->m_absoluteMapPosition.first << "\n";
            std::cerr << diffPos.second << " aaaa " << playerMoveableComp->m_absoluteMapPosition.second << "\n\n";
            pairFloat_t relativePosCenterGL = {diffPos.first * MAP_SIZE_GL / m_levelSizePX,
                                              diffPos.second * MAP_SIZE_GL / m_levelSizePX};
            //CONSIDER THAT MAP X AND Y ARE THE SAME
            if(posComp->m_vertex.empty())
            {
                posComp->m_vertex.resize(4);
            }
            posComp->m_vertex[0] = {relativePosCenterGL.first - m_halfTileSizeGL,
                                         relativePosCenterGL.second + m_halfTileSizeGL};
            posComp->m_vertex[1] = {relativePosCenterGL.first + m_halfTileSizeGL,
                                         relativePosCenterGL.second + m_halfTileSizeGL};
            posComp->m_vertex[2] = {relativePosCenterGL.first + m_halfTileSizeGL,
                                         relativePosCenterGL.second - m_halfTileSizeGL};
            posComp->m_vertex[3] = {relativePosCenterGL.first - m_halfTileSizeGL,
                                         relativePosCenterGL.second - m_halfTileSizeGL};

            //TEST
//            posComp->m_vertex[0] = {0.5f, -0.5};
//            posComp->m_vertex[1] = {0.65f,-0.5};
//            posComp->m_vertex[2] = {0.65f,-0.6};
//            posComp->m_vertex[3] = {0.5f, -0.6};

            std::cerr << relativePosCenterGL.first << "ddd " << m_halfTileSizeGL << "\n";

            std::cerr << posComp->m_vertex[0].first << " " <<
                         posComp->m_vertex[0].second << "\n";
            std::cerr << posComp->m_vertex[1].first << " " <<
                         posComp->m_vertex[1].second << "\n";
            std::cerr << posComp->m_vertex[2].first << " " <<
                         posComp->m_vertex[2].second << "\n";
            std::cerr << posComp->m_vertex[3].first << " " <<
                         posComp->m_vertex[3].second << "\n\n";
//        }
    }
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
void MapDisplaySystem::fillVertexFromEntities()
{
    m_verticesData.clear();
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
        m_verticesData.loadVertexTextureComponent(*posComp, *spriteComp);
    }
}

//===================================================================
void MapDisplaySystem::drawVertex()
{
    m_verticesData.confVertexBuffer();
    m_shader->use();
    m_verticesData.drawElement();
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
