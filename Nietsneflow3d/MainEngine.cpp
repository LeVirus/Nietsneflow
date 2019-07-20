#include "MainEngine.hpp"
#include <ECS/Components/PositionVertexComponent.hpp>
#include <ECS/Components/ColorVertexComponent.hpp>
#include <cassert>

//===================================================================
MainEngine::MainEngine()
{

}

//===================================================================
void MainEngine::init()
{
    m_ecsManager.init();
    linkSystemsToEngine();
    m_graphicEngine.confSystems();
}

//===================================================================
void MainEngine::loadECSEntities()
{

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
        uint32_t entityNum = m_ecsManager.addEntity(bitsetComponents);
        confGroundAndCeilingComponents(entityNum);
        ptr = &ceilingData;
    }
}

//===================================================================
void MainEngine::confGroundAndCeilingComponents(uint32_t entityNum)
{
    PositionVertexComponent *posComp = m_ecsManager.getComponentManager().
            searchComponentByType<PositionVertexComponent>(entityNum, Components_e::POSITION_VERTEX_COMPONENT);
    assert(posComp);
    posComp->m_vertex[0].first = 0.0f;
    posComp->m_vertex[0].second = 0.0f;
    posComp->m_vertex[1].first = 1.0f;
    posComp->m_vertex[1].second = 0.0f;
    posComp->m_vertex[2].first = 1.0f;
    posComp->m_vertex[2].second = 0.5f;
    posComp->m_vertex[3].first = 0.0f;
    posComp->m_vertex[3].second = 0.5f;

    ColorVertexComponent *colorComp = m_ecsManager.getComponentManager().
            searchComponentByType<ColorVertexComponent>(entityNum, Components_e::COLOR_VERTEX_COMPONENT);
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
void MainEngine::linkSystemsToEngine()
{
    m_graphicEngine.linkSystems(m_ecsManager.getSystemManager().
                    searchSystemByType<ColorDisplaySystem>(Systems_e::COLOR_DISPLAY_SYSTEM));
}
