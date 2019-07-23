#ifndef COLORVERTEXCOMPONENT_HPP
#define COLORVERTEXCOMPONENT_HPP

#include <BaseECS/component.hpp>
#include <constants.hpp>

using tupleTriFloat = std::tuple<float, float, float>;

template<size_t SIZE>
struct ColorVertexComponent : public ecs::Component
{
    ColorVertexComponent()
    {
        muiTypeComponent = Components_e::COLOR_VERTEX_COMPONENT;
    }
    std::array<tupleTriFloat, SIZE> m_vertex;
    virtual ~ColorVertexComponent() = default;
};

#endif // COLORVERTEXCOMPONENT_HPP

