#ifndef LEVEL_H
#define LEVEL_H

#include <stdint.h>
#include <functional>
#include <vector>
#include <constants.hpp>

using pairUI_t = std::pair<uint32_t, uint32_t>;
using vectPairUI_t = std::vector<pairUI_t>;
using pairFloat_t = std::pair<float, float>;

struct StaticLevelElementData
{
    uint8_t m_numSprite;
    //In Game sprite size in % relative to a tile
    std::pair<float, float> m_inGameSpriteSize;
    vectPairUI_t m_TileGamePosition;
    bool m_traversable;
};

struct WallData
{
    uint8_t m_numSprite;
    vectPairUI_t m_TileGamePosition;
};

struct DoorData
{
    uint8_t m_numSprite;
    vectPairUI_t m_TileGamePosition;
    //Position of axe true = vertical false = horizontal
    bool m_vertical;
};

struct EnemyData
{
    //m_staticSprites represent the motionless enemy
    //m_moveSprites represent the moving enemy
    //m_attackSprites represent the attacking enemy
    //m_dyingSprites represent the dying enemy

    //after dying animation the ennemy is represented by the last sprite
    //contained in m_dyingSprites
    std::vector<uint8_t> m_staticSprites, m_moveSprites,
    m_attackSprites, m_dyingSprites;

    //In Game sprite size in % relative to a tile
    pairFloat_t m_inGameSpriteSize;
    vectPairUI_t m_TileGamePosition;
    bool m_traversable;
};

class Level
{
private:
    pairUI_t m_size, m_playerDeparture;
    Direction_e m_playerDirection;
    std::vector<StaticLevelElementData> m_groundElement,
                                    m_ceilingElement,
                                    m_objectElement;
    std::vector<WallData> m_wallData;
    std::vector<DoorData> m_doorData;
    std::vector<EnemyData> m_enemyData;
public:
    Level();
    void setPlayerInitData(const pairFloat_t &pairInitPlayerPos,
                           Direction_e playerDir);
    inline const std::vector<WallData> &getWallData()const
    {
        return m_wallData;
    }

    inline void setLevelSize(const pairFloat_t &pairLevelSize)
    {
        m_size = pairLevelSize;
    }
    inline void setGroundElement(const std::vector<StaticLevelElementData> &vectGround)
    {
        m_groundElement = vectGround;
    }
    inline void setCeilingElement(const std::vector<StaticLevelElementData> &vectCeiling)
    {
        m_ceilingElement = vectCeiling;
    }

    inline void setObjectElement(const std::vector<StaticLevelElementData> &vectObject)
    {
        m_objectElement = vectObject;
    }
    inline void setWallElement(const std::vector<WallData> &vectWall)
    {
        m_wallData = vectWall;
    }
    inline void setDoorElement(const std::vector<DoorData> &vectDoor)
    {
        m_doorData = vectDoor;
    }
    inline void setEnemyElement(const std::vector<EnemyData> &vectEnemy)
    {
        m_enemyData = vectEnemy;
    }
    void display();//DEBUG
};

#endif // LEVEL_H
