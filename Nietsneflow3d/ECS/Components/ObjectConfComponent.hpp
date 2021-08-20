#pragma once

#include <BaseECS/component.hpp>
#include <constants.hpp>

enum class ObjectType_e
{
    WEAPON,
    AMMO_WEAPON,
    HEAL,
    CARD,
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
    std::optional<uint32_t> m_weaponID = std::nullopt, m_cardID = std::nullopt;
    virtual ~ObjectConfComponent() = default;
};
