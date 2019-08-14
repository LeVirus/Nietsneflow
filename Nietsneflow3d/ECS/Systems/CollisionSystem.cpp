#include "CollisionSystem.hpp"
#include "constants.hpp"


//===================================================================
CollisionSystem::CollisionSystem()
{
    setUsedComponents();
}

//===================================================================
void CollisionSystem::setUsedComponents()
{
    bAddComponentToSystem(Components_e::TAG_COMPONENT);
}

//===================================================================
void CollisionSystem::execSystem()
{

}

