#ifndef NUMBERVERTEXCOMPONENT_HPP
#define NUMBERVERTEXCOMPONENT_HPP

#include <BaseECS/component.hpp>
#include <constants.hpp>

struct NumberVertexComponent : public ecs::Component
{
    NumberVertexComponent(uint32_t numberVertex) : m_numberVertex(numberVertex)
    {
        muiTypeComponent = Components_e::NUMBER_VERTEX_COMPONENT;
    }
    constexpr inline size_t getNumberVertex()const{return m_numberVertex;}
    const size_t m_numberVertex;
    virtual ~NumberVertexComponent() = default;
};

#endif // NUMBERVERTEXCOMPONENT_HPP

