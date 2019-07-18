#ifndef COLORVERTEXCOMPONENT_HPP
#define COLORVERTEXCOMPONENT_HPP

#include <BaseECS/component.hpp>
#include <constants.hpp>

struct ColorVertexComponent : public ecs::Component
{
    ColorVertexComponent()
    {
        muiTypeComponent = Components_e::COLOR_VERTEX_COMPONENT;
    }

    vertexArray_t m_vertex;
    virtual ~ColorVertexComponent() = default;
};

#endif // COLORVERTEXCOMPONENT_HPP

