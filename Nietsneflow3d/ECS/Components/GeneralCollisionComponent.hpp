#ifndef TAGCOMPONENT_HPP
#define TAGCOMPONENT_HPP

#include <BaseECS/component.hpp>
#include <constants.hpp>

struct GeneralCollisionComponent : public ecs::Component
{
    GeneralCollisionComponent()
    {
        muiTypeComponent = Components_e::GENERAL_COLLISION_COMPONENT;
    }
    CollisionTag_e m_tag;
    CollisionShape_e m_shape;
    virtual ~GeneralCollisionComponent() = default;
};

#endif // TAGCOMPONENT_HPP
