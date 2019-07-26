#include "MapDisplaySystem.hpp"
#include <constants.hpp>

//===================================================================
MapDisplaySystem::MapDisplaySystem()
{
    setUsedComponents();
}

//===================================================================
void MapDisplaySystem::setUsedComponents()
{
    bAddComponentToSystem(Components_e::POSITION_VERTEX_COMPONENT);
    bAddExcludeComponentToSystem(Components_e::SPRITE_TEXTURE_COMPONENT);
}
