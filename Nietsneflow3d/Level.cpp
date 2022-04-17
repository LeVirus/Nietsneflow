#include "Level.hpp"
#include "constants.hpp"
#include <ECS/Components/MapCoordComponent.hpp>
#include <ECS/Components/MoveableComponent.hpp>
#include <cassert>

float Level::m_rangeViewPX;
PairUI_t Level::m_size;
std::vector<ElementRaycast> Level::m_levelCaseType;
std::string Level::m_musicFile;

//===================================================================
Level::Level()
{
    m_rangeViewPX = 200.0f;
}

//===================================================================
void Level::setPlayerInitData(const PairFloat_t &pairInitPlayerPos,
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
        m_levelCaseType[i].m_typeStd = LevelCaseType_e::EMPTY_LC;
    }
}

//===================================================================
void Level::clearLevelElement(const PairUI_t &tilePosition)
{
    uint32_t index = getLevelCaseIndex(tilePosition);
    m_levelCaseType[index].m_type = LevelCaseType_e::EMPTY_LC;
    m_levelCaseType[index].m_typeStd = LevelCaseType_e::EMPTY_LC;
}
//===================================================================
void Level::addElementCase(SpriteTextureComponent *spriteComp, const PairUI_t &tilePosition,
                           LevelCaseType_e type, uint32_t numEntity)
{
    uint32_t index = getLevelCaseIndex(tilePosition);
    m_levelCaseType[index].m_numEntity = numEntity;
    if(type != LevelCaseType_e::WALL_MOVE_LC)
    {
        m_levelCaseType[index].m_type = type;
        m_levelCaseType[index].m_typeStd = type;
    }
    else
    {
        m_levelCaseType[index].m_type = LevelCaseType_e::WALL_LC;
        m_levelCaseType[index].m_typeStd = LevelCaseType_e::EMPTY_LC;
    }
    m_levelCaseType[index].m_tileGamePosition = tilePosition;
    m_levelCaseType[index].m_spriteComp = spriteComp;
}

//===================================================================
void Level::memStaticMoveWallEntity(const PairUI_t &tilePosition, uint32_t entity)
{
    uint32_t index = getLevelCaseIndex(tilePosition);
    if(!m_levelCaseType[index].m_memStaticMoveableWall)
    {
        m_levelCaseType[index].m_memStaticMoveableWall = std::set<uint32_t>();
    }
    m_levelCaseType[index].m_memStaticMoveableWall->insert(entity);
    setElementTypeCase(tilePosition, LevelCaseType_e::WALL_LC);
}

//===================================================================
std::optional<ElementRaycast> Level::getElementCase(const PairUI_t &tilePosition)
{
    if((tilePosition.first >= m_size.first) ||
            (tilePosition.second >= m_size.second))
    {
        return std::nullopt;
    }
    return m_levelCaseType[getLevelCaseIndex(tilePosition)];
}

//===================================================================
void Level::memMoveWallEntity(const PairUI_t &tilePosition, uint32_t entity)
{
    ElementRaycast &element = m_levelCaseType[getLevelCaseIndex(tilePosition)];
    if(!element.m_memMoveWall)
    {
        element.m_memMoveWall = std::set<uint32_t>();
    }
    element.m_memMoveWall->insert(entity);
    element.m_moveableWallStopped = false;
    element.m_type = LevelCaseType_e::WALL_MOVE_LC;
}

//===================================================================
void Level::setElementTypeCase(const PairUI_t &tilePosition, LevelCaseType_e type)
{
    m_levelCaseType[getLevelCaseIndex(tilePosition)].m_type = type;
}

//===================================================================
void Level::setStandardElementTypeCase(const PairUI_t &tilePosition, LevelCaseType_e type)
{
    m_levelCaseType[getLevelCaseIndex(tilePosition)].m_typeStd = type;
}

//===================================================================
void Level::setMoveableWallStopped(const PairUI_t &tilePosition, bool stopped)
{
    m_levelCaseType[getLevelCaseIndex(tilePosition)].m_moveableWallStopped = stopped;
}

//===================================================================
void Level::setElementEntityCase(const PairUI_t &tilePosition, uint32_t entity)
{
    m_levelCaseType[getLevelCaseIndex(tilePosition)].m_numEntity = entity;
}

//===================================================================
void Level::resetMoveWallElementCase(const PairUI_t &tilePosition, uint32_t numEntity)
{
    ElementRaycast &element = m_levelCaseType[getLevelCaseIndex(tilePosition)];
    if(!element.m_memMoveWall)
    {
        return;
    }
    std::set<uint32_t>::iterator it = element.m_memMoveWall->find(numEntity);
    if(it == element.m_memMoveWall->end())
    {
        return;
    }
    element.m_memMoveWall->erase(*it);
}

//===================================================================
bool Level::removeStaticMoveWallElementCase(const PairUI_t &tilePosition, uint32_t numEntity)
{
    ElementRaycast &element = m_levelCaseType[getLevelCaseIndex(tilePosition)];
    if(!element.m_memStaticMoveableWall)
    {
        return false;
    }
    std::set<uint32_t>::iterator it = element.m_memStaticMoveableWall->find(numEntity);
    if(it == element.m_memStaticMoveableWall->end())
    {
        return !element.m_memStaticMoveableWall->empty();
    }
    element.m_memStaticMoveableWall->erase(*it);
    if(element.m_memStaticMoveableWall->empty())
    {
        if(!element.m_moveableWallStopped)
        {
            element.m_type = element.m_typeStd;
        }
        element.m_memStaticMoveableWall.reset();
        assert(element.m_type != LevelCaseType_e::WALL_MOVE_LC);
    }
    else
    {
        if(numEntity == element.m_numEntity)
        {
            setElementEntityCase(tilePosition, *element.m_memStaticMoveableWall->begin());
        }
    }
    return !element.m_memStaticMoveableWall->empty();
}

//===================================================================
uint32_t Level::getLevelCaseIndex(const PairUI_t &tilePosition)
{
    uint32_t index = (tilePosition.second * m_size.first + tilePosition.first);
    assert(tilePosition.first < m_size.first);
    assert(tilePosition.second < m_size.second);
    return index;
}

//===================================================================
PairFloat_t getAbsolutePosition(const PairUI_t &coord)
{
    return {static_cast<float>(coord.first) * LEVEL_TILE_SIZE_PX,
                static_cast<float>(coord.second) * LEVEL_TILE_SIZE_PX};
}

//===================================================================
PairFloat_t getCenteredAbsolutePosition(const PairUI_t &coord)
{
    return {(static_cast<float>(coord.first) + 0.5f) * LEVEL_TILE_SIZE_PX,
                (static_cast<float>(coord.second + 0.5f)) * LEVEL_TILE_SIZE_PX};
}

//===================================================================
std::optional<PairUI_t> getLevelCoord(const PairFloat_t &position)
{
    if(position.first < 0.0f || position.second < 0.0f)
    {
        return {};
    }
    return {{static_cast<uint32_t>(position.first / LEVEL_TILE_SIZE_PX),
                static_cast<uint32_t>(position.second / LEVEL_TILE_SIZE_PX)}};
}
