#include "MainEngine.hpp"

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
        m_ecsManager.addEntity(bitsetComponents);//CRASH HERE
        ptr = &ceilingData;
    }
}

//===================================================================
void MainEngine::linkSystemsToEngine()
{
    m_graphicEngine.linkSystems(m_ecsManager.getSystemManager().
                    searchSystemByType<ColorDisplaySystem>(Systems_e::COLOR_DISPLAY_SYSTEM));
}
