#ifndef COLORVERTEXCOMPONENT_HPP
#define COLORVERTEXCOMPONENT_HPP

#include <BaseECS/component.hpp>
#include <constants.hpp>

using tupleTriFloat = std::tuple<float, float, float>;

struct ColorVertexComponent : public ecs::Component
{
    ColorVertexComponent()
    {
        muiTypeComponent = Components_e::COLOR_VERTEX_COMPONENT;
    }
    std::vector<tupleTriFloat> m_vertex;
    virtual ~ColorVertexComponent() = default;
};

#endif // COLORVERTEXCOMPONENT_HPP

