#pragma once

#include <BaseECS/component.hpp>
#include <constants.hpp>
#include <array>
#include <set>

enum class MapMode_e
{
    NONE,
    MINI_MAP,
    FULL_MAP
};

enum class PlayerEntities_e
{
    WEAPON,
    AMMO_WRITE,
    TITLE_MENU,
    MENU_ENTRIES,
    MENU_CURSOR,
    LIFE_AMMO_PANNEL,
    MENU_INFO_WRITE,
    ACTION,
    HIT_MELEE,
    LIFE_WRITE,
    NUM_INFO_WRITE,
    DISPLAY_TELEPORT,
    MAP_DETECT_SHAPE,
    LIFE_ICON,
    AMMO_ICON,
    CURSOR_WEAPON_PREVIEW,
    MENU_GENERIC_BACKGROUND,
    MENU_TITLE_BACKGROUND,
    MENU_SELECTED_LINE,
    TOTAL
};

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
    bool m_playerShoot = false, m_takeDamage = false, m_inMovement = false, m_inputModified, m_firstMenu = true,
    m_pickItem = false, m_crush = false, m_frozen = false, m_teleported, m_insideWall = false, m_keyboardInputMenuMode = true;
    std::pair<bool, std::string> m_infoWriteData = {false, ""};
    std::set<uint32_t> m_card;
    uint32_t m_currentCursorPos = 0, m_currentSelectedSaveFile, m_life = 100, m_currentCustomLevelCusorMenu, m_levelToLoad;
    std::array<uint32_t, static_cast<uint32_t>(PlayerEntities_e::TOTAL)> m_vectEntities;
    //display only weapons when changing weapons
    std::array<uint32_t, 6> m_vectPossessedWeaponsPreviewEntities;
    std::optional<uint32_t> m_secretsFound, m_enemiesKilled;
    std::optional<std::pair<uint32_t, Direction_e>> m_currentCheckpoint;
    std::optional<PairUI_t> m_checkpointReached;
    MenuMode_e m_menuMode, m_previousMenuMode;
    MapMode_e m_mapMode = MapMode_e::NONE;
    virtual ~PlayerConfComponent() = default;
};
