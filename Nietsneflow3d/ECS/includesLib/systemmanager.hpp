#ifndef SYSTEMMANAGER_HPP
#define SYSTEMMANAGER_HPP

#include <vector>
#include <iostream>
#include <memory>

#include "system.hpp"

namespace ecs
{

class Engine;
class System;

/**
 * @brief La classe SystemManager gère les systèmes actifs.
 * Elle contient un tableau de système dont le nombre de case est égal au nombre de type de système.
 * Cette classe permet d'ajouter/supprimer les systèmes et de les exécuter.
 */
class SystemManager{
    std::vector<std::unique_ptr<System>> mVectSystem;
    Engine* mptrEngine;
    unsigned int muiNumberSystem = 0;
public:
    SystemManager();
    Engine* getptrEngine();
    void linkEngine(Engine* ptrEngine);
    bool bAddExternSystem(std::unique_ptr<System> newSystem);
    void RmAllSystem();
    void bExecAllSystem();

    /**
     * @brief SystemManager::searchSystemByType
     * Fonction renvoyant la référence du système recherché.
     * Une vérification est effectué avant l'envoie:
     * -Le paramètre renvoie bien à une case du tableau(pas de dépassement de mémoire).
     * -Le paramètre renvoie bien à une case contenant un système instancié.
     * La fonction renvoie le Système avec un static_cast.
     * @param uiTypeSystem le type de système a rechercher.
     * @return Un pointeur vers le système demandé, NULL en cas de paramètre invalide.
     */
    template <typename systemTemplate>
     systemTemplate * searchSystemByType(unsigned int uiTypeSystem)
     {
        if(uiTypeSystem >= mVectSystem.size() || ! mVectSystem[uiTypeSystem])return nullptr;
        static_assert(std::is_base_of<System, systemTemplate>(), "systemTemplate n'est pas un système");
        //récupérer un pointeur vers l'objet contenu dans le unique_ptr
        return static_cast<systemTemplate*>(mVectSystem[uiTypeSystem].get());
    }
};

}//fin namespace
#endif // SYSTEMMANAGER_HPP
