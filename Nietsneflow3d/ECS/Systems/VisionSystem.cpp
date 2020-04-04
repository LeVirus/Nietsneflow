#include <cassert>
#include "VisionSystem.hpp"
#include <constants.hpp>
#include <CollisionUtils.hpp>
#include <ECS/ECSManager.hpp>
#include <ECS/Components/PositionVertexComponent.hpp>
#include <ECS/Components/MapCoordComponent.hpp>
#include <ECS/Components/GeneralCollisionComponent.hpp>
#include <ECS/Components/VisionComponent.hpp>
#include <ECS/Components/CircleCollisionComponent.hpp>
#include <ECS/Components/RectangleCollisionComponent.hpp>
#include <ECS/Components/PositionVertexComponent.hpp>

//===========================================================================
VisionSystem::VisionSystem(const ECSManager *memECSManager) :
    m_memECSManager(memECSManager)
{
    setUsedComponents();
}

//===========================================================================
void VisionSystem::setUsedComponents()
{
    bAddComponentToSystem(Components_e::VISION_COMPONENT);
}

//===========================================================================
void VisionSystem::execSystem()
{
    System::execSystem();
    for(uint32_t i = 0; i < mVectNumEntity.size(); ++i)
    {
        GeneralCollisionComponent *collComp = stairwayToComponentManager().
                searchComponentByType<GeneralCollisionComponent>(mVectNumEntity[i],
                                      Components_e::GENERAL_COLLISION_COMPONENT);
        assert(collComp);
        std::bitset<Components_e::TOTAL_COMPONENTS> bitsetComp;
        bitsetComp[Components_e::MAP_COORD_COMPONENT] = true;
        std::vector<uint32_t> vectEntities = m_memECSManager->getEntityContainingComponents(bitsetComp);
        for(uint32_t j = 0; j < vectEntities.size(); ++j)
        {
            if(mVectNumEntity[i] == vectEntities[j])
            {
                continue;
            }
            GeneralCollisionComponent *collCompB = stairwayToComponentManager().
                    searchComponentByType<GeneralCollisionComponent>(vectEntities[j],
                                          Components_e::GENERAL_COLLISION_COMPONENT);
            assert(collCompB);
            if(collComp->m_tag == CollisionTag_e::ENEMY_CT && collCompB->m_tag != CollisionTag_e::PLAYER_CT)
            {
                continue;
            }
            treatVisible(mVectNumEntity[i], vectEntities[j], collCompB->m_shape);
        }
    }
}

//===========================================================================
void VisionSystem::treatVisible(uint32_t observerId, uint32_t checkVisibleId, CollisionShape_e shapeElement)
{
    VisionComponent *visionCompA = stairwayToComponentManager().
            searchComponentByType<VisionComponent>(observerId, Components_e::VISION_COMPONENT);
    MapCoordComponent *mapCompA = stairwayToComponentManager().
            searchComponentByType<MapCoordComponent>(observerId, Components_e::MAP_COORD_COMPONENT);
    MapCoordComponent *mapCompB = stairwayToComponentManager().
            searchComponentByType<MapCoordComponent>(checkVisibleId, Components_e::MAP_COORD_COMPONENT);
    assert(visionCompA);
    assert(mapCompA);
    assert(mapCompB);

    //maj triangle a faire
    switch(shapeElement)
    {
    case CollisionShape_e::CIRCLE_C:
    {
        CircleCollisionComponent *circleColl = stairwayToComponentManager().
                searchComponentByType<CircleCollisionComponent>(checkVisibleId, Components_e::CIRCLE_COLLISION_COMPONENT);
        assert(circleColl);
        if(checkTriangleCircleCollision(visionCompA->m_triangleVision, circleColl->m_center, circleColl->m_ray))
        {
            visionCompA->m_vectVisibleEntities.push_back(checkVisibleId);
        }
    }
        break;
    case CollisionShape_e::RECTANGLE_C:
    {
        RectangleCollisionComponent *rectColl = stairwayToComponentManager().
                searchComponentByType<RectangleCollisionComponent>(checkVisibleId, Components_e::RECTANGLE_COLLISION_COMPONENT);
        assert(rectColl);
        if(checkTriangleRectCollision(visionCompA->m_triangleVision, {mapCompB->m_absoluteMapPositionPX, rectColl->m_size}))
        {
            visionCompA->m_vectVisibleEntities.push_back(checkVisibleId);
        }
    }
        break;
    case CollisionShape_e::SEGMENT_C:
        break;
    }
}

