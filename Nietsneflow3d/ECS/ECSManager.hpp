#ifndef ECSMANAGER_H
#define ECSMANAGER_H

#include <includesLib/BaseECS/componentmanager.hpp>
#include <includesLib/BaseECS/systemmanager.hpp>
#include <includesLib/BaseECS/engine.hpp>
#include <constants.hpp>

class ECSManager
{
private:
    ecs::Engine m_ecsEngine;
    ecs::ComponentManager *m_componentManager = nullptr;
    ecs::SystemManager *m_systemManager = nullptr;
private:
    void initComponents();
    void initSystems();
    void syncComponentsFromEntities(uint32_t numEntity,
                                    const std::vector<Components_e> &vectComp);
public:
    ECSManager();
    void init();
    uint32_t addEntity(const std::bitset<Components_e::TOTAL_COMPONENTS> &bitsetComponents);
    inline ecs::Engine &getEngine(){return m_ecsEngine;}
    inline ecs::ComponentManager &getComponentManager(){return *m_componentManager;}
    inline ecs::SystemManager &getSystemManager(){return *m_systemManager;}
};

#endif // ECSMANAGER_H
