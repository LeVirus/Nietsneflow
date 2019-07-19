#ifndef COMPONENT_HPP
#define COMPONENT_HPP

#include <iostream>


namespace ecs
{

/**
 * @brief La structure Component est la classe mère de tous les composants.
 * Un composant représente un élément d'une entité.
 * Elle est abstraite, et comporte un numéro qui détermine le type de composant.
 */
struct Component
{
    inline unsigned int muiGetTypeComponent()const{return muiTypeComponent;}
    inline unsigned int muiGetIdComponent()const{return muiIDComponent;}
    inline unsigned int muiGetIdEntityAssociated()const{return muiIDEntityAssociated;}

	/**
	 * @brief setIDEntityAssociated Fonction d'attribution du numéro d'entité à associer au composant.
	 * @param uiEntityID Le numéro de l'entité à associer.
	 */
    void setIDEntityAssociated(unsigned int uiEntityID)
    {
		muiIDEntityAssociated = uiEntityID;
	}
    virtual ~Component() = default;
protected:
	unsigned int muiTypeComponent, muiIDComponent, muiIDEntityAssociated;
	inline Component(){}
};

}//fin namespace
#endif // COMPONENT_HPP
