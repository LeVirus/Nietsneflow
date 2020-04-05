#pragma once

#include <BaseECS/component.hpp>
#include <constants.hpp>
#include <vector>

using array3PairFloat_t = std::array<pairFloat_t, 3>;

struct VisionComponent : public ecs::Component
{
    VisionComponent()
    {
        muiTypeComponent = Components_e::VISION_COMPONENT;
    }
    //first point angle ref
    float m_distanceVisibility = 100.0f, m_coneVision = 40.0f;
    array3PairFloat_t m_triangleVision;
    std::vector<uint32_t> m_vectVisibleEntities;
};
