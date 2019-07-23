#ifndef NUMBERVERTEXCOMPONENT_HPP
#define NUMBERVERTEXCOMPONENT_HPP

#include <BaseECS/component.hpp>
#include <constants.hpp>

struct NumberVertexComponent : public ecs::Component
{
    NumberVertexComponent(BaseShapeType_t shapeType) : m_shapeType(shapeType)
    {
        muiTypeComponent = Components_e::NUMBER_VERTEX_COMPONENT;
    }
    const BaseShapeType_t m_shapeType;
    virtual ~NumberVertexComponent() = default;
};

#endif // NUMBERVERTEXCOMPONENT_HPP

