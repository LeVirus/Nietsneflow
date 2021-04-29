#include <ECS/Components/EnemyConfComponent.hpp>
#include <ECS/Components/MapCoordComponent.hpp>
#include <ECS/Components/EnemyConfComponent.hpp>
#include <ECS/Components/MoveableComponent.hpp>
#include <ECS/Components/SegmentCollisionComponent.hpp>
#include <ECS/Components/GeneralCollisionComponent.hpp>
#include <ECS/Components/TimerComponent.hpp>
#include <cassert>
#include <random>
#include <iostream>
#include "IASystem.hpp"
#include "PhysicalEngine.hpp"
#include "CollisionUtils.hpp"
#include "MainEngine.hpp"

//===================================================================
IASystem::IASystem()
{
    std::srand(std::time(nullptr));
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
            MoveableComponent *moveComp = stairwayToComponentManager().
                    searchComponentByType<MoveableComponent>(mVectNumEntity[i],
                                                             Components_e::MOVEABLE_COMPONENT);
            assert(moveComp);
            updateEnemyDirection(enemyConfComp, moveComp, enemyMapComp);
        }
        if(enemyConfComp->m_behaviourMode == EnemyBehaviourMode_e::ATTACK)
        {
            treatEnemyBehaviourAttack(mVectNumEntity[i], enemyMapComp, enemyConfComp, distancePlayer);
        }
    }
}

//===================================================================
void IASystem::updateEnemyDirection(EnemyConfComponent *enemyConfComp, MoveableComponent *moveComp,
                                    MapCoordComponent *enemyMapComp)
{
    enemyConfComp->m_attackPhase =
            static_cast<EnemyAttackPhase_e>(std::rand() / ((RAND_MAX + 1u) / 4));
    if(enemyConfComp->m_attackPhase == EnemyAttackPhase_e::SHOOT)
    {
        GeneralCollisionComponent *genComp = stairwayToComponentManager().
                searchComponentByType<GeneralCollisionComponent>(
                    *enemyConfComp->m_ammo[0], Components_e::GENERAL_COLLISION_COMPONENT);
        SegmentCollisionComponent *segmentComp = stairwayToComponentManager().
                searchComponentByType<SegmentCollisionComponent>(
                    *enemyConfComp->m_ammo[0], Components_e::SEGMENT_COLLISION_COMPONENT);
        assert(genComp);
        assert(segmentComp);
        confBullet(genComp, segmentComp, CollisionTag_e::BULLET_ENEMY_CT,
                   enemyMapComp->m_absoluteMapPositionPX, moveComp->m_degreeOrientation);
    }
    moveComp->m_degreeOrientation = getTrigoAngle(enemyMapComp->m_absoluteMapPositionPX,
                                                  m_playerMapComp->m_absoluteMapPositionPX);
    if(enemyConfComp->m_attackPhase == EnemyAttackPhase_e::MOVE_TO_TARGET_RIGHT)
    {
        moveComp->m_degreeOrientation -= 30.0f;
    }
    else if(enemyConfComp->m_attackPhase == EnemyAttackPhase_e::MOVE_TO_TARGET_LEFT)
    {
        moveComp->m_degreeOrientation += 30.0f;
    }
}

//===================================================================
void IASystem::treatEnemyBehaviourAttack(uint32_t enemyEntity, MapCoordComponent *enemyMapComp,
                                         EnemyConfComponent *enemyConfComp, float distancePlayer)
{
    MoveableComponent *moveComp = stairwayToComponentManager().
            searchComponentByType<MoveableComponent>(enemyEntity,
                                                     Components_e::MOVEABLE_COMPONENT);
    SegmentCollisionComponent *segmentComp = stairwayToComponentManager().
            searchComponentByType<SegmentCollisionComponent>(*enemyConfComp->m_ammo[0],
            Components_e::SEGMENT_COLLISION_COMPONENT);
    GeneralCollisionComponent *genComp = stairwayToComponentManager().
            searchComponentByType<GeneralCollisionComponent>(*enemyConfComp->m_ammo[0],
            Components_e::GENERAL_COLLISION_COMPONENT);
    TimerComponent *timerComp = stairwayToComponentManager().searchComponentByType<TimerComponent>(
                enemyEntity, Components_e::TIMER_COMPONENT);
    assert(moveComp);
    assert(segmentComp);
    assert(genComp);
    assert(timerComp);
    std::chrono::duration<double> elapsed_seconds = std::chrono::system_clock::now() -
            timerComp->m_clockB;
    if(elapsed_seconds.count() > 0.7)
    {
        timerComp->m_clockB = std::chrono::system_clock::now();
        updateEnemyDirection(enemyConfComp, moveComp, enemyMapComp);
    }
    if(enemyConfComp->m_attackPhase != EnemyAttackPhase_e::SHOOT &&
            distancePlayer > LEVEL_TILE_SIZE_PX)
    {
        if(enemyConfComp->m_wallTouch.first)
        {
            float directionToTarget = getTrigoAngle(enemyMapComp->m_absoluteMapPositionPX,
                                            m_playerMapComp->m_absoluteMapPositionPX, false);
            switch(enemyConfComp->m_wallTouch.second)
            {
            case Direction_e::EAST:
            case Direction_e::WEST:
                std::cerr << "EASSS\n";
                if(std::sin(directionToTarget) < EPSILON_FLOAT)
                {
                    moveComp->m_degreeOrientation = 270.0f;
                }
                else
                {
                    moveComp->m_degreeOrientation = 90.0f;
                }
                break;
            case Direction_e::NORTH:
            case Direction_e::SOUTH:
                std::cerr << "NOOOOR\n";
                if(std::cos(directionToTarget) < EPSILON_FLOAT)
                {
                    moveComp->m_degreeOrientation = 180.0f;
                }
                else
                {
                    moveComp->m_degreeOrientation = 0.0f;
                }
                break;
            }
            enemyConfComp->m_wallTouch.first = false;
        }
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
