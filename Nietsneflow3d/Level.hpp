#pragma once

#include <stdint.h>
#include <functional>
#include <vector>
#include <set>
#include <constants.hpp>
#include <ECS/Components/ObjectConfComponent.hpp>

using vectPairUI_t = std::vector<pairUI_t>;
using PairMemMoveableWall_t = std::pair<LevelCaseType_e, uint32_t>;

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
    std::optional<uint32_t> m_weaponID, m_cardID = std::nullopt;
    uint32_t m_containing;
    bool m_traversable = true;
};

struct DoorData
{
    uint8_t m_numSprite;
    vectPairUI_t m_TileGamePosition;
    std::optional<std::pair<uint32_t, std::string>> m_cardID = std::nullopt;
    //Position of axe true = vertical false = horizontal
    bool m_vertical;
};

struct ElementRaycast
{
    //first mem origin case type second moveable wall entities which on the case
    std::optional<PairMemMoveableWall_t> m_memMoveWall;
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
    m_staticLeftSprites, m_staticRightSprites, m_attackSprites, m_dyingSprites, m_touched;

    //In Game sprite size in % relative to a tile
    pairDouble_t m_inGameSpriteSize;
    vectPairUI_t m_TileGamePosition;
    std::string m_visibleShootID, m_impactID;
    uint32_t m_attackPower, m_life;
    std::optional<uint32_t> m_meleeDamage;
    float m_velocity, m_shotVelocity;
    std::string m_dropedObjectID;
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
    static void memMoveWallEntity(const pairUI_t &tilePosition, LevelCaseType_e type, uint32_t entity);
    static void setElementTypeCase(const pairUI_t &tilePosition, LevelCaseType_e type);
    static void setElementEntityCase(const pairUI_t &tilePosition, uint32_t entity);
    //in the case of moveable wall reset case
    static void resetElementCase(const pairUI_t &tilePosition);

    static uint32_t getLevelCaseIndex(const pairUI_t &tilePosition);

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

    static inline void setLevelSize(const pairFloat_t &pairLevelSize)
    {
        m_size = pairLevelSize;
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
    static std::vector<ElementRaycast> m_levelCaseType;
    static float m_rangeViewPX;
};

pairFloat_t getAbsolutePosition(const pairUI_t &coord);
pairFloat_t getCenteredAbsolutePosition(const pairUI_t &coord);

std::optional<pairUI_t> getLevelCoord(const pairFloat_t &position);
