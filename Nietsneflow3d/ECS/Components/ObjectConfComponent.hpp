#pragma once

#include <BaseECS/component.hpp>
#include <constants.hpp>

struct ObjectConfComponent : public ecs::Component
{
    ObjectConfComponent()
    {
        muiTypeComponent = Components_e::CIRCLE_COLLISION_COMPONENT;
    }
    virtual ~ObjectConfComponent() = default;
};
