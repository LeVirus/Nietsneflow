#pragma once

#include <BaseECS/component.hpp>
#include <constants.hpp>

struct GeneralCollisionComponent : public ecs::Component
{
    GeneralCollisionComponent()
    {
        muiTypeComponent = Components_e::GENERAL_COLLISION_COMPONENT;
    }
    CollisionTag_e m_tagA, m_tagB = CollisionTag_e::GHOST_CT;
    CollisionShape_e m_shape;
    bool m_active = true;
    virtual ~GeneralCollisionComponent() = default;
};
