#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <vector>
#include <map>
#include "systemmanager.hpp"
#include "entity.hpp"
#include "componentmanager.hpp"

namespace ecs
{

/**
 * @brief La classe Engine est la classe de base de la bibliothèque.
 * Elle contient un vector d'Entity le gestionnaire de système et le gestionnaire de composant.
 * Cette classe permet de gérer les Entités.
 */

class Engine
{
private:
    std::vector< Entity > mVectEntity;
    SystemManager mSystemManager;
    ComponentManager mComponentManager;
public:
    Engine();
    const std::vector< Entity > & getVectEntity()const;
    void synchronizeVectorEntity();
    unsigned int AddEntity();
    bool bRmEntity(unsigned int uiIdEntity);
    void RmAllEntity();
    bool bAddComponentToEntity(unsigned int uiIdEntity, unsigned int uiTypeComponent);
    bool bRmComponentToEntity(unsigned int uiIdEntity, unsigned int uiTypeComponent);
    void setEntityUpToDate();
    void execIteration();
    SystemManager &getSystemManager();
    ComponentManager &getComponentManager();
};

}//fin namespace
#endif // ENGINE_HPP
