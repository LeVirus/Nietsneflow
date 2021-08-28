#pragma once

#include <includesLib/BaseECS/componentmanager.hpp>
#include <includesLib/BaseECS/systemmanager.hpp>
#include <includesLib/BaseECS/engine.hpp>
#include <constants.hpp>

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
    inline ecs::ComponentManager &getComponentManager(){return *m_componentManager;}
    inline ecs::SystemManager &getSystemManager(){return *m_systemManager;}
private:
    void initComponents();
    void initSystems();
    void syncComponentsFromEntities(uint32_t numEntity,
                                    const std::vector<Components_e> &vectComp);
    void instanciatePositionVertexComponent(uint32_t numEntity);
    void instanciateColorVertexComponent(uint32_t numEntity);
private:
    ecs::Engine m_ecsEngine;
    ecs::ComponentManager *m_componentManager = nullptr;
    ecs::SystemManager *m_systemManager = nullptr;
};
