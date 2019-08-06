#include "MainEngine.hpp"
#include <ECS/Components/PositionVertexComponent.hpp>
#include <ECS/Components/ColorVertexComponent.hpp>
#include <ECS/Components/NumberVertexComponent.hpp>
#include <ECS/Systems/ColorDisplaySystem.hpp>
#include <ECS/Systems/MapDisplaySystem.hpp>
#include <Level.hpp>
#include <cassert>

//===================================================================
MainEngine::MainEngine()
{

}

//===================================================================
void MainEngine::init()
{
    m_ecsManager.init();
    linkSystemsToGraphicEngine();
    m_graphicEngine.confSystems();
}

//===================================================================
void MainEngine::launchLoop()
{
    do
    {
        m_graphicEngine.runIteration();
    }while(!m_graphicEngine.windowShouldClose());
}

//===================================================================
void MainEngine::loadGraphicPicture(const PictureData &picData)
{
    m_graphicEngine.loadPictureData(picData);
    loadGroundAndCeilingEntities(picData.getGroundData(), picData.getCeilingData());
}


//===================================================================
void MainEngine::loadGroundAndCeilingEntities(const GroundCeilingData &groundData,
                                         const GroundCeilingData &ceilingData)
{
    GroundCeilingData const *ptr = &groundData;
    for(uint32_t i = 0; i < 2; ++i)
    {
        std::bitset<Components_e::TOTAL_COMPONENTS> bitsetComponents;
        bitsetComponents[Components_e::POSITION_VERTEX_COMPONENT] = true;
        bitsetComponents[Components_e::NUMBER_VERTEX_COMPONENT] = true;
        if(ptr->m_apparence == DisplayType_e::COLOR)
        {
            bitsetComponents[Components_e::COLOR_VERTEX_COMPONENT] = true;
        }
        else if(ptr->m_apparence == DisplayType_e::TEXTURE)
        {
            bitsetComponents[Components_e::SPRITE_TEXTURE_COMPONENT] = true;
        }
        else
        {
            bitsetComponents[Components_e::SPRITE_TEXTURE_COMPONENT] = true;
            bitsetComponents[Components_e::COLOR_VERTEX_COMPONENT] = true;
        }
        uint32_t entityNum = m_ecsManager.addEntity(bitsetComponents, BaseShapeType_e::RECTANGLE);
        if(i)
        {
            confGroundComponents(entityNum);
        }
        else
        {
            confCeilingComponents(entityNum);
        }
        ptr = &ceilingData;
    }
}

//===================================================================
void MainEngine::loadLevelEntities(const Level &level)
{
    const std::vector<WallData> &wallData = level.getWallData();
    for(uint32_t i = 0; i < wallData.size(); ++i)
    {

    }
}

//===================================================================
void MainEngine::confGroundComponents(uint32_t entityNum)
{
    PositionVertexComponent<4> *posComp = m_ecsManager.getComponentManager().
            searchComponentByType<PositionVertexComponent<4>>(entityNum, Components_e::POSITION_VERTEX_COMPONENT);
    assert(posComp);
    posComp->m_vertex[0].first  = -1.0f;
    posComp->m_vertex[0].second = 0.0f;
    posComp->m_vertex[1].first  = 1.0f;
    posComp->m_vertex[1].second = 0.0f;
    posComp->m_vertex[2].first  = 1.0f;
    posComp->m_vertex[2].second = -1.0f;
    posComp->m_vertex[3].first  = -1.0f;
    posComp->m_vertex[3].second = -1.0f;

    ColorVertexComponent<4> *colorComp = m_ecsManager.getComponentManager().
            searchComponentByType<ColorVertexComponent<4>>(entityNum, Components_e::COLOR_VERTEX_COMPONENT);
    assert(colorComp);
    std::get<0>(colorComp->m_vertex[0]) = 0.1f;
    std::get<1>(colorComp->m_vertex[0]) = 0.2f;
    std::get<2>(colorComp->m_vertex[0]) = 0.3f;
    std::get<0>(colorComp->m_vertex[1]) = 1.0f;
    std::get<1>(colorComp->m_vertex[1]) = 0.8f;
    std::get<2>(colorComp->m_vertex[1]) = 0.1f;
    std::get<0>(colorComp->m_vertex[2]) = 0.3f;
    std::get<1>(colorComp->m_vertex[2]) = 0.9f;
    std::get<2>(colorComp->m_vertex[2]) = 0.8f;
    std::get<0>(colorComp->m_vertex[3]) = 0.6f;
    std::get<1>(colorComp->m_vertex[3]) = 0.5f;
    std::get<2>(colorComp->m_vertex[3]) = 0.1f;
}

//===================================================================
void MainEngine::confCeilingComponents(uint32_t entityNum)
{
    PositionVertexComponent<4> *posComp = m_ecsManager.getComponentManager().
            searchComponentByType<PositionVertexComponent<4>>(entityNum, Components_e::POSITION_VERTEX_COMPONENT);
    assert(posComp);
    posComp->m_vertex[0].first  = -1.0f;
    posComp->m_vertex[0].second = 1.0f;
    posComp->m_vertex[1].first  = 1.0f;
    posComp->m_vertex[1].second = 1.0f;
    posComp->m_vertex[2].first  = 1.0f;
    posComp->m_vertex[2].second = 0.0f;
    posComp->m_vertex[3].first  = -1.0f;
    posComp->m_vertex[3].second = 0.0f;

    ColorVertexComponent<4> *colorComp = m_ecsManager.getComponentManager().
            searchComponentByType<ColorVertexComponent<4>>(entityNum, Components_e::COLOR_VERTEX_COMPONENT);
    assert(colorComp);
    std::get<0>(colorComp->m_vertex[0]) = 0.5f;
    std::get<1>(colorComp->m_vertex[0]) = 0.5f;
    std::get<2>(colorComp->m_vertex[0]) = 0.5f;
    std::get<0>(colorComp->m_vertex[1]) = 0.0f;
    std::get<1>(colorComp->m_vertex[1]) = 0.5f;
    std::get<2>(colorComp->m_vertex[1]) = 0.5f;
    std::get<0>(colorComp->m_vertex[2]) = 0.5f;
    std::get<1>(colorComp->m_vertex[2]) = 0.5f;
    std::get<2>(colorComp->m_vertex[2]) = 0.0f;
    std::get<0>(colorComp->m_vertex[3]) = 0.5f;
    std::get<1>(colorComp->m_vertex[3]) = 0.0f;
    std::get<2>(colorComp->m_vertex[3]) = 0.5f;
}

//===================================================================
void MainEngine::linkSystemsToGraphicEngine()
{
    ColorDisplaySystem *color = m_ecsManager.getSystemManager().
            searchSystemByType<ColorDisplaySystem>(Systems_e::COLOR_DISPLAY_SYSTEM);
    MapDisplaySystem *map = m_ecsManager.getSystemManager().
            searchSystemByType<MapDisplaySystem>(Systems_e::MAP_DISPLAY_SYSTEM);

    m_graphicEngine.linkSystems(color, map);
}
