#include <ECS/ECSManager.hpp>
#include <ECS/Components/PositionVertexComponent.hpp>
#include <ECS/Components/SpriteTextureComponent.hpp>
#include <ECS/Components/ColorVertexComponent.hpp>
#include <ECS/Components/MapCoordComponent.hpp>
#include <ECS/Components/WallMultiSpriteConf.hpp>
#include <ECS/Components/FPSVisibleStaticElementComponent.hpp>
#include <ECS/Components/MoveableComponent.hpp>
#include <ECS/Components/MoveableWallConfComponent.hpp>
#include <ECS/Components/InputComponent.hpp>
#include <ECS/Components/MemFPSGLSizeComponent.hpp>
#include <ECS/Components/TeleportComponent.hpp>
#include <ECS/Components/LogComponent.hpp>
#include <ECS/Components/WeaponComponent.hpp>
#include <ECS/Components/GeneralCollisionComponent.hpp>
#include <ECS/Components/CircleCollisionComponent.hpp>
#include <ECS/Components/SegmentCollisionComponent.hpp>
#include <ECS/Components/RectangleCollisionComponent.hpp>
#include <ECS/Components/DoorComponent.hpp>
#include <ECS/Components/VisionComponent.hpp>
#include <ECS/Components/PlayerConfComponent.hpp>
#include <ECS/Components/MemPositionsVertexComponents.hpp>
#include <ECS/Components/MemSpriteDataComponent.hpp>
#include <ECS/Components/BarrelComponent.hpp>
#include <ECS/Components/TimerComponent.hpp>
#include <ECS/Components/EnemyConfComponent.hpp>
#include <ECS/Components/WriteComponent.hpp>
#include <ECS/Components/ShotConfComponent.hpp>
#include <ECS/Components/ObjectConfComponent.hpp>
#include <ECS/Components/ImpactShotComponent.hpp>
#include <ECS/Components/CheckpointComponent.hpp>
#include <ECS/Components/TriggerComponent.hpp>
#include <ECS/Components/AudioComponent.hpp>
#include <ECS/Systems/ColorDisplaySystem.hpp>
#include <ECS/Systems/MapDisplaySystem.hpp>
#include <ECS/Systems/InputSystem.hpp>
#include <ECS/Systems/CollisionSystem.hpp>
#include <ECS/Systems/FirstPersonDisplaySystem.hpp>
#include <ECS/Systems/VisionSystem.hpp>
#include <ECS/Systems/DoorWallSystem.hpp>
#include <ECS/Systems/StaticDisplaySystem.hpp>
#include <ECS/Systems/IASystem.hpp>
#include <ECS/Systems/SoundSystem.hpp>
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
    m_systemManager->bAddExternSystem(std::make_unique<ColorDisplaySystem>(m_newComponentManager));
    m_systemManager->bAddExternSystem(std::make_unique<MapDisplaySystem>(m_newComponentManager));
    m_systemManager->bAddExternSystem(std::make_unique<InputSystem>(m_newComponentManager));
    m_systemManager->bAddExternSystem(std::make_unique<CollisionSystem>(m_newComponentManager));
    m_systemManager->bAddExternSystem(std::make_unique<FirstPersonDisplaySystem>(m_newComponentManager));
    m_systemManager->bAddExternSystem(std::make_unique<VisionSystem>(m_newComponentManager, this));
    m_systemManager->bAddExternSystem(std::make_unique<DoorWallSystem>(m_newComponentManager, this));
    m_systemManager->bAddExternSystem(std::make_unique<StaticDisplaySystem>(m_newComponentManager));
    m_systemManager->bAddExternSystem(std::make_unique<IASystem>(m_newComponentManager, this));
    m_systemManager->bAddExternSystem(std::make_unique<SoundSystem>(m_newComponentManager));
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
    for(uint32_t i = 0; i < bitsetComponents.size(); ++i)
    {
        if(bitsetComponents[i])
        {
            m_ecsEngine.bAddComponentToEntity(newEntity, i);
        }
    }
    m_newComponentManager.addEntity(newEntity, bitsetComponents);
    return newEntity;
}

//===================================================================
bool ECSManager::bRmEntity(uint32_t numEntity)
{
    const ecs::Entity &entity = m_ecsEngine.getVectEntity()[numEntity];
    m_newComponentManager.rmEntityComponents(numEntity, entity.getEntityBitSet());
    return m_ecsEngine.bRmEntity(numEntity);
}

//===================================================================
std::vector<uint32_t> ECSManager::getEntitiesContainingComponents(const std::bitset<TOTAL_COMPONENTS> &bitsetComponents) const
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

