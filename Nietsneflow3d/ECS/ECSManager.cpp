#include <ECS/ECSManager.hpp>
#include <ECS/Components/PositionVertexComponent.hpp>
#include <ECS/Components/SpriteTextureComponent.hpp>
#include <ECS/Components/ColorVertexComponent.hpp>
#include <ECS/Components/MapCoordComponent.hpp>
#include <ECS/Components/StaticElementComponent.hpp>
#include <ECS/Components/MoveableComponent.hpp>
#include <ECS/Components/InputComponent.hpp>
#include <ECS/Components/GeneralCollisionComponent.hpp>
#include <ECS/Components/CircleCollisionComponent.hpp>
#include <ECS/Components/LineCollisionComponent.hpp>
#include <ECS/Components/RectangleCollisionComponent.hpp>
#include <ECS/Components/VisionComponent.hpp>
#include <ECS/Components/MemSpriteDataComponent.hpp>
#include <ECS/Components/TimerComponent.hpp>
#include <ECS/Systems/ColorDisplaySystem.hpp>
#include <ECS/Systems/MapDisplaySystem.hpp>
#include <ECS/Systems/InputSystem.hpp>
#include <ECS/Systems/CollisionSystem.hpp>
#include <ECS/Systems/FirstPersonDisplaySystem.hpp>
#include <ECS/Systems/VisionSystem.hpp>
#include <constants.hpp>
#include <memory>
#include <cassert>

//===================================================================
ECSManager::ECSManager():m_componentManager(&m_ecsEngine.getComponentManager()),
    m_systemManager(&m_ecsEngine.getSystemManager())
{
}

//===================================================================
void ECSManager::initComponents()
{
    assert(m_componentManager && "m_componentManager is null.");
    m_componentManager->addEmplacementsForExternComponent(Components_e::TOTAL_COMPONENTS);
}

//===================================================================
void ECSManager::initSystems()
{
    assert(m_systemManager && "m_systemManager is null.");
    m_systemManager->bAddExternSystem(std::make_unique<ColorDisplaySystem>());
    m_systemManager->bAddExternSystem(std::make_unique<MapDisplaySystem>());
    m_systemManager->bAddExternSystem(std::make_unique<InputSystem>());
    m_systemManager->bAddExternSystem(std::make_unique<CollisionSystem>());
    m_systemManager->bAddExternSystem(std::make_unique<FirstPersonDisplaySystem>());
    m_systemManager->bAddExternSystem(std::make_unique<VisionSystem>(this));
}


//===================================================================
void ECSManager::init()
{
    initComponents();
    initSystems();
}

//===================================================================
//Execute syncComponentsFromEntities before launching game
uint32_t ECSManager::addEntity(const std::bitset<Components_e::TOTAL_COMPONENTS> &bitsetComponents)
{
    uint32_t newEntity = m_ecsEngine.AddEntity();
    std::vector<Components_e> vectMemComponent;
    for(uint32_t i = 0; i < bitsetComponents.size(); ++i)
    {
        if(bitsetComponents[i])
        {
            m_ecsEngine.bAddComponentToEntity(newEntity, i);
            vectMemComponent.emplace_back(static_cast<Components_e>(i));
        }
    }
    syncComponentsFromEntities(newEntity, vectMemComponent);
    return newEntity;
}

//===================================================================
std::vector<uint32_t> ECSManager::getEntityContainingComponents(const std::bitset<TOTAL_COMPONENTS> &bitsetComponents) const
{
    std::vector<uint32_t> vectReturn;
    const std::vector<ecs::Entity> &vectEntity = m_ecsEngine.getVectEntity();
    bool ok = true;
    for(uint32_t i = 0; i < vectEntity.size(); ++i)
    {
        const std::vector<bool> &vectComp = vectEntity[i].getEntityBitSet();
        for(uint32_t j = 0; j < bitsetComponents.size(); ++j)
        {
            if(bitsetComponents[j] && !vectComp[j])
            {
                ok = false;
                break;
            }
        }
        if(ok)
        {
            vectReturn.push_back(vectEntity[i].muiGetIDEntity());
        }
        ok = true;
    }
    return vectReturn;
}

//===================================================================
void ECSManager::syncComponentsFromEntities(uint32_t numEntity,
                                            const std::vector<Components_e> &vectComp)
{
    assert(m_componentManager && "m_componentManager is null.");
    m_componentManager->updateComponentFromEntity();
    for(uint32_t i = 0; i < vectComp.size(); ++i)
    {
        switch(vectComp[i])
        {
        case Components_e::POSITION_VERTEX_COMPONENT:
        {
            instanciatePositionVertexComponent(numEntity);
        }
            break;
        case Components_e::COLOR_VERTEX_COMPONENT:
        {
            instanciateColorVertexComponent(numEntity);
        }
            break;
        case Components_e::SPRITE_TEXTURE_COMPONENT:
        {
            m_componentManager->instanciateExternComponent(
                        numEntity,
                        std::make_unique<SpriteTextureComponent>());
        }
            break;
        case Components_e::MAP_COORD_COMPONENT:
        {
            m_componentManager->instanciateExternComponent(
                        numEntity,
                        std::make_unique<MapCoordComponent>());
        }
            break;
        case Components_e::STATIC_ELEMENT_COMPONENT:
        {
            m_componentManager->instanciateExternComponent(
                        numEntity,
                        std::make_unique<StaticElementComponent>());
        }
            break;
        case Components_e::MOVEABLE_COMPONENT:
        {
            m_componentManager->instanciateExternComponent(
                        numEntity,
                        std::make_unique<MoveableComponent>());
        }
            break;
        case Components_e::INPUT_COMPONENT:
        {
            m_componentManager->instanciateExternComponent(
                        numEntity,
                        std::make_unique<InputComponent>());
        }
            break;
        case Components_e::GENERAL_COLLISION_COMPONENT:
        {
            m_componentManager->instanciateExternComponent(
                        numEntity,
                        std::make_unique<GeneralCollisionComponent>());
        }
            break;
        case Components_e::CIRCLE_COLLISION_COMPONENT:
        {
            m_componentManager->instanciateExternComponent(
                        numEntity,
                        std::make_unique<CircleCollisionComponent>());
        }
            break;
        case Components_e::LINE_COLLISION_COMPONENT:
        {
            m_componentManager->instanciateExternComponent(
                        numEntity,
                        std::make_unique<SegmentCollisionComponent>());
        }
            break;
        case Components_e::RECTANGLE_COLLISION_COMPONENT:
        {
            m_componentManager->instanciateExternComponent(
                        numEntity,
                        std::make_unique<RectangleCollisionComponent>());
        }
            break;
        case Components_e::VISION_COMPONENT:
        {
            m_componentManager->instanciateExternComponent(
                        numEntity,
                        std::make_unique<VisionComponent>());
        }
            break;
        case Components_e::MEM_SPRITE_DATA_COMPONENT:
        {
            m_componentManager->instanciateExternComponent(numEntity,
                        std::make_unique<MemSpriteDataComponent>());
        }
            break;
        case Components_e::TIMER_COMPONENT:
        {
            m_componentManager->instanciateExternComponent(numEntity,
                        std::make_unique<TimerComponent>());
        }
            break;
        case Components_e::TOTAL_COMPONENTS:
            assert("Bad enum.");
            break;
        }
    }
}

//===================================================================
void ECSManager::instanciatePositionVertexComponent(uint32_t numEntity)
{
    m_componentManager->instanciateExternComponent(
                numEntity,
                std::make_unique<PositionVertexComponent>());
}

//===================================================================
void ECSManager::instanciateColorVertexComponent(uint32_t numEntity)
{
    m_componentManager->instanciateExternComponent(
                numEntity,
                std::make_unique<ColorVertexComponent>());
}

