#pragma once

#include <BaseECS/component.hpp>
#include <constants.hpp>
#include <array>
#include <functional>

using pairFloat_t = std::pair<float, float>;

/**
 * @brief The VertexComponent struct
 * Coordinated data for OpenGL displaying.
 */
struct PositionVertexComponent : public ecs::Component
{
    PositionVertexComponent()
    {
        muiTypeComponent = Components_e::POSITION_VERTEX_COMPONENT;
    }

    std::vector<pairFloat_t> m_vertex;
    virtual ~PositionVertexComponent() = default;
};
