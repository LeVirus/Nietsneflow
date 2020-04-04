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
    array3PairFloat_t m_triangleVision;
    float m_degreeAngle;
    std::vector<uint32_t> m_vectVisibleEntities;
};
