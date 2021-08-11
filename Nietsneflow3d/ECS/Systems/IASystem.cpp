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
#include <ECS/Components/DoorComponent.hpp>
#include <ECS/Components/MapCoordComponent.hpp>
#include <ECS/Components/RectangleCollisionComponent.hpp>
#include <ECS/Components/WeaponComponent.hpp>
#include <cassert>
#include <random>
#include <iostream>
#include "IASystem.hpp"
#include "FirstPersonDisplaySystem.hpp"
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
    WeaponComponent *weaponComp = stairwayToComponentManager().
            searchComponentByType<WeaponComponent>(m_playerComp->m_weaponEntity,
                                                   Components_e::WEAPON_COMPONENT);
    const WeaponData &weaponData = weaponComp->m_weaponsData[weaponComp->m_currentWeapon];
    assert(weaponComp);
    if(weaponData.m_attackType == AttackType_e::VISIBLE_SHOTS)
    {
        assert(weaponData.m_visibleShootEntities);
        treatVisibleShots(*weaponData.m_visibleShootEntities);
    }
    MapCoordComponent *enemyMapComp;
    EnemyConfComponent *enemyConfComp;
    TimerComponent *timerComp;
    float distancePlayer;
    for(uint32_t i = 0; i < mVectNumEntity.size(); ++i)
    {
        enemyConfComp = stairwayToComponentManager().searchComponentByType<EnemyConfComponent>(
                    mVectNumEntity[i], Components_e::ENEMY_CONF_COMPONENT);
        assert(enemyConfComp);
        if(enemyConfComp->m_visibleShot)
        {
            treatEnemyVisibleShots(enemyConfComp->m_visibleAmmo);
        }
        if(enemyConfComp->m_behaviourMode == EnemyBehaviourMode_e::DEAD ||
                enemyConfComp->m_behaviourMode == EnemyBehaviourMode_e::DYING)
        {
            continue;
        }
        enemyMapComp = stairwayToComponentManager().searchComponentByType<MapCoordComponent>(
                    mVectNumEntity[i], Components_e::MAP_COORD_COMPONENT);
        assert(enemyMapComp);
        distancePlayer = getDistance(m_playerMapComp->m_absoluteMapPositionPX,
                                     enemyMapComp->m_absoluteMapPositionPX);
        float radiantAnglePlayerDirection = getTrigoAngle(enemyMapComp->m_absoluteMapPositionPX,
                                                          m_playerMapComp->m_absoluteMapPositionPX, false);
        if(enemyConfComp->m_behaviourMode != EnemyBehaviourMode_e::ATTACK &&
                checkEnemyTriggerAttackMode(radiantAnglePlayerDirection,
                                            distancePlayer, enemyMapComp))
        {
            timerComp = stairwayToComponentManager().searchComponentByType<TimerComponent>(
                        mVectNumEntity[i], Components_e::TIMER_COMPONENT);
            assert(timerComp);
            timerComp->m_clockB = std::chrono::system_clock::now();
            enemyConfComp->m_behaviourMode = EnemyBehaviourMode_e::ATTACK;
            enemyConfComp->m_countPlayerInvisibility = 0;
        }
        if(enemyConfComp->m_behaviourMode == EnemyBehaviourMode_e::ATTACK)
        {
            treatEnemyBehaviourAttack(mVectNumEntity[i], enemyMapComp, radiantAnglePlayerDirection,
                                      enemyConfComp, distancePlayer);
        }
    }
}

//===================================================================
bool IASystem::checkEnemyTriggerAttackMode(float radiantAngle, float distancePlayer,
                                           MapCoordComponent *enemyMapComp)
{
    if(distancePlayer > m_distanceEnemyBehaviour)
    {
        return false;
    }
    optionalTargetRaycast_t result = mptrSystemManager->searchSystemByType<FirstPersonDisplaySystem>(
                static_cast<uint32_t>(Systems_e::FIRST_PERSON_DISPLAY_SYSTEM))->
            calcLineSegmentRaycast(radiantAngle, enemyMapComp->m_absoluteMapPositionPX, false);
    return (getDistance(enemyMapComp->m_absoluteMapPositionPX, std::get<0>(*result)) > distancePlayer);
}

//===================================================================
void IASystem::treatVisibleShots(const std::array<uint32_t, TOTAL_SHOT_NUMBER> &stdAmmo)
{
    for(uint32_t i = 0; i < stdAmmo.size(); ++i)
    {
        treatVisibleShot(stdAmmo[i]);
    }
}

//===================================================================
void IASystem::treatEnemyVisibleShots(const ArrayWeaponVisibleShot_t &stdAmmo)
{
    for(uint32_t i = 0; i < stdAmmo.size(); ++i)
    {
        treatVisibleShot(stdAmmo[i]);
    }
}

//===================================================================
void IASystem::treatVisibleShot(uint32_t numEntity)
{
    GeneralCollisionComponent *genColl = stairwayToComponentManager().
            searchComponentByType<GeneralCollisionComponent>(numEntity,
                                                             Components_e::GENERAL_COLLISION_COMPONENT);
    assert(genColl);
    if(!genColl->m_active)
    {
        return;
    }
    ShotConfComponent *shotComp = stairwayToComponentManager().
            searchComponentByType<ShotConfComponent>(numEntity,
                                                     Components_e::SHOT_CONF_COMPONENT);
    assert(shotComp);
    if(shotComp->m_destructPhase)
    {
        return;
    }
    TimerComponent *timerComp = stairwayToComponentManager().
            searchComponentByType<TimerComponent>(numEntity, Components_e::TIMER_COMPONENT);
    assert(timerComp);
    std::chrono::duration<double> elapsed_seconds = std::chrono::system_clock::now() -
            timerComp->m_clockA;
    if(elapsed_seconds.count() > 5.0)
    {
        genColl->m_active = false;
        return;
    }
    MapCoordComponent *ammoMapComp = stairwayToComponentManager().
            searchComponentByType<MapCoordComponent>(numEntity,
                                                     Components_e::MAP_COORD_COMPONENT);
    MoveableComponent *ammoMoveComp = stairwayToComponentManager().
            searchComponentByType<MoveableComponent>(numEntity,
                                                     Components_e::MOVEABLE_COMPONENT);
    assert(ammoMapComp);
    assert(ammoMoveComp);
    assert(genColl->m_shape == CollisionShape_e::CIRCLE_C);
    moveElementFromAngle((*ammoMoveComp).m_velocity,
                         getRadiantAngle(ammoMoveComp->m_degreeOrientation),
                         ammoMapComp->m_absoluteMapPositionPX);
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
                          MapCoordComponent *enemyMapComp)
{
    GeneralCollisionComponent *genComp;
    if(enemyConfComp->m_visibleShot)
    {
        confVisibleShoot(enemyConfComp->m_visibleAmmo, enemyMapComp->m_absoluteMapPositionPX, moveComp->m_degreeOrientation);
    }
    else
    {
        genComp = stairwayToComponentManager().
                searchComponentByType<GeneralCollisionComponent>(
                    enemyConfComp->m_stdAmmo[0], Components_e::GENERAL_COLLISION_COMPONENT);
        SegmentCollisionComponent *segmentComp = stairwayToComponentManager().
                searchComponentByType<SegmentCollisionComponent>(
                    enemyConfComp->m_stdAmmo[0], Components_e::SEGMENT_COLLISION_COMPONENT);
        assert(genComp);
        assert(segmentComp);
        confBullet(genComp, segmentComp, CollisionTag_e::BULLET_ENEMY_CT,
                   enemyMapComp->m_absoluteMapPositionPX, moveComp->m_degreeOrientation);
    }
}

//===================================================================
void IASystem::treatEnemyBehaviourAttack(uint32_t enemyEntity, MapCoordComponent *enemyMapComp,
                                         float radiantAnglePlayerDirection,
                                         EnemyConfComponent *enemyConfComp, float distancePlayer)
{
    MoveableComponent *moveComp = stairwayToComponentManager().
            searchComponentByType<MoveableComponent>(enemyEntity,
                                                     Components_e::MOVEABLE_COMPONENT);
    TimerComponent *timerComp = stairwayToComponentManager().searchComponentByType<TimerComponent>(
                enemyEntity, Components_e::TIMER_COMPONENT);
    assert(moveComp);
    assert(timerComp);
    std::chrono::duration<double> elapsed_seconds = std::chrono::system_clock::now() -
            timerComp->m_clockB;
    if(elapsed_seconds.count() > 0.4)
    {
        enemyConfComp->m_prevWall = false;
        enemyConfComp->m_attackPhase =
                static_cast<EnemyAttackPhase_e>(std::rand() / ((RAND_MAX + 1u) / 4));
        timerComp->m_clockB = std::chrono::system_clock::now();
        updateEnemyDirection(enemyConfComp, moveComp, enemyMapComp);
        if(enemyConfComp->m_attackPhase == EnemyAttackPhase_e::SHOOT)
        {
            enemyShoot(enemyConfComp, moveComp, enemyMapComp);
        }
        if(!checkEnemyTriggerAttackMode(radiantAnglePlayerDirection, distancePlayer, enemyMapComp))
        {
            if(++enemyConfComp->m_countPlayerInvisibility > 5)
            {
               enemyConfComp->m_behaviourMode = EnemyBehaviourMode_e::PASSIVE;
            }
        }
    }
    else if(enemyConfComp->m_attackPhase != EnemyAttackPhase_e::SHOOT &&
            enemyConfComp->m_attackPhase != EnemyAttackPhase_e::SHOOTED &&
            distancePlayer > LEVEL_TILE_SIZE_PX)
    {
        moveElementFromAngle(moveComp->m_velocity, getRadiantAngle(moveComp->m_degreeOrientation),
                             enemyMapComp->m_absoluteMapPositionPX);
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
void IASystem::confVisibleShoot(const ArrayWeaponVisibleShot_t &visibleShots,
                                const pairFloat_t &point, float degreeAngle)
{
    uint32_t currentShot = 0;
    for(; currentShot < visibleShots.size(); ++currentShot)
    {
        GeneralCollisionComponent *genComp = stairwayToComponentManager().searchComponentByType<GeneralCollisionComponent>(
                    visibleShots[currentShot], Components_e::GENERAL_COLLISION_COMPONENT);
        assert(genComp);
        if(!genComp->m_active)
        {
            genComp->m_active = true;
            TimerComponent *timerComp = stairwayToComponentManager().
                    searchComponentByType<TimerComponent>(
                        visibleShots[currentShot], Components_e::TIMER_COMPONENT);
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
                visibleShots[currentShot], Components_e::MAP_COORD_COMPONENT);
    MoveableComponent *ammoMoveComp = stairwayToComponentManager().
            searchComponentByType<MoveableComponent>(
                visibleShots[currentShot], Components_e::MOVEABLE_COMPONENT);
    TimerComponent *ammoTimeComp = stairwayToComponentManager().
            searchComponentByType<TimerComponent>(
                visibleShots[currentShot], Components_e::TIMER_COMPONENT);
    assert(ammoTimeComp);
    assert(mapComp);
    assert(ammoMoveComp);
    mapComp->m_absoluteMapPositionPX = point;
    ammoMoveComp->m_degreeOrientation = degreeAngle;
}
