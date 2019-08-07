#ifndef MAPPOSITIONCOMPONENT_HPP
#define MAPPOSITIONCOMPONENT_HPP

#include <BaseECS/component.hpp>
#include <constants.hpp>
#include <functional>

using pairUint_t = std::pair<uint32_t, uint32_t>;

struct MapPositionComponent : public ecs::Component
{
    MapPositionComponent()
    {
        muiTypeComponent = Components_e::MAP_POSITION_COMPONENT;
    }
    pairUint_t m_coord;
    virtual ~MapPositionComponent() = default;
};

#endif // MAPPOSITIONCOMPONENT_HPP

