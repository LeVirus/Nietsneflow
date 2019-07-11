#include "Level.hpp"

//===================================================================
Level::Level()
{

}

//===================================================================
void Level::setLevelSize(const pairFloat_t &pairLevelSize)
{
    m_size = pairLevelSize;
}

//===================================================================
void Level::setPlayerInitData(const pairFloat_t &pairInitPlayerPos, Direction_e playerDir)
{
    m_playerDeparture = pairInitPlayerPos;
    m_playerDirection = playerDir;
}

