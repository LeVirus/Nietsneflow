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
    m_weaponChange = false, m_inMovement, m_spritePositionCorrected = true;
    pairFloat_t m_currentMove = {-0.02f, -0.01f};
    uint32_t m_weaponEntity;
    WeaponsType_e m_currentWeapon, m_previousWeapon;
    virtual ~PlayerConfComponent() = default;
};
