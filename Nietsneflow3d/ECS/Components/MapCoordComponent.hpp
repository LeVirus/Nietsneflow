#pragma once

#include <BaseECS/component.hpp>
#include <constants.hpp>
#include <functional>

struct MapCoordComponent : public ecs::Component
{
    MapCoordComponent()
    {
        muiTypeComponent = Components_e::MAP_COORD_COMPONENT;
    }
    PairUI_t m_coord;
    PairFloat_t m_absoluteMapPositionPX;
};
