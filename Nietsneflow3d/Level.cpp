#include "Level.hpp"
#include "constants.hpp"
#include <ECS/Components/MapCoordComponent.hpp>
#include <ECS/Components/MoveableComponent.hpp>
#include <cassert>

float Level::m_rangeViewPX;
pairUI_t Level::m_size;
std::vector<ElementRaycast> Level::m_levelCaseType;


//===================================================================
Level::Level()
{
    m_rangeViewPX = 100.0f;
}

//===================================================================
void Level::setPlayerInitData(const pairFloat_t &pairInitPlayerPos,
                              Direction_e playerDir)
{
    m_playerDeparture = pairInitPlayerPos;
    m_playerDepartureDirection = playerDir;
}

//===================================================================
void Level::initLevelElementArray()
{
    m_levelCaseType.resize(m_size.first * m_size.second);
    for(uint32_t i = 0; i < m_levelCaseType.size(); ++i)
    {
        m_levelCaseType[i].m_type = LevelCaseType_e::EMPTY_LC;
    }
}
//===================================================================
void Level::addElementCase(SpriteTextureComponent *spriteComp, const pairUI_t &tilePosition,
                           LevelCaseType_e type, uint32_t numEntity)
{
    uint32_t index = getLevelCaseIndex(tilePosition);
    m_levelCaseType[index].m_numEntity = numEntity;
    m_levelCaseType[index].m_type = type;
    m_levelCaseType[index].m_tileGamePosition = tilePosition;
    m_levelCaseType[index].m_spriteComp = spriteComp;
}

//===================================================================
std::optional<ElementRaycast> Level::getElementCase(const pairUI_t &tilePosition)
{
    if((tilePosition.first >= m_size.first) ||
            (tilePosition.second >= m_size.second))
    {
        return std::nullopt;
    }
    return m_levelCaseType[getLevelCaseIndex(tilePosition)];
}

//===================================================================
uint32_t Level::getLevelCaseIndex(const pairUI_t &tilePosition)
{
    uint32_t index = (tilePosition.second * m_size.first + tilePosition.first);
    assert(tilePosition.first < m_size.first);
    assert(tilePosition.second < m_size.second);
    return index;
}

//===================================================================
pairFloat_t getAbsolutePosition(const pairUI_t &coord)
{
    return {static_cast<float>(coord.first) * LEVEL_TILE_SIZE_PX,
                static_cast<float>(coord.second) * LEVEL_TILE_SIZE_PX};
}

//===================================================================
pairUI_t getLevelCoord(const pairFloat_t &position)
{
    return {static_cast<uint32_t>(position.first / LEVEL_TILE_SIZE_PX),
                static_cast<uint32_t>(position.second / LEVEL_TILE_SIZE_PX)};
}
