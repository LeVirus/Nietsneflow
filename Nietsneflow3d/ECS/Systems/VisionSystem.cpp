#include <cassert>
#include "VisionSystem.hpp"
#include <constants.hpp>
#include <CollisionUtils.hpp>
#include <PhysicalEngine.hpp>
#include <ECS/ECSManager.hpp>
#include <math.h>
#include <ECS/Components/PositionVertexComponent.hpp>
#include <ECS/Components/MapCoordComponent.hpp>
#include <ECS/Components/GeneralCollisionComponent.hpp>
#include <ECS/Components/VisionComponent.hpp>
#include <ECS/Components/CircleCollisionComponent.hpp>
#include <ECS/Components/RectangleCollisionComponent.hpp>
#include <ECS/Components/PositionVertexComponent.hpp>
#include <ECS/Components/MoveableComponent.hpp>

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

        VisionComponent *visionCompA = stairwayToComponentManager().
                searchComponentByType<VisionComponent>(mVectNumEntity[i], Components_e::VISION_COMPONENT);
        MapCoordComponent *mapCompA = stairwayToComponentManager().
                searchComponentByType<MapCoordComponent>(mVectNumEntity[i], Components_e::MAP_COORD_COMPONENT);
        MoveableComponent *movCompA = stairwayToComponentManager().
                searchComponentByType<MoveableComponent>(mVectNumEntity[i], Components_e::MOVEABLE_COMPONENT);
        updateTriangleVisionFromPosition(visionCompA, mapCompA, movCompA);
        visionCompA->m_vectVisibleEntities.clear();
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
            //FAIRE DES TESTS POUR LES COLLISIONS
            treatVisible(visionCompA, vectEntities[j], collCompB->m_shape);
        }
    }
}

//===========================================================================
void VisionSystem::treatVisible(VisionComponent *visionComp, uint32_t checkVisibleId, CollisionShape_e shapeElement)
{
    MapCoordComponent *mapCompB = stairwayToComponentManager().
            searchComponentByType<MapCoordComponent>(checkVisibleId, Components_e::MAP_COORD_COMPONENT);
    assert(visionComp);
    assert(mapCompB);

    switch(shapeElement)
    {
    case CollisionShape_e::CIRCLE_C:
    {
        CircleCollisionComponent *circleColl = stairwayToComponentManager().
                searchComponentByType<CircleCollisionComponent>(checkVisibleId, Components_e::CIRCLE_COLLISION_COMPONENT);
        assert(circleColl);
        if(checkTriangleCircleCollision(visionComp->m_triangleVision, mapCompB->m_absoluteMapPositionPX, circleColl->m_ray))
        {
            visionComp->m_vectVisibleEntities.push_back(checkVisibleId);
        }
    }
        break;
    case CollisionShape_e::RECTANGLE_C:
    {
        RectangleCollisionComponent *rectColl = stairwayToComponentManager().
                searchComponentByType<RectangleCollisionComponent>(checkVisibleId, Components_e::RECTANGLE_COLLISION_COMPONENT);
        assert(rectColl);
        if(checkTriangleRectCollision(visionComp->m_triangleVision, {mapCompB->m_absoluteMapPositionPX, rectColl->m_size}))
        {
            visionComp->m_vectVisibleEntities.push_back(checkVisibleId);
        }
    }
        break;
    case CollisionShape_e::SEGMENT_C:
        break;
    }
}

//===========================================================================
void updateTriangleVisionFromPosition(VisionComponent *visionComp, const MapCoordComponent *mapComp,
                                      const MoveableComponent *movComp)
{
    assert(visionComp);
    assert(mapComp);
    assert(movComp);
    visionComp->m_triangleVision[0] = mapComp->m_absoluteMapPositionPX;
    visionComp->m_triangleVision[1] = mapComp->m_absoluteMapPositionPX;
    visionComp->m_triangleVision[2] = mapComp->m_absoluteMapPositionPX;
    //second point of view
    float angleDegree = movComp->m_degreeOrientation - (visionComp->m_coneVision / 2);
    float radiantAngle;
    for(uint32_t i = 1; i < 3; ++i)
    {
        radiantAngle = getRadiantAngle(angleDegree);
        visionComp->m_triangleVision[i].first += cos(radiantAngle) * visionComp->m_distanceVisibility;
        visionComp->m_triangleVision[i].second += sin(radiantAngle) * visionComp->m_distanceVisibility;
        if(i == 2)
        {
            break;
        }
        angleDegree += visionComp->m_coneVision;
    }
}
