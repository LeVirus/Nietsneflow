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
    bool m_playerShoot = false, m_takeDamage = false, m_inMovement = false;
    uint32_t m_weaponEntity, m_ammoWriteEntity, m_menuEntity, m_menuCursorEntity,
    m_actionEntity, m_hitMeleeEntity, m_lifeWriteEntity, m_life = 100;
    ArrayVisibleShot_t m_shootEntities;
    CurrentMenuCursorPos_e m_currentCursorPos = static_cast<CurrentMenuCursorPos_e>(0);
    virtual ~PlayerConfComponent() = default;
};
