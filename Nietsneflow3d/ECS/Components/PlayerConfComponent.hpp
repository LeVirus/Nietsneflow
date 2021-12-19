#pragma once

#include <BaseECS/component.hpp>
#include <constants.hpp>
#include <array>
#include <set>

struct PlayerConfComponent : public ecs::Component
{
    PlayerConfComponent()
    {
        muiTypeComponent = Components_e::PLAYER_CONF_COMPONENT;
    }
    void takeDamage(uint32_t damage)
    {
        m_takeDamage = true;
        if(m_life <= damage)
        {
            m_life = 0;
            m_inMovement = false;
        }
        else
        {
            m_life -= damage;
        }
    }
    bool m_playerShoot = false, m_takeDamage = false, m_inMovement = false,
    m_pickItem = false, m_crush = false, m_frozen = false, m_teleported, m_scratch = false;
    std::pair<bool, std::string> m_infoWriteData = {false, ""};
    std::set<uint32_t> m_card;
    uint32_t m_weaponEntity, m_ammoWriteEntity, m_menuEntity, m_menuCursorEntity,
    m_actionEntity, m_hitMeleeEntity, m_lifeWriteEntity, m_numInfoWriteEntity,
    m_life = 100, m_displayTeleportEntity;
    CurrentMenuCursorPos_e m_currentCursorPos = static_cast<CurrentMenuCursorPos_e>(0);
    virtual ~PlayerConfComponent() = default;
};
