#pragma once

#include <stdint.h>
#include <functional>
#include <vector>
#include <set>
#include <constants.hpp>
#include <ECS/Components/ObjectConfComponent.hpp>

using VectPairUI_t = std::vector<PairUI_t>;

struct MoveableComponent;
struct PositionVertexComponent;
struct SpriteTextureComponent;

struct TeleportData
{
    VectPairUI_t m_targetTeleport;
    std::vector<bool> m_biDirection;
};

struct BarrelData
{
    VectPairUI_t m_TileGamePosition;
    std::vector<uint16_t> m_staticSprite, m_explosionSprite;
    PairDouble_t m_inGameStaticSpriteSize;
    std::vector<PairFloat_t> m_vectinGameExplosionSpriteSize;
    std::string m_explosionSoundFile;
};

struct StaticLevelElementData
{
    //GLOBAL
    uint16_t m_numSprite;
    //In Game sprite size in % relative to a tile
    PairDouble_t m_inGameSpriteSize;
    VectPairUI_t m_TileGamePosition;
    ObjectType_e m_type;
    bool m_traversable = true;
    //OBJECT
    std::optional<uint32_t> m_weaponID, m_cardID = std::nullopt;
    std::string m_cardName;
    uint32_t m_containing;
    //TELEPORT
    std::optional<TeleportData> m_teleportData;
};

struct DoorData
{
    uint16_t m_numSprite;
    VectPairUI_t m_TileGamePosition;
    std::optional<std::pair<uint32_t, std::string>> m_cardID = std::nullopt;
    //Position of axe true = vertical false = horizontal
    bool m_vertical;
};

struct ElementRaycast
{
    //first mem origin case type second moveable wall entities which on the case
    std::optional<std::set<uint32_t>> m_memMoveWall, m_memStaticMoveableWall;
    bool m_moveableWallStopped = false;
    uint32_t m_numEntity;
    LevelCaseType_e m_type;
    //Base level Type without moveable wall
    LevelCaseType_e m_typeStd;
    PairUI_t m_tileGamePosition;
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
    std::vector<uint16_t> m_staticFrontSprites, m_staticFrontLeftSprites, m_staticFrontRightSprites,
    m_staticBackSprites, m_staticBackLeftSprites, m_staticBackRightSprites,
    m_staticLeftSprites, m_staticRightSprites, m_attackSprites, m_dyingSprites, m_touched;

    //In Game sprite size in % relative to a tile
    PairDouble_t m_inGameSpriteSize;
    VectPairUI_t m_TileGamePosition;
    std::string m_visibleShootID, m_impactID, m_dropedObjectID;
    std::string m_detectBehaviourSoundFile, m_attackSoundFile, m_deathSoundFile;
    uint32_t m_attackPower, m_life;
    std::optional<uint32_t> m_meleeDamage, m_simultaneousShot;
    bool m_frozenOnAttack;
    std::optional<PairUI_t> m_endLevelPos;
    std::optional<float> m_damageZone;
    float m_velocity, m_shotVelocity;
    bool m_traversable, m_meleeOnly;
};

class Level
{
public:
    Level();
    void setPlayerInitData(const PairFloat_t &pairInitPlayerPos,
                           Direction_e playerDir);
    static void initLevelElementArray();
    static void clearLevelElement(const PairUI_t &tilePosition);
    static const std::vector<ElementRaycast> &getLevelCaseType()
    {
        return m_levelCaseType;
    }
    static void addElementCase(SpriteTextureComponent &spriteComp, const PairUI_t &tilePosition,
                               LevelCaseType_e type, uint32_t numEntity);
    static void memStaticMoveWallEntity(const PairUI_t &tilePosition, uint32_t entity);
    static std::optional<ElementRaycast> getElementCase(const PairUI_t &tilePosition);
    static void memMoveWallEntity(const PairUI_t &tilePosition, uint32_t entity);
    static void setElementTypeCase(const PairUI_t &tilePosition, LevelCaseType_e type);
    static void setMoveableWallStopped(const PairUI_t &tilePosition, bool stopped);
    static void setElementEntityCase(const PairUI_t &tilePosition, uint32_t entity);
    inline static void setMusicFile(const std::string filename)
    {
        m_musicFile = filename;
    }
    inline static void clearMusicFilePath()
    {
        m_musicFile.clear();
    }
    inline static std::string getMusicFilename()
    {
        return m_musicFile;
    }
    //in the case of moveable wall reset case
    static void resetMoveWallElementCase(const PairUI_t &tilePosition, uint32_t numEntity);
    static bool removeStaticMoveWallElementCase(const PairUI_t &tilePosition, uint32_t numEntity);

    static uint32_t getLevelCaseIndex(const PairUI_t &tilePosition);

    inline const PairUI_t &getPlayerDeparture()const
    {
        return m_playerDeparture;
    }
    static void setStandardElementTypeCase(const PairUI_t &tilePosition, LevelCaseType_e type);

    static inline PairUI_t &getSize()
    {
        return m_size;
    }

    inline Direction_e getPlayerDepartureDirection()const
    {
        return m_playerDepartureDirection;
    }

    static inline void setLevelSize(const PairFloat_t &pairLevelSize)
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
    PairUI_t m_playerDeparture;
    static PairUI_t m_size;
    Direction_e m_playerDepartureDirection;
    static std::vector<ElementRaycast> m_levelCaseType;
    static float m_rangeViewPX;
    static std::string m_musicFile;
};

PairFloat_t getAbsolutePosition(const PairUI_t &coord);
PairFloat_t getCenteredAbsolutePosition(const PairUI_t &coord);

std::optional<PairUI_t> getLevelCoord(const PairFloat_t &position);
