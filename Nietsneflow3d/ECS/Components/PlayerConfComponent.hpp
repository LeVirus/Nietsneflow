#pragma once

#include <BaseECS/component.hpp>
#include <constants.hpp>
#include <array>
#include <optional>

struct PlayerConfComponent : public ecs::Component
{
    PlayerConfComponent()
    {
        muiTypeComponent = Components_e::PLAYER_CONF_COMPONENT;
    }
    bool takeDamage(uint32_t damage)
    {
        m_takeDamage = true;
        if(m_life <= damage)
        {
            m_life = 0;
            m_inMovement = false;
            return false;
        }
        else
        {
            m_life -= damage;
            return true;
        }
    }
    bool m_playerShoot = false, m_timerShootActive = false, m_shootFirstPhase,
    m_weaponChange = false, m_inMovement = false, m_spritePositionCorrected = true, m_takeDamage = false;
    uint32_t m_numWeaponSprite, m_maxWeapons;
    pairFloat_t m_currentWeaponMove = {-0.02f, -0.01f};
    uint32_t m_weaponEntity, m_ammoWriteEntity, m_menuEntity, m_menuCursorEntity, m_actionEntity,
    m_hitAxeEntity, m_lifeWriteEntity, m_life = 100;
    AmmoContainer_t m_shootEntities, m_visibleShootEntities;
    std::array<uint32_t, SEGMENT_SHOT_NUMBER> m_shotImpact;
    uint32_t m_currentWeapon, m_previousWeapon;
    std::vector<uint32_t> m_ammunationsCount;
    std::vector<bool> m_weapons;
    CurrentMenuCursorPos_e m_currentCursorPos = static_cast<CurrentMenuCursorPos_e>(0);
    virtual ~PlayerConfComponent() = default;
};
