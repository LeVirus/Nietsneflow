#pragma once

#include <BaseECS/component.hpp>
#include <constants.hpp>
#include <vector>
#include <ECS/Components/PositionVertexComponent.hpp>

struct VisionComponent : public ecs::Component
{
    VisionComponent()
    {
        muiTypeComponent = Components_e::VISION_COMPONENT;
    }
    //first point angle ref
    float m_distanceVisibility = 100.0f,
    //total cone vision
    m_coneVision = 40.0f;
    std::vector<uint32_t> m_vectVisibleEntities;

    //absolute position
    std::array<pairFloat_t, 3> m_triangleVision;
    //relative position (map)
    PositionVertexComponent m_vertexComp;
    virtual ~VisionComponent() = default;
};
