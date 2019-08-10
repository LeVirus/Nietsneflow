#ifndef MAPPOSITIONCOMPONENT_HPP
#define MAPPOSITIONCOMPONENT_HPP

#include <BaseECS/component.hpp>
#include <constants.hpp>
#include <functional>

using pairUint_t = std::pair<uint32_t, uint32_t>;
using pairFloat_t = std::pair<float, float>;

struct MapCoordComponent : public ecs::Component
{
    MapCoordComponent()
    {
        muiTypeComponent = Components_e::MAP_COORD_COMPONENT;
    }
    pairUint_t m_coord;
    pairFloat_t m_absoluteMapPositionPX;
    virtual ~MapCoordComponent() = default;
};

#endif // MAPPOSITIONCOMPONENT_HPP

