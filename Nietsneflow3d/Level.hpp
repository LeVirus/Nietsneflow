#ifndef LEVEL_H
#define LEVEL_H

#include <stdint.h>
#include <functional>
#include <vector>
#include <constants.hpp>

using vectPairUI = std::vector<std::pair<uint32_t, uint32_t>>;
using pairFloat_t = std::pair<float, float>;

struct StaticLevelElementData
{
    uint8_t m_numSprite;
    //In Game sprite size in % relative to a tile
    std::pair<float, float> m_inGameSpriteSize;
    vectPairUI m_TileGamePosition;
    bool m_traversable;
};

struct WallData
{
    uint8_t m_numSprite;
    vectPairUI m_TileGamePosition;
};

struct DoorData
{
    uint8_t m_numSprite;
    vectPairUI m_TileGamePosition;
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
    vectPairUI m_TileGamePosition;
    bool m_traversable;
};

class Level
{
private:
    std::pair<uint32_t, uint32_t> m_size,
                                  m_playerDeparture;
    Direction_e m_playerDirection;
    std::vector<StaticLevelElementData> m_groundElement,
                                    m_ceilingElement,
                                    m_objectElement;
    std::vector<WallData> m_wallData;
    std::vector<EnemyData> m_enemyData;
public:
    Level();
    void setLevelSize(const pairFloat_t &pairLevelSize);
    void setPlayerInitData(const pairFloat_t &pairInitPlayerPos, Direction_e playerDir);
};

#endif // LEVEL_H
