#pragma once

#include <BaseECS/component.hpp>
#include <constants.hpp>

struct SegmentCollisionComponent : public ecs::Component
{
    SegmentCollisionComponent()
    {
        muiTypeComponent = Components_e::LINE_COLLISION_COMPONENT;
    }
    //first point is in map Componnent
    std::pair<float, float> m_secondPoint;
    virtual ~SegmentCollisionComponent() = default;
};
