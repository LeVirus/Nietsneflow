#ifndef SYSTEM_HPP
#define SYSTEM_HPP

#include "systemmanager.hpp"
#include "componentmanager.hpp"
#include <vector>
#include <bitset>
#include <memory>




namespace ecs
{

class SystemManager;

/**
 * @brief La classe System est la classe mère(abstraite) de tous systèmes.
 * Cette classe permet la récupération des numéros des entités à traiter par le système,
 * cela en fonction des données présentes dans le bitset mBitSetComponentSystem.
 * Les composants nécessaires sont déterminés dans les classes filles de System.
 */
class System
{
protected:
	unsigned int muiIdSystem, muiTypeSystem, muiMemNumberComponent;
    std::vector< unsigned int > mVectNumEntity;
	std::vector< bool > mBitSetComponentSystem;
	ecs::SystemManager* mptrSystemManager;
    bool bAddComponentToSystem(unsigned int uiTypeComponent);
public:
    System();
	const std::vector< unsigned int > &getVectNumEntity()const;
    void linkSystemManager(SystemManager* ptrSystemManager);
    void refreshEntity();
    virtual void execSystem();
    bool bComponentAlreadyExist(unsigned int uiTypeComponent);
    ComponentManager & stairwayToComponentManager();

    inline unsigned int uiGetIdSystem()const{ return muiIdSystem; }
    inline unsigned int uiGetTypeSystem()const{ return muiTypeSystem; }

    inline void modValueIdSystem(unsigned int uiIdSystem){ muiIdSystem = uiIdSystem; }
    inline void modValueTypeSystem(unsigned int uiTypeSystem){ muiTypeSystem = uiTypeSystem; }
};

}//fin namespace
#endif // SYSTEM_HPP
