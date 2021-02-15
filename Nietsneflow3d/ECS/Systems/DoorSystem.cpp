#include "DoorSystem.hpp"
#include "constants.hpp"
#include <ECS/Components/DoorComponent.hpp>
#include <ECS/Components/TimerComponent.hpp>
#include <ECS/Components/MapCoordComponent.hpp>
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
        std::chrono::duration<double> elapsed_seconds = std::chrono::system_clock::now() - timerComp->m_clock;
        if(doorComp->m_currentState == DoorState_e::STATIC_OPEN)
        {
            if(elapsed_seconds.count() > 3.0)
            {
                doorComp->m_currentState = DoorState_e::MOVE_CLOSE;
                timerComp->m_clock = std::chrono::system_clock::now();
            }
            continue;
        }
        if(elapsed_seconds.count() > 0.01)
        {
            MapCoordComponent *mapComp = stairwayToComponentManager().
                    searchComponentByType<MapCoordComponent>(mVectNumEntity[i],
                                                             Components_e::MAP_COORD_COMPONENT);
            assert(mapComp);
            if(doorComp->m_currentState == DoorState_e::MOVE_CLOSE)
            {
                if(doorComp->m_vertical)
                {
                    mapComp->m_absoluteMapPositionPX.second += doorComp->m_speedMove;
                    if(mapComp->m_absoluteMapPositionPX.second >= doorComp->m_initPosition.second)
                    {
                        doorComp->m_currentState = DoorState_e::STATIC_CLOSED;
                        mapComp->m_absoluteMapPositionPX.second = doorComp->m_initPosition.second;
                    }
                }
                else
                {
                    mapComp->m_absoluteMapPositionPX.first += doorComp->m_speedMove;
                    if(mapComp->m_absoluteMapPositionPX.first >= doorComp->m_initPosition.first)
                    {
                        doorComp->m_currentState = DoorState_e::STATIC_CLOSED;
                        mapComp->m_absoluteMapPositionPX.first = doorComp->m_initPosition.first;
                    }
                }
            }
            else if(doorComp->m_currentState == DoorState_e::MOVE_OPEN)
            {
                float openPosition;
                if(doorComp->m_vertical)
                {
                    openPosition = doorComp->m_initPosition.second - LEVEL_TILE_SIZE_PX;
                    mapComp->m_absoluteMapPositionPX.second -= doorComp->m_speedMove;
                    if(mapComp->m_absoluteMapPositionPX.second <= openPosition)
                    {
                        doorComp->m_currentState = DoorState_e::STATIC_OPEN;
                        mapComp->m_absoluteMapPositionPX.second = openPosition;
                    }
                }
                else
                {
                    openPosition = doorComp->m_initPosition.first - LEVEL_TILE_SIZE_PX;
                    mapComp->m_absoluteMapPositionPX.first -= doorComp->m_speedMove;
                    if(mapComp->m_absoluteMapPositionPX.first <= openPosition)
                    {
                        doorComp->m_currentState = DoorState_e::STATIC_OPEN;
                        mapComp->m_absoluteMapPositionPX.first = openPosition;
                    }
                }
            }
            timerComp->m_clock = std::chrono::system_clock::now();
        }
    }

}

//===================================================================
void DoorSystem::setUsedComponents()
{
    bAddComponentToSystem(Components_e::DOOR_COMPONENT);
}

