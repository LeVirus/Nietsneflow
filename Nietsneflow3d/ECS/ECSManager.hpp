#pragma once

//#include <includesLib/BaseECS/componentmanager.hpp>
#include <includesLib/BaseECS/systemmanager.hpp>
#include <includesLib/BaseECS/engine.hpp>
#include <constants.hpp>
#include <ECS/NewComponentManager.hpp>

class ECSManager
{
public:
    ECSManager();
    void init();
    uint32_t addEntity(const std::bitset<Components_e::TOTAL_COMPONENTS> &bitsetComponents);
    inline bool bRmEntity(uint32_t numEntity)
    {
        return m_ecsEngine.bRmEntity(numEntity);
    }
    std::vector<uint32_t> getEntitiesContainingComponents(const std::bitset<Components_e::TOTAL_COMPONENTS> &bitsetComponents)const;
    inline ecs::Engine &getEngine(){return m_ecsEngine;}
    inline NewComponentManager &getComponentManager()
    {
        return m_newComponentManager;
    }
    inline ecs::SystemManager &getSystemManager(){return *m_systemManager;}
private:
    void initComponents();
    void initSystems();
private:
    ecs::Engine m_ecsEngine;
    ecs::ComponentManager *m_componentManager = nullptr;
    ecs::SystemManager *m_systemManager = nullptr;
    NewComponentManager m_newComponentManager;
};
