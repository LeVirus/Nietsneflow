#pragma once

#include <BaseECS/component.hpp>
#include <constants.hpp>

enum class ObjectType_e
{
    GUN_AMMO,
    SHOTGUN_AMMO,
    LITTLE_HEAL,
    BIG_HEAL,
    CARD,
    SHOTGUN,
    PLASMA_RIFLE,
    PLASMA_RIFLE_AMMO,
    TOTAL
};

struct ObjectConfComponent : public ecs::Component
{
    ObjectConfComponent()
    {
        muiTypeComponent = Components_e::OBJECT_CONF_COMPONENT;
    }
    ObjectType_e m_type;
    uint32_t m_containing;
    virtual ~ObjectConfComponent() = default;
};
