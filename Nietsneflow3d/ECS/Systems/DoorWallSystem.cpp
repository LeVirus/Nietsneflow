#include "DoorWallSystem.hpp"
#include "constants.hpp"
#include <Level.hpp>
#include <ECS/ECSManager.hpp>
#include <ECS/Components/DoorComponent.hpp>
#include <ECS/Components/TimerComponent.hpp>
#include <ECS/Components/MapCoordComponent.hpp>
#include <ECS/Components/RectangleCollisionComponent.hpp>
#include <ECS/Components/MoveableWallConfComponent.hpp>
#include <ECS/Components/MoveableComponent.hpp>
#include <cassert>

//===================================================================
DoorWallSystem::DoorWallSystem(const ECSManager *memECSManager)
{
    m_ECSManager = memECSManager;
    bAddComponentToSystem(Components_e::DOOR_COMPONENT);
}

//===================================================================
void DoorWallSystem::updateEntities()
{
    System::execSystem();
    std::bitset<TOTAL_COMPONENTS> bitset;
    bitset[MOVEABLE_WALL_CONF_COMPONENT] = true;
    m_vectMoveableWall = m_ECSManager->getEntitiesContainingComponents(bitset);
}


//===================================================================
void DoorWallSystem::execSystem()
{
    if(mVectNumEntity.empty() && m_vectMoveableWall.empty())
    {
        updateEntities();
    }
    treatDoors();
    treatWalls();
}

//===================================================================
void DoorWallSystem::treatDoors()
{
    for(uint32_t i = 0; i < mVectNumEntity.size(); ++i)
    {
        DoorComponent *doorComp = stairwayToComponentManager().
                searchComponentByType<DoorComponent>(mVectNumEntity[i],
                                                     Components_e::DOOR_COMPONENT);
        assert(doorComp);
        if(doorComp->m_currentState == DoorState_e::STATIC_CLOSED)
        {
            continue;
        }
        TimerComponent *timerComp = stairwayToComponentManager().
                searchComponentByType<TimerComponent>(mVectNumEntity[i],
                                                     Components_e::TIMER_COMPONENT);
        assert(timerComp);
        std::chrono::duration<double> elapsed_seconds = std::chrono::system_clock::now() -
                timerComp->m_clockA;
        if(doorComp->m_currentState == DoorState_e::STATIC_OPEN)
        {
            if(elapsed_seconds.count() > m_timeDoorClosed)
            {
                doorComp->m_currentState = DoorState_e::MOVE_CLOSE;
                timerComp->m_clockA = std::chrono::system_clock::now();
            }
            continue;
        }
        if(elapsed_seconds.count() > doorComp->m_speedMove)
        {
            treatDoorMovementSize(doorComp, mVectNumEntity[i]);
            timerComp->m_clockA = std::chrono::system_clock::now();
        }
    }
}

//===================================================================
void DoorWallSystem::treatWalls()
{
    bool next;
    pairUI_t memPreviousPos;
    for(uint32_t i = 0; i < m_vectMoveableWall.size(); ++i)
    {
        MoveableWallConfComponent *moveWallComp = stairwayToComponentManager().
                searchComponentByType<MoveableWallConfComponent>(m_vectMoveableWall[i],
                                                     Components_e::MOVEABLE_WALL_CONF_COMPONENT);
        assert(moveWallComp);
        if(!moveWallComp->m_inMovement)
        {
            continue;
        }
        next = false;
        MapCoordComponent *mapComp = stairwayToComponentManager().
                searchComponentByType<MapCoordComponent>(m_vectMoveableWall[i],
                                                         Components_e::MAP_COORD_COMPONENT);
        assert(mapComp);
        Direction_e currentDir = moveWallComp->m_directionMove[moveWallComp->m_currentPhase].first;
        if(moveWallComp->m_initPos)
        {
            setInitPhaseMoveWall(mapComp, moveWallComp, currentDir, m_vectMoveableWall[i]);
        }
        MoveableComponent *moveComp = stairwayToComponentManager().
                searchComponentByType<MoveableComponent>(m_vectMoveableWall[i],
                                                         Components_e::MOVEABLE_COMPONENT);
        assert(moveComp);
        memPreviousPos = mapComp->m_coord;
        switch(currentDir)
        {
        case Direction_e::EAST:
            mapComp->m_absoluteMapPositionPX.first += moveComp->m_velocity;
            if(mapComp->m_absoluteMapPositionPX.first >= moveWallComp->m_nextPhasePos.first)
            {
                ++mapComp->m_coord.first;
                next = true;
            }
            break;
        case Direction_e::WEST:
            mapComp->m_absoluteMapPositionPX.first -= moveComp->m_velocity;
            if(mapComp->m_absoluteMapPositionPX.first <= moveWallComp->m_nextPhasePos.first)
            {
                --mapComp->m_coord.first;
                next = true;
            }
            break;
        case Direction_e::NORTH:
            mapComp->m_absoluteMapPositionPX.second -= moveComp->m_velocity;
            if(mapComp->m_absoluteMapPositionPX.second <= moveWallComp->m_nextPhasePos.second)
            {
                --mapComp->m_coord.second;
                next = true;
            }
            break;
        case Direction_e::SOUTH:
            mapComp->m_absoluteMapPositionPX.second += moveComp->m_velocity;
            if(mapComp->m_absoluteMapPositionPX.second >= moveWallComp->m_nextPhasePos.second)
            {
                ++mapComp->m_coord.second;
                next = true;
            }
            break;
        }
        if(next)
        {
            switchToNextPhaseMoveWall(mapComp, moveWallComp, memPreviousPos);
        }
    }
}


//===================================================================
void setInitPhaseMoveWall(MapCoordComponent *mapComp, MoveableWallConfComponent *moveWallComp,
                          Direction_e currentDir, uint32_t wallEntity)
{
    pairUI_t nextCase;
    if(Level::getElementCase(mapComp->m_coord)->m_type != LevelCaseType_e::WALL_LC)
    {
        Level::setElementTypeCase(mapComp->m_coord, LevelCaseType_e::WALL_MOVE_LC);
    }
    moveWallComp->m_initPos = false;
    switch(currentDir)
    {
    case Direction_e::EAST:
        moveWallComp->m_nextPhasePos = {mapComp->m_absoluteMapPositionPX.first + LEVEL_TILE_SIZE_PX,
                                        mapComp->m_absoluteMapPositionPX.second};
        nextCase = {mapComp->m_coord.first + 1, mapComp->m_coord.second};
        break;
    case Direction_e::WEST:
        moveWallComp->m_nextPhasePos = {mapComp->m_absoluteMapPositionPX.first - LEVEL_TILE_SIZE_PX,
                                        mapComp->m_absoluteMapPositionPX.second};
        nextCase = {mapComp->m_coord.first - 1, mapComp->m_coord.second};
        break;
    case Direction_e::NORTH:
        moveWallComp->m_nextPhasePos = {mapComp->m_absoluteMapPositionPX.first,
                                        mapComp->m_absoluteMapPositionPX.second - LEVEL_TILE_SIZE_PX};
        nextCase = {mapComp->m_coord.first, mapComp->m_coord.second - 1};
        break;
    case Direction_e::SOUTH:
        moveWallComp->m_nextPhasePos = {mapComp->m_absoluteMapPositionPX.first,
                                        mapComp->m_absoluteMapPositionPX.second + LEVEL_TILE_SIZE_PX};
        nextCase = {mapComp->m_coord.first, mapComp->m_coord.second + 1};
        break;
    }
    LevelCaseType_e type = Level::getElementCase(nextCase)->m_type;
    Level::memMoveWallEntity(nextCase, type, wallEntity);
    if(type != LevelCaseType_e::WALL_LC)
    {
        Level::setElementTypeCase(nextCase, LevelCaseType_e::WALL_MOVE_LC);
    }
}

//===================================================================
void switchToNextPhaseMoveWall(MapCoordComponent *mapComp,
                               MoveableWallConfComponent *moveWallComp,
                               const pairUI_t &previousPos)
{
    Level::resetElementCase(previousPos);
    moveWallComp->m_initPos = true;
    if(++moveWallComp->m_currentMove >=
            moveWallComp->m_directionMove[moveWallComp->m_currentPhase].second)
    {
        //IF CYCLE END
        if(++moveWallComp->m_currentPhase == moveWallComp->m_directionMove.size())
        {
            std::optional<ElementRaycast> element = Level::getElementCase(mapComp->m_coord);
            if(element->m_type != LevelCaseType_e::WALL_LC)
            {
                Level::setElementTypeCase(mapComp->m_coord, LevelCaseType_e::WALL_LC);
                Level::setElementEntityCase(mapComp->m_coord, element->m_memMoveWall->second[0]);
            }
            moveWallComp->m_inMovement = false;
        }
    }
}

//===================================================================
void DoorWallSystem::clearSystem()
{
    mVectNumEntity.clear();
    m_vectMoveableWall.clear();
}

//===================================================================
void DoorWallSystem::treatDoorMovementSize(DoorComponent *doorComp, uint32_t entityNum)
{
    RectangleCollisionComponent *rectComp = stairwayToComponentManager().
            searchComponentByType<RectangleCollisionComponent>(entityNum,
                                                               Components_e::RECTANGLE_COLLISION_COMPONENT);
    assert(rectComp);
    if(doorComp->m_currentState == DoorState_e::MOVE_CLOSE)
    {
        if(doorComp->m_vertical)
        {
            rectComp->m_size.second += 1.0f;
            if(rectComp->m_size.second >= LEVEL_TILE_SIZE_PX)
            {
                doorComp->m_currentState = DoorState_e::STATIC_CLOSED;
                rectComp->m_size.second = LEVEL_TILE_SIZE_PX;
            }
        }
        else
        {
            rectComp->m_size.first += 1.0f;
            if(rectComp->m_size.first >= LEVEL_TILE_SIZE_PX)
            {
                doorComp->m_currentState = DoorState_e::STATIC_CLOSED;
                rectComp->m_size.first = LEVEL_TILE_SIZE_PX;
            }
        }
    }
    else if(doorComp->m_currentState == DoorState_e::MOVE_OPEN)
    {
        if(doorComp->m_vertical)
        {
            rectComp->m_size.second -= 1.0f;
            if(rectComp->m_size.second <= 0.0f)
            {
                doorComp->m_currentState = DoorState_e::STATIC_OPEN;
                rectComp->m_size.second = 0.0f;
            }
        }
        else
        {
            rectComp->m_size.first -= 1.0f;
            if(rectComp->m_size.first <= 0.0f)
            {
                doorComp->m_currentState = DoorState_e::STATIC_OPEN;
                rectComp->m_size.first = 0.0f;
            }
        }
    }
}
