#pragma once

#include <BaseECS/component.hpp>
#include <constants.hpp>

struct SegmentCollisionComponent : public ecs::Component
{
    SegmentCollisionComponent()
    {
        muiTypeComponent = Components_e::SEGMENT_COLLISION_COMPONENT;
    }
    //first point is in map Componnent
    std::pair<pairFloat_t, pairFloat_t> m_points;
    //orientation from first point to second
    float m_degreeOrientation;
    virtual ~SegmentCollisionComponent() = default;
};
