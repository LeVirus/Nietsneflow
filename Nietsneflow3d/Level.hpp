#pragma once

#include <stdint.h>
#include <functional>
#include <vector>
#include <set>
#include <constants.hpp>
#include <ECS/Components/ObjectConfComponent.hpp>

using vectPairUI_t = std::vector<pairUI_t>;
using pairUIPairFloat_t = std::pair<uint8_t, pairFloat_t>;

struct MoveableComponent;
struct PositionVertexComponent;
struct SpriteTextureComponent;

struct StaticLevelElementData
{
    uint8_t m_numSprite;
    //In Game sprite size in % relative to a tile
    vectPairUI_t m_TileGamePosition;
    pairDouble_t m_inGameSpriteSize;
    ObjectType_e m_type;
    uint32_t m_containing;
    bool m_traversable = true;
};

struct WallData
{
    std::vector<uint8_t> m_sprites;
    std::set<pairUI_t> m_TileGamePosition;
};

struct DoorData
{
    uint8_t m_numSprite;
    vectPairUI_t m_TileGamePosition;
    //Position of axe true = vertical false = horizontal
    bool m_vertical;
};

struct ElementRaycast
{
    uint32_t m_numEntity;
    LevelCaseType_e m_type;
    pairUI_t m_tileGamePosition;
    SpriteTextureComponent const *m_spriteComp;
};

struct EnemyData
{
    //m_staticSprites represent the motionless enemy
    //m_moveSprites represent the moving enemy
    //m_attackSprites represent the attacking enemy
    //m_dyingSprites represent the dying enemy

    //after dying animation the ennemy is represented by the last sprite
    //contained in m_dyingSprites
    std::vector<uint8_t> m_staticFrontSprites, m_staticFrontLeftSprites, m_staticFrontRightSprites,
    m_staticBackSprites, m_staticBackLeftSprites, m_staticBackRightSprites,
    m_staticLeftSprites, m_staticRightSprites, m_moveSprites,
    m_attackSprites, m_dyingSprites, m_visibleShotSprites, m_visibleShotDestructSprites;

    //In Game sprite size in % relative to a tile
    pairDouble_t m_inGameSpriteSize;
    vectPairUI_t m_TileGamePosition;
    bool m_traversable;
};

class Level
{
public:
    Level();
    void setPlayerInitData(const pairFloat_t &pairInitPlayerPos,
                           Direction_e playerDir);
    static void initLevelElementArray();
    static const std::vector<ElementRaycast> &getLevelCaseType()
    {
        return m_levelCaseType;
    }
    static void addElementCase(SpriteTextureComponent *spriteComp, const pairUI_t &tilePosition,
                               LevelCaseType_e type, uint32_t numEntity);
    static std::optional<ElementRaycast> getElementCase(const pairUI_t &tilePosition);

    static uint32_t getLevelCaseIndex(const pairUI_t &tilePosition);
    inline const std::vector<pairUIPairFloat_t> &getWeaponsData()const
    {
        return m_weaponsDisplayData;
    }

    inline const pairUI_t &getPlayerDeparture()const
    {
        return m_playerDeparture;
    }

    static inline pairUI_t &getSize()
    {
        return m_size;
    }

    inline Direction_e getPlayerDepartureDirection()const
    {
        return m_playerDepartureDirection;
    }

    inline const std::vector<uint8_t> &getVisibleShotsData()const
    {
        return m_vectVisibleShot;
    }

    static inline void setLevelSize(const pairFloat_t &pairLevelSize)
    {
        m_size = pairLevelSize;
    }
    inline void setWeaponsElement(const std::vector<pairUIPairFloat_t> &vectWeapons,
                                  const std::vector<uint8_t> &visibleShot)
    {
        m_weaponsDisplayData = vectWeapons;
        m_vectVisibleShot = visibleShot;
    }

    inline static float getRangeView()
    {
        return Level::m_rangeViewPX;
    }

    /**
     * @brief updateVisualOrientation Modify vertex position relative to orientation.
     */
    static void updatePlayerOrientation(const MoveableComponent &moveComp,
                                        PositionVertexComponent &posComp);
private:
    pairUI_t m_playerDeparture;
    static pairUI_t m_size;
    Direction_e m_playerDepartureDirection;
    //store the sprite number and the screen display size
    std::vector<pairUIPairFloat_t> m_weaponsDisplayData;
    std::vector<uint8_t> m_vectVisibleShot;
    static std::vector<ElementRaycast> m_levelCaseType;
    static float m_rangeViewPX;
};

pairFloat_t getAbsolutePosition(const pairUI_t &coord);
pairFloat_t getCenteredAbsolutePosition(const pairUI_t &coord);

std::optional<pairUI_t> getLevelCoord(const pairFloat_t &position);
