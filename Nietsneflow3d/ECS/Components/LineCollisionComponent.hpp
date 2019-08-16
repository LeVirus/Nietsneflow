#pragma once

#include <BaseECS/component.hpp>
#include <constants.hpp>

struct LineCollisionComponent : public ecs::Component
{
    LineCollisionComponent()
    {
        muiTypeComponent = Components_e::LINE_COLLISION_COMPONENT;
    }
    //first point is in map Componnent
    std::pair<float, float> m_secondPoint;
    virtual ~LineCollisionComponent() = default;
};
