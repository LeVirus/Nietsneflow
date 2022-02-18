#pragma once

#include <BaseECS/component.hpp>
#include <constants.hpp>

struct RectangleCollisionComponent : public ecs::Component
{
    RectangleCollisionComponent()
    {
        muiTypeComponent = Components_e::RECTANGLE_COLLISION_COMPONENT;
    }
    PairFloat_t m_size;
    virtual ~RectangleCollisionComponent() = default;
};
