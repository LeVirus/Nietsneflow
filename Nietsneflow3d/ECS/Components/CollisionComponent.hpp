#ifndef TAGCOMPONENT_HPP
#define TAGCOMPONENT_HPP

#include <BaseECS/component.hpp>
#include <constants.hpp>

struct CollisionComponent : public ecs::Component
{
    CollisionComponent()
    {
        muiTypeComponent = Components_e::COLLISION_COMPONENT;
    }
    CollisionTag_e m_tag;
    CollisionShape_e m_shape;
    virtual ~CollisionComponent() = default;
};

#endif // TAGCOMPONENT_HPP
