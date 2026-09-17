#pragma once

#include <BaseECS/component.hpp>
#include <constants.hpp>
#include <vector>

using ArrayTetraFloat = std::array<float, 4>;

struct ColorVertexComponent : public ecs::Component
{
    ColorVertexComponent()
    {
        muiTypeComponent = Components_e::COLOR_VERTEX_COMPONENT;
    }
    std::vector<ArrayTetraFloat> m_vertex;
    virtual ~ColorVertexComponent() = default;
};
