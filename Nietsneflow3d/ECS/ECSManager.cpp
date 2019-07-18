#include <ECS/ECSManager.hpp>
#include <ECS/Components/PositionVertexComponent.hpp>
#include <ECS/Components/SpriteTextureComponent.hpp>
#include <ECS/Components/ColorVertexComponent.hpp>
#include <ECS/Systems/ColorDisplaySystem.hpp>
#include <constants.hpp>
#include <memory>
#include <cassert>

//===================================================================
ECSManager::ECSManager():m_componentManager(&m_ecsEngine.getComponentManager()),
    m_systemManager(&m_ecsEngine.getSystemManager())
{
    confECSMemory();
}

//===================================================================
void ECSManager::initComponents()
{
    m_componentManager->addEmplacementsForExternComponent(Components_e::TOTAL_COMPONENTS);
}

//===================================================================
void ECSManager::initSystems()
{
    m_systemManager->bAddExternSystem(std::make_unique<ColorDisplaySystem>());
}


//===================================================================
void ECSManager::init()
{
    initComponents();
    initSystems();
}

//===================================================================
//Execute syncComponentsFromEntities before launching game
void ECSManager::addEntity(const std::bitset<Components_e::TOTAL_COMPONENTS> &bitsetComponents)
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
}

//===================================================================
void ECSManager::syncComponentsFromEntities(uint32_t numEntity,
                                            const std::vector<Components_e> &vectComp)
{
    m_componentManager->updateComponentFromEntity();
    for(uint32_t i = 0; i < vectComp.size(); ++i)
    {
        switch(vectComp[i])
        {
        case Components_e::POSITION_VERTEX_COMPONENT:
            m_componentManager->instanciateExternComponent(numEntity,
                                                             std::make_unique<PositionVertexComponent>());
            break;
        case Components_e::COLOR_VERTEX_COMPONENT:
            m_componentManager->instanciateExternComponent(numEntity,
                                                             std::make_unique<ColorVertexComponent>());
            break;
        case Components_e::SPRITE_TEXTURE_COMPONENT:
            m_componentManager->instanciateExternComponent(numEntity,
                                                             std::make_unique<SpriteTextureComponent>());
            break;
        case Components_e::TOTAL_COMPONENTS:
            assert("Bad enum.");
        }
    }
}

//===================================================================
void ECSManager::confECSMemory()
{
//    m_componentManager = m_ecsEngine.getComponentManager();
//    m_systemManager = m_ecsEngine.getSystemManager();
}

