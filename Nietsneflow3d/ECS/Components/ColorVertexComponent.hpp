#ifndef COLORVERTEXCOMPONENT_HPP
#define COLORVERTEXCOMPONENT_HPP

#include <BaseECS/component.hpp>
#include <constants.hpp>

using vertexArray_t = std::array<std::pair<float, float>, 4>;

struct ColorVertexComponent : public ecs::Component
{
    ColorVertexComponent(uint32_t entityNum)
    {
        muiIDEntityAssociated = entityNum;
        muiTypeComponent = Components_e::COLOR_VERTEX_COMPONENT;
    }

    vertexArray_t m_vertex;
    virtual ~ColorVertexComponent() = default;
};

#endif // COLORVERTEXCOMPONENT_HPP

