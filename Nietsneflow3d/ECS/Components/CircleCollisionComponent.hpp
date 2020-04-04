#pragma once

#include <BaseECS/component.hpp>
#include <constants.hpp>

struct CircleCollisionComponent : public ecs::Component
{
    CircleCollisionComponent()
    {
        muiTypeComponent = Components_e::CIRCLE_COLLISION_COMPONENT;
    }
    pairFloat_t m_center;
    float m_ray;
    virtual ~CircleCollisionComponent() = default;
};
