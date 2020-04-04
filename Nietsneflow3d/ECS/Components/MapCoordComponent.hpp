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
    pairUI_t m_coord;
    pairFloat_t m_absoluteMapPositionPX;
    virtual ~MapCoordComponent() = default;
};
