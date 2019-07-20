#ifndef COLORVERTEXCOMPONENT_HPP
#define COLORVERTEXCOMPONENT_HPP

#include <BaseECS/component.hpp>
#include <constants.hpp>

using vertexTupleArray_t = std::array<std::tuple<float, float, float>, 4>;

struct ColorVertexComponent : public ecs::Component
{
    ColorVertexComponent()
    {
        muiTypeComponent = Components_e::COLOR_VERTEX_COMPONENT;
    }

    vertexTupleArray_t m_vertex;
    virtual ~ColorVertexComponent() = default;
};

#endif // COLORVERTEXCOMPONENT_HPP

