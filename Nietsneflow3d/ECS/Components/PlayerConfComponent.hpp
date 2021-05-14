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
        if(m_life <= damage)
        {
            m_life = 0;
            return false;
        }
        else
        {
            m_life -= damage;
            return true;
        }
    }
    bool m_playerAction = false, m_playerShoot = false, m_timerShootActive = false,
    m_weaponChange = false, m_inMovement, m_spritePositionCorrected = true;
    pairFloat_t m_currentWeaponMove = {-0.02f, -0.01f};
    uint32_t m_weaponEntity, m_ammoWriteEntity, m_menuEntity, m_menuCursorEntity,
    m_lifeWriteEntity, m_life = 100;
    AmmoContainer_t m_shootEntities, m_visibleShootEntities;
    WeaponsType_e m_currentWeapon, m_previousWeapon;
    std::array<uint32_t, static_cast<uint32_t>(WeaponsType_e::TOTAL)> m_ammunationsCount = {100, 50};
    std::array<bool, static_cast<uint32_t>(WeaponsType_e::TOTAL)> m_weapons = {true, false};
    CurrentMenuCursorPos_e m_currentCursorPos = CurrentMenuCursorPos_e::NEW_GAME;
    virtual ~PlayerConfComponent() = default;
};
