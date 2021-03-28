#pragma once

#include <BaseECS/component.hpp>
#include <constants.hpp>

struct PlayerConfComponent : public ecs::Component
{
    PlayerConfComponent()
    {
        muiTypeComponent = Components_e::PLAYER_CONF_COMPONENT;
    }
    bool m_playerAction = false, m_playerShoot = false, m_timerShootActive = false,
    m_weaponChange = false;
    uint32_t m_weaponEntity;
    WeaponsType_e m_currentWeapon;
    virtual ~PlayerConfComponent() = default;
};
