#pragma once

#include <BaseECS/component.hpp>
#include <constants.hpp>

struct RectangleCollisionComponent : public ecs::Component
{
    RectangleCollisionComponent()
    {
        muiTypeComponent = Components_e::RECTANGLE_COLLISION_COMPONENT;
    }
    //position up left
    std::pair<float, float> m_size;
    virtual ~RectangleCollisionComponent() = default;
};
