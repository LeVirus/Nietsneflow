#include <ECS/Components/EnemyConfComponent.hpp>
#include <ECS/Components/MapCoordComponent.hpp>
#include <ECS/Components/EnemyConfComponent.hpp>
#include <ECS/Components/MoveableComponent.hpp>
#include <ECS/Components/SegmentCollisionComponent.hpp>
#include <ECS/Components/GeneralCollisionComponent.hpp>
#include <ECS/Components/TimerComponent.hpp>
#include <cassert>
#include <iostream>
#include "IASystem.hpp"
#include "PhysicalEngine.hpp"
#include "CollisionUtils.hpp"
#include "MainEngine.hpp"

//===================================================================
IASystem::IASystem()
{
    bAddComponentToSystem(Components_e::ENEMY_CONF_COMPONENT);
}

//===================================================================
void IASystem::execSystem()
{
    assert(m_playerMapComp);
    System::execSystem();
    MapCoordComponent *enemyMapComp;
    EnemyConfComponent *enemyConfComp;
    TimerComponent *timerComp;
    float distancePlayer;
    for(uint32_t i = 0; i < mVectNumEntity.size(); ++i)
    {
        enemyConfComp = stairwayToComponentManager().searchComponentByType<EnemyConfComponent>(
                    mVectNumEntity[i], Components_e::ENEMY_CONF_COMPONENT);
        assert(enemyConfComp);
        if(enemyConfComp->m_behaviourMode == EnemyBehaviourMode_e::DEAD)
        {
            continue;
        }
        enemyMapComp = stairwayToComponentManager().searchComponentByType<MapCoordComponent>(
                    mVectNumEntity[i], Components_e::MAP_COORD_COMPONENT);
        assert(enemyMapComp);
        distancePlayer = getDistance(m_playerMapComp->m_absoluteMapPositionPX,
                                     enemyMapComp->m_absoluteMapPositionPX);
        if(enemyConfComp->m_behaviourMode != EnemyBehaviourMode_e::ATTACK &&
                distancePlayer < m_distanceEnemyBehaviour)
        {
            timerComp = stairwayToComponentManager().searchComponentByType<TimerComponent>(
                        mVectNumEntity[i], Components_e::TIMER_COMPONENT);
            assert(timerComp);
            timerComp->m_clockB = std::chrono::system_clock::now();
            enemyConfComp->m_behaviourMode = EnemyBehaviourMode_e::ATTACK;
        }
        if(enemyConfComp->m_behaviourMode == EnemyBehaviourMode_e::ATTACK)
        {
            treatEnemyBehaviourAttack(mVectNumEntity[i], enemyMapComp, enemyConfComp, distancePlayer);
        }
    }
}

//===================================================================
void IASystem::treatEnemyBehaviourAttack(uint32_t enemyEntity, MapCoordComponent *enemyMapComp,
                                         EnemyConfComponent *enemyConfComp, float distancePlayer)
{
    MoveableComponent *moveComp = stairwayToComponentManager().
            searchComponentByType<MoveableComponent>(enemyEntity, Components_e::MOVEABLE_COMPONENT);
    assert(moveComp);
    moveComp->m_degreeOrientation = getTrigoAngle(enemyMapComp->m_absoluteMapPositionPX,
                                                  m_playerMapComp->m_absoluteMapPositionPX);
    SegmentCollisionComponent *segmentComp = stairwayToComponentManager().
            searchComponentByType<SegmentCollisionComponent>(*enemyConfComp->m_ammo[0],
            Components_e::SEGMENT_COLLISION_COMPONENT);
    GeneralCollisionComponent *genComp = stairwayToComponentManager().
            searchComponentByType<GeneralCollisionComponent>(*enemyConfComp->m_ammo[0],
            Components_e::GENERAL_COLLISION_COMPONENT);
    assert(segmentComp);
    assert(genComp);
    TimerComponent *timerComp = stairwayToComponentManager().searchComponentByType<TimerComponent>(
                enemyEntity, Components_e::TIMER_COMPONENT);
    assert(timerComp);
    std::chrono::duration<double> elapsed_seconds = std::chrono::system_clock::now() -
            timerComp->m_clockB;
    if(elapsed_seconds.count() > 0.5)
    {
        timerComp->m_clockB = std::chrono::system_clock::now();
        confBullet(genComp, segmentComp, CollisionTag_e::BULLET_ENEMY_CT,
                   enemyMapComp->m_absoluteMapPositionPX, moveComp->m_degreeOrientation);
    }
    if(distancePlayer > LEVEL_TILE_SIZE_PX)
    {
        moveElement(*moveComp, *enemyMapComp, MoveOrientation_e::FORWARD);
    }
}

//===================================================================
void IASystem::loadPlayerDatas(uint32_t playerEntity)
{
    m_playerEntity = playerEntity;
    m_playerMapComp = stairwayToComponentManager().searchComponentByType<MapCoordComponent>(
                m_playerEntity, Components_e::MAP_COORD_COMPONENT);
    assert(m_playerMapComp);
}

