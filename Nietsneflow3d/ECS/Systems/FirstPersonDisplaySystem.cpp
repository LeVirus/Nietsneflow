#include "FirstPersonDisplaySystem.hpp"
#include <CollisionUtils.hpp>
#include <ECS/Components/MapCoordComponent.hpp>
#include <ECS/Components/PositionVertexComponent.hpp>
#include <ECS/Components/ColorVertexComponent.hpp>
#include <ECS/Components/GeneralCollisionComponent.hpp>
#include <ECS/Components/CircleCollisionComponent.hpp>
#include <ECS/Components/RectangleCollisionComponent.hpp>

//===================================================================
FirstPersonDisplaySystem::FirstPersonDisplaySystem()
{
    setUsedComponents();
}

//===================================================================
void FirstPersonDisplaySystem::setUsedComponents()
{
    bAddComponentToSystem(Components_e::POSITION_VERTEX_COMPONENT);
    bAddComponentToSystem(Components_e::SPRITE_TEXTURE_COMPONENT);
    bAddComponentToSystem(Components_e::MAP_COORD_COMPONENT);
}

//===================================================================
void FirstPersonDisplaySystem::excludeOutVisionEntities()
{
//    m_playerComp.m_mapCoordComp;
    array3PairFloat_t playerVisionTriangle;
    for(uint32_t i = 0; i < mVectNumEntity.size(); ++i)
    {
        if(mVectNumEntity[i] == m_memPlayerEntity)
        {
            continue;
        }
        MapCoordComponent *mapComp = stairwayToComponentManager().
                searchComponentByType<MapCoordComponent>(i, Components_e::MAP_COORD_COMPONENT);
        GeneralCollisionComponent *collComp = stairwayToComponentManager().
                searchComponentByType<GeneralCollisionComponent>(i, Components_e::GENERAL_COLLISION_COMPONENT);
        assert(mapComp);
        assert(collComp);
        switch (collComp->m_shape)
        {
        case CollisionShape_e::SEGMENT_C:
            break;
        case CollisionShape_e::CIRCLE_C:
        {
            CircleCollisionComponent *circle = stairwayToComponentManager().
                    searchComponentByType<CircleCollisionComponent>(i, Components_e::CIRCLE_COLLISION_COMPONENT);
            assert(circle);
        }
            break;
        case CollisionShape_e::RECTANGLE_C:
        {
            RectangleCollisionComponent *rectangle = stairwayToComponentManager().
                    searchComponentByType<RectangleCollisionComponent>(i, Components_e::RECTANGLE_COLLISION_COMPONENT);
            assert(rectangle);
        }
            break;
        }
    }
}

//===================================================================
void FirstPersonDisplaySystem::execSystem()
{
    System::execSystem();
    excludeOutVisionEntities();
}

//===================================================================
void FirstPersonDisplaySystem::confPlayerComp(uint32_t playerNum)
{
    m_memPlayerEntity = playerNum;
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

