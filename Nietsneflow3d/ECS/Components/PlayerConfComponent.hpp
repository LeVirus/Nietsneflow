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
    bool m_playerShoot = false, m_takeDamage = false, m_inMovement = false, m_inputModified,
    m_pickItem = false, m_crush = false, m_frozen = false, m_teleported, m_insideWall = false, m_keyboardInputMenuMode = true;
    std::pair<bool, std::string> m_infoWriteData = {false, ""};
    std::set<uint32_t> m_card;
    uint32_t m_weaponEntity, m_ammoWriteEntity, m_currentCursorPos = 0, m_currentSelectedSaveFile,
    m_titleMenuEntity, m_menuEntriesEntity, m_menuCursorEntity, m_menuInfoWriteEntity,
    m_actionEntity, m_hitMeleeEntity, m_lifeWriteEntity, m_numInfoWriteEntity,
    m_life = 100, m_displayTeleportEntity;
    std::optional<uint32_t> m_currentCheckpoint, m_secretFound, m_enemiesKilled;
    std::optional<PairUI_t> m_checkpointReached;
    MenuMode_e m_menuMode, m_previousMenuMode;
    virtual ~PlayerConfComponent() = default;
};
