#include "FirstPersonDisplaySystem.hpp"
#include <CollisionUtils.hpp>
#include <ECS/Components/VisionComponent.hpp>
#include <ECS/Components/GeneralCollisionComponent.hpp>

//===================================================================
FirstPersonDisplaySystem::FirstPersonDisplaySystem()
{
    setUsedComponents();
}

//===================================================================
void FirstPersonDisplaySystem::setUsedComponents()
{
    bAddComponentToSystem(Components_e::VISION_COMPONENT);
}

//===================================================================
void FirstPersonDisplaySystem::execSystem()
{
    System::execSystem();
    createVerticesFromMemEntities();
    drawVertex();
}

//===================================================================
void FirstPersonDisplaySystem::createVerticesFromMemEntities()
{
    //treat one player
    for(uint32_t i = 0; i < mVectNumEntity.size(); ++i)
    {
        VisionComponent *visionComp = stairwayToComponentManager().
                searchComponentByType<VisionComponent>(mVectNumEntity[i], Components_e::VISION_COMPONENT);
        assert(visionComp);
        for(uint32_t j = 0; j < visionComp->m_vectVisibleEntities.size(); ++j)
        {
            GeneralCollisionComponent *genCollComp = stairwayToComponentManager().
                    searchComponentByType<GeneralCollisionComponent>(mVectNumEntity[i], Components_e::GENERAL_COLLISION_COMPONENT);
            assert(genCollComp);
            switch(genCollComp->m_shape)
            {
            case CollisionShape_e::CIRCLE_C:
                break;
            case CollisionShape_e::RECTANGLE_C:
                break;
            case CollisionShape_e::SEGMENT_C:
                break;
            }
        }
    }
}

//===================================================================
void FirstPersonDisplaySystem::drawVertex()
{

}


