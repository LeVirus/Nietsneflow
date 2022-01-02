#pragma once

#include <BaseECS/component.hpp>
#include <constants.hpp>
#include <vector>

using TupleTetraFloat_t = std::tuple<float, float, float, float>;

struct ColorVertexComponent : public ecs::Component
{
    ColorVertexComponent()
    {
        muiTypeComponent = Components_e::COLOR_VERTEX_COMPONENT;
    }
    std::vector<TupleTetraFloat_t> m_vertex;
    virtual ~ColorVertexComponent() = default;
};
