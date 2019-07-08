#ifndef ENTITY_HPP
#define ENTITY_HPP

#include <vector>
#include <bitset>
#include <memory>
#include "component.hpp"
#include "componentmanager.hpp"

namespace ecs
{
class ComponentManager;
/**
 * @brief La classe Entity est représentative d'un élément de la scène du jeu.
 * Elle contient un bitset qui indique quels sont les composants que contient l'entité,
 * ils sont positionnés dans le bitset en fonction de leurs types(muiTypeComponent).
 */
class Entity
{
private:
    unsigned int mUiIDEntity;
    std::vector<bool> mBitSetComponent;

    /*mbActive détermine si l'entité est actuellement activée dans la scène du jeu.
     * mbEntityInUse détermine pour une entité activée si celle ci est "endormie" ou non.
     * Exemple: mbEntityInUse = false si l'entité est hors de l'écran.
     */
    bool mbActive, mbUpToDate, mbEntityInUse;
    void reinitComponentBitSet();
public:
    const std::vector<bool> & getEntityBitSet()const;

    Entity();
    Entity(unsigned int uiIdEntity);
    bool ComponentExist(unsigned int uiTypeComponent)const;
    bool bEntityIsUpToDate()const;
    bool bInUse()const;
    void initEntity();
    void modifyEntityInUse(bool bInUse);
    void setUpToDate();
    void attributeIDEntity(unsigned int uiIdEntity);
    bool bAddComponent(unsigned int uiTypeComponent);
    bool bRmComponent(unsigned int uiTypeComponent);
    void RmAllComponent();
    bool bEntityIsActive()const;
    inline unsigned int muiGetIDEntity()const{return mUiIDEntity;}
};

}//fin namespace
#endif // ENTITY_HPP
