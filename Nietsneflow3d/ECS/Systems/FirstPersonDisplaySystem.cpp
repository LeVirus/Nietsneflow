#include "FirstPersonDisplaySystem.hpp"
#include <ECS/Components/MapCoordComponent.hpp>
#include <ECS/Components/PositionVertexComponent.hpp>
#include <ECS/Components/ColorVertexComponent.hpp>

//===================================================================
FirstPersonDisplaySystem::FirstPersonDisplaySystem()
{

}

//===================================================================
void FirstPersonDisplaySystem::execSystem()
{
    System::execSystem();
}

//===================================================================
void FirstPersonDisplaySystem::confPlayerComp(uint32_t playerNum)
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
    assert(m_playerComp.m_posComp);
    assert(m_playerComp.m_colorComp);
    assert(m_playerComp.m_mapCoordComp);
}

