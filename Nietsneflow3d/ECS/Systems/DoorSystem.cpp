#include "DoorSystem.hpp"
#include "constants.hpp"
#include <ECS/Components/DoorComponent.hpp>
#include <ECS/Components/TimerComponent.hpp>
#include <ECS/Components/MapCoordComponent.hpp>
#include <ECS/Components/RectangleCollisionComponent.hpp>
#include <cassert>

//===================================================================
DoorSystem::DoorSystem()
{
    setUsedComponents();
}

//===================================================================
void DoorSystem::execSystem()
{
    System::execSystem();
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
                timerComp->m_clock;
        if(doorComp->m_currentState == DoorState_e::STATIC_OPEN)
        {
            if(elapsed_seconds.count() > m_timeDoorClosed)
            {
                doorComp->m_currentState = DoorState_e::MOVE_CLOSE;
                timerComp->m_clock = std::chrono::system_clock::now();
            }
            continue;
        }
        if(elapsed_seconds.count() > doorComp->m_speedMove)
        {
            treatDoorMovementSize(doorComp, mVectNumEntity[i]);
            timerComp->m_clock = std::chrono::system_clock::now();
        }
    }
}

//===================================================================
void DoorSystem::treatDoorMovementSize(DoorComponent *doorComp, uint32_t entityNum)
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

//===================================================================
void DoorSystem::setUsedComponents()
{
    bAddComponentToSystem(Components_e::DOOR_COMPONENT);
}

