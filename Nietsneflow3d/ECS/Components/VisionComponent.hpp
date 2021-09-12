#pragma once

#include <BaseECS/component.hpp>
#include <constants.hpp>
#include <vector>
#include <ECS/Components/PositionVertexComponent.hpp>
#include <ECS/Components/ColorVertexComponent.hpp>

struct VisionComponent : public ecs::Component
{
    VisionComponent()
    {
        muiTypeComponent = Components_e::VISION_COMPONENT;
    }
    //first point angle ref
    float m_distanceVisibility = 500.0f;
    std::vector<uint32_t> m_vectVisibleEntities;
    //absolute position
    std::array<PairFloat_t, 3> m_triangleVision;
    //relative position (map)
    PositionVertexComponent m_positionVertexComp;
    ColorVertexComponent m_colorVertexComp;
    virtual ~VisionComponent() = default;
};
