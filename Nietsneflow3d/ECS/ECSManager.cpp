#include <ECS/ECSManager.hpp>
#include <ECS/Components/PositionVertexComponent.hpp>
#include <ECS/Components/SpriteTextureComponent.hpp>
#include <ECS/Components/ColorVertexComponent.hpp>
#include <ECS/Components/MapCoordComponent.hpp>
#include <ECS/Components/StaticElementComponent.hpp>
#include <ECS/Components/MoveableComponent.hpp>
#include <ECS/Components/InputComponent.hpp>
#include <ECS/Components/CollisionComponent.hpp>
#include <ECS/Components/CircleCollisionComponent.hpp>
#include <ECS/Components/LineCollisionComponent.hpp>
#include <ECS/Components/RectangleCollisionComponent.hpp>
#include <ECS/Systems/ColorDisplaySystem.hpp>
#include <ECS/Systems/MapDisplaySystem.hpp>
#include <ECS/Systems/InputSystem.hpp>
#include <ECS/Systems/CollisionSystem.hpp>
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
        case Components_e::TAG_COMPONENT:
        {
            m_componentManager->instanciateExternComponent(
                        numEntity,
                        std::make_unique<CollisionComponent>());
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

