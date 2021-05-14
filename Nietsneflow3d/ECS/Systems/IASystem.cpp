#include <ECS/Components/EnemyConfComponent.hpp>
#include <ECS/Components/MapCoordComponent.hpp>
#include <ECS/Components/EnemyConfComponent.hpp>
#include <ECS/Components/MoveableComponent.hpp>
#include <ECS/Components/SegmentCollisionComponent.hpp>
#include <ECS/Components/GeneralCollisionComponent.hpp>
#include <ECS/Components/TimerComponent.hpp>
#include <ECS/Components/CircleCollisionComponent.hpp>
#include <ECS/Components/PlayerConfComponent.hpp>
#include <ECS/Components/ShotConfComponent.hpp>
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
    assert(m_playerComp);
    System::execSystem();
    treatVisibleShot(m_playerComp->m_visibleShootEntities);
    MapCoordComponent *enemyMapComp;
    EnemyConfComponent *enemyConfComp;
    TimerComponent *timerComp;
    float distancePlayer;
    for(uint32_t i = 0; i < mVectNumEntity.size(); ++i)
    {
        enemyConfComp = stairwayToComponentManager().searchComponentByType<EnemyConfComponent>(
                    mVectNumEntity[i], Components_e::ENEMY_CONF_COMPONENT);
        assert(enemyConfComp);
        treatVisibleShot(enemyConfComp->m_visibleAmmo);
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
void IASystem::treatVisibleShot(const ammoContainer_t &stdAmmo)
{
    for(uint32_t i = 0; i < stdAmmo.size(); ++i)
    {
        GeneralCollisionComponent *genColl = stairwayToComponentManager().
                searchComponentByType<GeneralCollisionComponent>(*stdAmmo[i],
                                                                 Components_e::GENERAL_COLLISION_COMPONENT);
        assert(genColl);
        if(!genColl->m_active)
        {
            continue;
        }
        ShotConfComponent *shotComp = stairwayToComponentManager().
                searchComponentByType<ShotConfComponent>(*stdAmmo[i],
                                                         Components_e::SHOT_CONF_COMPONENT);
        assert(shotComp);
        if(shotComp->m_destructPhase)
        {
            continue;
        }
        TimerComponent *timerComp = stairwayToComponentManager().
                searchComponentByType<TimerComponent>(*stdAmmo[i], Components_e::TIMER_COMPONENT);
        assert(timerComp);
        std::chrono::duration<double> elapsed_seconds = std::chrono::system_clock::now() -
                timerComp->m_clockA;
        if(elapsed_seconds.count() > 5.0)
        {
            genColl->m_active = false;
            return;
        }
        MapCoordComponent *ammoMapComp = stairwayToComponentManager().
                searchComponentByType<MapCoordComponent>(*stdAmmo[i],
                                                         Components_e::MAP_COORD_COMPONENT);
        MoveableComponent *ammoMoveComp = stairwayToComponentManager().
                searchComponentByType<MoveableComponent>(*stdAmmo[i],
                                                         Components_e::MOVEABLE_COMPONENT);
        assert(ammoMapComp);
        assert(ammoMoveComp);
        moveElement(*ammoMoveComp, *ammoMapComp, MoveOrientation_e::FORWARD);
    }
}

//===================================================================
void IASystem::updateEnemyDirection(EnemyConfComponent *enemyConfComp, MoveableComponent *moveComp,
                                    MapCoordComponent *enemyMapComp)
{
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
void IASystem::enemyShoot(EnemyConfComponent *enemyConfComp, MoveableComponent *moveComp,
                          MapCoordComponent *enemyMapComp, bool visibleShot)
{
    GeneralCollisionComponent *genComp;
    if(visibleShot)
    {
        confVisibleShoot(enemyConfComp->m_visibleAmmo, enemyMapComp->m_absoluteMapPositionPX,
                         moveComp->m_degreeOrientation);
    }
    else
    {
        genComp = stairwayToComponentManager().
                searchComponentByType<GeneralCollisionComponent>(
                    *enemyConfComp->m_stdAmmo[0], Components_e::GENERAL_COLLISION_COMPONENT);
        SegmentCollisionComponent *segmentComp = stairwayToComponentManager().
                searchComponentByType<SegmentCollisionComponent>(
                    *enemyConfComp->m_stdAmmo[0], Components_e::SEGMENT_COLLISION_COMPONENT);
        assert(genComp);
        assert(segmentComp);
        confBullet(genComp, segmentComp, CollisionTag_e::BULLET_ENEMY_CT,
                   enemyMapComp->m_absoluteMapPositionPX, moveComp->m_degreeOrientation);
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
            searchComponentByType<SegmentCollisionComponent>(*enemyConfComp->m_stdAmmo[0],
            Components_e::SEGMENT_COLLISION_COMPONENT);
    GeneralCollisionComponent *genComp = stairwayToComponentManager().
            searchComponentByType<GeneralCollisionComponent>(*enemyConfComp->m_stdAmmo[0],
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
        enemyConfComp->m_attackPhase =
                static_cast<EnemyAttackPhase_e>(std::rand() / ((RAND_MAX + 1u) / 4));

        timerComp->m_clockB = std::chrono::system_clock::now();
        updateEnemyDirection(enemyConfComp, moveComp, enemyMapComp);
        if(enemyConfComp->m_attackPhase == EnemyAttackPhase_e::SHOOT)
        {
            enemyShoot(enemyConfComp, moveComp, enemyMapComp, true);
        }
    }
    else if(enemyConfComp->m_attackPhase != EnemyAttackPhase_e::SHOOT &&
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
    m_playerComp = stairwayToComponentManager().
            searchComponentByType<PlayerConfComponent>(m_playerEntity,
                                                       Components_e::PLAYER_CONF_COMPONENT);
    assert(m_playerMapComp);
    assert(m_playerComp);
}

//===================================================================
void IASystem::confVisibleShoot(const ammoContainer_t &visibleShots,
                                const pairFloat_t &point, float degreeAngle)
{
    uint32_t currentShot = 0;
    for(; currentShot < visibleShots.size(); ++currentShot)
    {
        GeneralCollisionComponent *genComp = stairwayToComponentManager().searchComponentByType<GeneralCollisionComponent>(
                    *visibleShots[currentShot], Components_e::GENERAL_COLLISION_COMPONENT);
        assert(genComp);
        if(!genComp->m_active)
        {
            genComp->m_active = true;
            TimerComponent *timerComp = stairwayToComponentManager().
                    searchComponentByType<TimerComponent>(
                        *visibleShots[currentShot], Components_e::TIMER_COMPONENT);
            assert(timerComp);
            timerComp->m_clockA = std::chrono::system_clock::now();
            break;
        }
        //if all shoot active
        if(currentShot == (visibleShots.size() - 1))
        {
            return;
        }
    }
    MapCoordComponent *mapComp = stairwayToComponentManager().
            searchComponentByType<MapCoordComponent>(
                *visibleShots[currentShot], Components_e::MAP_COORD_COMPONENT);
    MoveableComponent *ammoMoveComp = stairwayToComponentManager().
            searchComponentByType<MoveableComponent>(
                *visibleShots[currentShot], Components_e::MOVEABLE_COMPONENT);
    TimerComponent *ammoTimeComp = stairwayToComponentManager().
            searchComponentByType<TimerComponent>(
                *visibleShots[currentShot], Components_e::TIMER_COMPONENT);
    assert(ammoTimeComp);
    assert(mapComp);
    assert(ammoMoveComp);
    mapComp->m_absoluteMapPositionPX = point;
    ammoMoveComp->m_degreeOrientation = degreeAngle;
}