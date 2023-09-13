#include <ECS/Components/EnemyConfComponent.hpp>
#include <ECS/Components/ImpactShotComponent.hpp>
#include <ECS/Components/MapCoordComponent.hpp>
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
#include <ECS/Components/MemSpriteDataComponent.hpp>
#include <ECS/Components/MemFPSGLSizeComponent.hpp>
#include <ECS/Components/MoveableComponent.hpp>
#include <ECS/Components/SpriteTextureComponent.hpp>
#include <ECS/Components/FPSVisibleStaticElementComponent.hpp>
#include <ECS/Components/EnemyConfComponent.hpp>
#include <ECS/ECSManager.hpp>
#include <cassert>
#include <random>
#include <iostream>
#include "IASystem.hpp"
#include "FirstPersonDisplaySystem.hpp"
#include "PhysicalEngine.hpp"
#include "CollisionUtils.hpp"
#include "MainEngine.hpp"

//===================================================================
IASystem::IASystem(NewComponentManager &newComponentManager, ECSManager* memECSManager) :
    m_newComponentManager(newComponentManager),
    m_componentsContainer(m_newComponentManager.getComponentsContainer()),
    m_memECSManager(memECSManager)
{
    std::srand(std::time(nullptr));
    bAddComponentToSystem(Components_e::ENEMY_CONF_COMPONENT);
}

//===================================================================
void IASystem::treatEject()
{
    std::bitset<TOTAL_COMPONENTS> bitset;
    bitset[MOVEABLE_COMPONENT] = true;
    m_vectMoveableEntities = m_memECSManager->getEntitiesContainingComponents(bitset);
    OptUint_t numCom;
    for(uint32_t i = 0; i < m_vectMoveableEntities.size(); ++i)
    {
        numCom = m_newComponentManager.getComponentEmplacement(m_vectMoveableEntities[i], Components_e::MOVEABLE_COMPONENT);
        assert(numCom);
        MoveableComponent &moveComp = m_componentsContainer.m_vectMoveableComp[*numCom];
        if(moveComp.m_ejectData)
        {
            numCom = m_newComponentManager.getComponentEmplacement(m_vectMoveableEntities[i], Components_e::TIMER_COMPONENT);
            assert(numCom);
            TimerComponent &timerComp = m_componentsContainer.m_vectTimerComp[*numCom];
            if(++timerComp.m_cycleCountD >= moveComp.m_ejectData->second)
            {
                moveComp.m_ejectData = std::nullopt;
                return;
            }
            numCom = m_newComponentManager.getComponentEmplacement(m_vectMoveableEntities[i], Components_e::MAP_COORD_COMPONENT);
            assert(numCom);
            MapCoordComponent &mapComp = m_componentsContainer.m_vectMapCoordComp[*numCom];
            moveElementFromAngle(moveComp.m_ejectData->first, getRadiantAngle(moveComp.m_currentDegreeMoveDirection),
                                 mapComp.m_absoluteMapPositionPX);
        }
    }
}

//===================================================================
void IASystem::execSystem()
{
    System::execSystem();
    OptUint_t compNum = m_newComponentManager.getComponentEmplacement(m_playerEntity, Components_e::PLAYER_CONF_COMPONENT);
    assert(compNum);
    PlayerConfComponent &playerConfComp = m_componentsContainer.m_vectPlayerConfComp[*compNum];
    compNum = m_newComponentManager.getComponentEmplacement(
        playerConfComp.m_vectEntities[static_cast<uint32_t>(PlayerEntities_e::WEAPON)], Components_e::WEAPON_COMPONENT);
    assert(compNum);
    WeaponComponent &weaponComp = m_componentsContainer.m_vectWeaponComp[*compNum];
    treatEject();
    for(uint32_t i = 0; i < weaponComp.m_weaponsData.size(); ++i)
    {
        if(weaponComp.m_weaponsData[i].m_attackType == AttackType_e::VISIBLE_SHOTS)
        {
            assert(weaponComp.m_weaponsData[i].m_visibleShootEntities);
            treatVisibleShots(*weaponComp.m_weaponsData[i].m_visibleShootEntities);
        }
    }
    float distancePlayer;
    OptUint_t numCom;
    for(uint32_t i = 0; i < mVectNumEntity.size(); ++i)
    {
        compNum = m_newComponentManager.getComponentEmplacement(mVectNumEntity[i], Components_e::ENEMY_CONF_COMPONENT);
        assert(compNum);
        EnemyConfComponent &enemyConfComp = m_componentsContainer.m_vectEnemyConfComp[*compNum];
        if(enemyConfComp.m_visibleShot)
        {
            treatVisibleShots(enemyConfComp.m_visibleAmmo);
        }
        if(enemyConfComp.m_behaviourMode == EnemyBehaviourMode_e::DEAD ||
                enemyConfComp.m_behaviourMode == EnemyBehaviourMode_e::DYING)
        {
            if(enemyConfComp.m_playDeathSound)
            {
                activeSound(mVectNumEntity[i], static_cast<uint32_t>(EnemySoundEffect_e::DEATH));
                enemyConfComp.m_playDeathSound = false;
            }
            continue;
        }
        numCom = m_newComponentManager.getComponentEmplacement(m_playerEntity, Components_e::MAP_COORD_COMPONENT);
        assert(numCom);
        MapCoordComponent &playerMapComp = m_componentsContainer.m_vectMapCoordComp[*numCom];
        numCom = m_newComponentManager.getComponentEmplacement(mVectNumEntity[i], Components_e::MAP_COORD_COMPONENT);
        assert(numCom);
        MapCoordComponent &enemyMapComp = m_componentsContainer.m_vectMapCoordComp[*numCom];
        distancePlayer = getDistance(playerMapComp.m_absoluteMapPositionPX,
                                     enemyMapComp.m_absoluteMapPositionPX);
        float radiantAnglePlayerDirection = getTrigoAngle(enemyMapComp.m_absoluteMapPositionPX,
                                                          playerMapComp.m_absoluteMapPositionPX, false);
        if(enemyConfComp.m_behaviourMode != EnemyBehaviourMode_e::ATTACK)
        {
            numCom = m_newComponentManager.getComponentEmplacement(mVectNumEntity[i], Components_e::TIMER_COMPONENT);
            assert(numCom);
            TimerComponent &timerComp = m_componentsContainer.m_vectTimerComp[*numCom];
            if(checkEnemyTriggerAttackMode(radiantAnglePlayerDirection, distancePlayer, enemyMapComp))
            {
                timerComp.m_cycleCountB = 0;
                enemyConfComp.m_behaviourMode = EnemyBehaviourMode_e::ATTACK;
                activeSound(mVectNumEntity[i], static_cast<uint32_t>(EnemySoundEffect_e::DETECT));
                enemyConfComp.m_countPlayerInvisibility = 0;
            }
        }
        if(enemyConfComp.m_behaviourMode == EnemyBehaviourMode_e::ATTACK)
        {
            treatEnemyBehaviourAttack(mVectNumEntity[i], enemyMapComp, radiantAnglePlayerDirection, enemyConfComp, distancePlayer);
        }
    }
}

//===================================================================
bool IASystem::checkEnemyTriggerAttackMode(float radiantAngle, float distancePlayer,
                                           MapCoordComponent &enemyMapComp)
{
    if(distancePlayer > m_distanceEnemyBehaviour)
    {
        return false;
    }
    optionalTargetRaycast_t result = mptrSystemManager->searchSystemByType<FirstPersonDisplaySystem>(
                static_cast<uint32_t>(Systems_e::FIRST_PERSON_DISPLAY_SYSTEM))->
            calcLineSegmentRaycast(radiantAngle, enemyMapComp.m_absoluteMapPositionPX, false);
    return (getDistance(enemyMapComp.m_absoluteMapPositionPX, std::get<0>(*result)) > distancePlayer);
}

//===================================================================
void IASystem::treatVisibleShots(const std::vector<uint32_t> &stdAmmo)
{
    for(uint32_t i = 0; i < stdAmmo.size(); ++i)
    {
        treatVisibleShot(stdAmmo[i]);
    }
}

//===================================================================
void IASystem::treatVisibleShot(uint32_t numEntity)
{
    OptUint_t numCom = m_newComponentManager.getComponentEmplacement(numEntity, Components_e::GENERAL_COLLISION_COMPONENT);
    assert(numCom);
    GeneralCollisionComponent &genColl = m_componentsContainer.m_vectGeneralCollisionComp[*numCom];
    if(!genColl.m_active)
    {
        return;
    }
    numCom = m_newComponentManager.getComponentEmplacement(numEntity, Components_e::SHOT_CONF_COMPONENT);
    assert(numCom);
    ShotConfComponent &shotComp = m_componentsContainer.m_vectShotConfComp[*numCom];
    if(shotComp.m_destructPhase)
    {
        return;
    }
    numCom = m_newComponentManager.getComponentEmplacement(numEntity, Components_e::TIMER_COMPONENT);
    assert(numCom);
    TimerComponent &timerComp = m_componentsContainer.m_vectTimerComp[*numCom];
    if(++timerComp.m_cycleCountA > m_intervalVisibleShotLifeTime)
    {
        genColl.m_active = false;
        timerComp.m_cycleCountA = 0;
        return;
    }
    numCom = m_newComponentManager.getComponentEmplacement(numEntity, Components_e::MAP_COORD_COMPONENT);
    assert(numCom);
    MapCoordComponent &ammoMapComp = m_componentsContainer.m_vectMapCoordComp[*numCom];
    numCom = m_newComponentManager.getComponentEmplacement(numEntity, Components_e::MOVEABLE_COMPONENT);
    assert(numCom);
    MoveableComponent &ammoMoveComp = m_componentsContainer.m_vectMoveableComp[*numCom];
    assert(genColl.m_shape == CollisionShape_e::CIRCLE_C);
    moveElementFromAngle(ammoMoveComp.m_velocity, getRadiantAngle(ammoMoveComp.m_degreeOrientation),
                         ammoMapComp.m_absoluteMapPositionPX);
}

//===================================================================
void IASystem::activeSound(uint32_t entityNum, uint32_t soundNum)
{
    OptUint_t numCom = m_newComponentManager.getComponentEmplacement(entityNum, Components_e::AUDIO_COMPONENT);
    assert(numCom);
    AudioComponent &audioComp = m_componentsContainer.m_vectAudioComp[*numCom];
    audioComp.m_soundElements[soundNum]->m_toPlay = true;
}

//===================================================================
void IASystem::updateEnemyDirection(EnemyConfComponent &enemyConfComp, MoveableComponent &moveComp,
                                    MapCoordComponent &enemyMapComp)
{
    OptUint_t numCom = m_newComponentManager.getComponentEmplacement(m_playerEntity, Components_e::MAP_COORD_COMPONENT);
    assert(numCom);
    MapCoordComponent &playerMapComp = m_componentsContainer.m_vectMapCoordComp[*numCom];
    moveComp.m_degreeOrientation = getTrigoAngle(enemyMapComp.m_absoluteMapPositionPX, playerMapComp.m_absoluteMapPositionPX);
    if(enemyConfComp.m_attackPhase == EnemyAttackPhase_e::MOVE_TO_TARGET_DIAG_RIGHT)
    {
        moveComp.m_degreeOrientation -= 30.0f;
    }
    else if(enemyConfComp.m_attackPhase == EnemyAttackPhase_e::MOVE_TO_TARGET_DIAG_LEFT)
    {
        moveComp.m_degreeOrientation += 30.0f;
    }
    else if(enemyConfComp.m_attackPhase == EnemyAttackPhase_e::MOVE_TO_TARGET_LEFT)
    {
        moveComp.m_degreeOrientation += 90.0f;
    }
    else if(enemyConfComp.m_attackPhase == EnemyAttackPhase_e::MOVE_TO_TARGET_RIGHT)
    {
        moveComp.m_degreeOrientation -= 90.0f;
    }
    moveComp.m_currentDegreeMoveDirection = moveComp.m_degreeOrientation;
}

//===================================================================
void IASystem::enemyShoot(EnemyConfComponent &enemyConfComp, MoveableComponent &moveComp,
                          MapCoordComponent &enemyMapComp, float distancePlayer)
{
    OptUint_t compNum;
    if(enemyConfComp.m_meleeAttackDamage && distancePlayer < 32.0f)
    {
        compNum = m_newComponentManager.getComponentEmplacement(m_playerEntity, Components_e::PLAYER_CONF_COMPONENT);
        assert(compNum);
        PlayerConfComponent &playerConfComp = m_componentsContainer.m_vectPlayerConfComp[*compNum];
        playerConfComp.takeDamage(*enemyConfComp.m_meleeAttackDamage);
    }
    else if(enemyConfComp.m_visibleShot)
    {
        confVisibleShoot(enemyConfComp.m_visibleAmmo, enemyMapComp.m_absoluteMapPositionPX,
                         moveComp.m_degreeOrientation, CollisionTag_e::BULLET_ENEMY_CT);
    }
    else
    {
        assert(!enemyConfComp.m_stdAmmo.empty());
        for(uint32_t i = 0; i < enemyConfComp.m_simultaneousShot; ++i)
        {
            compNum = m_newComponentManager.getComponentEmplacement(enemyConfComp.m_stdAmmo[i],
                    Components_e::GENERAL_COLLISION_COMPONENT);
            assert(compNum);
            GeneralCollisionComponent &genComp = m_componentsContainer.m_vectGeneralCollisionComp[*compNum];

            compNum = m_newComponentManager.getComponentEmplacement(enemyConfComp.m_stdAmmo[i], Components_e::SEGMENT_COLLISION_COMPONENT);
            assert(compNum);
            SegmentCollisionComponent &segmentComp = m_componentsContainer.m_vectSegmentCollisionComp[*compNum];
            compNum = m_newComponentManager.getComponentEmplacement(enemyConfComp.m_stdAmmo[i], Components_e::SHOT_CONF_COMPONENT);
            assert(compNum);
            ShotConfComponent &shotComp = m_componentsContainer.m_vectShotConfComp[*compNum];
            compNum = m_newComponentManager.getComponentEmplacement(shotComp.m_impactEntity, Components_e::IMPACT_CONF_COMPONENT);
            assert(compNum);
            ImpactShotComponent &impactComp = m_componentsContainer.m_vectImpactShotComp[*compNum];
            compNum = m_newComponentManager.getComponentEmplacement(shotComp.m_impactEntity, Components_e::MOVEABLE_COMPONENT);
            assert(compNum);
            segmentComp.m_impactEntity = shotComp.m_impactEntity;
            MoveableComponent &impactMoveComp = m_componentsContainer.m_vectMoveableComp[*compNum];
            confBullet(impactComp, genComp, segmentComp, impactMoveComp, CollisionTag_e::BULLET_ENEMY_CT,
                       enemyMapComp.m_absoluteMapPositionPX, moveComp.m_degreeOrientation);
            compNum = m_newComponentManager.getComponentEmplacement(shotComp.m_impactEntity,
                                                                   Components_e::MAP_COORD_COMPONENT);
            assert(compNum);
            MapCoordComponent &mapComp = m_componentsContainer.m_vectMapCoordComp[*compNum];
            std::optional<PairUI_t> coord = getLevelCoord(mapComp.m_absoluteMapPositionPX);
            assert(coord);
            m_mainEngine->addEntityToZone(shotComp.m_impactEntity, *coord);
        }
    }
}

//===================================================================
void IASystem::treatEnemyBehaviourAttack(uint32_t enemyEntity, MapCoordComponent &enemyMapComp,
                                         float radiantAnglePlayerDirection, EnemyConfComponent &enemyConfComp, float distancePlayer)
{
    OptUint_t numCom = m_newComponentManager.getComponentEmplacement(enemyEntity, Components_e::MOVEABLE_COMPONENT);
    assert(numCom);
    MoveableComponent &moveComp = m_componentsContainer.m_vectMoveableComp[*numCom];

    numCom = m_newComponentManager.getComponentEmplacement(enemyEntity, Components_e::TIMER_COMPONENT);
    assert(numCom);
    TimerComponent &timerComp = m_componentsContainer.m_vectTimerComp[*numCom];
    if(!enemyConfComp.m_stuck)
    {
        enemyConfComp.m_previousMove = {EnemyAttackPhase_e::TOTAL, EnemyAttackPhase_e::TOTAL};
    }
    if(enemyConfComp.m_stuck || ++timerComp.m_cycleCountB >= m_intervalEnemyBehaviour)
    {
        if(enemyConfComp.m_countTillLastAttack > 3 && (!enemyConfComp.m_meleeOnly || distancePlayer < 32.0f))
        {
            enemyConfComp.m_attackPhase = EnemyAttackPhase_e::SHOOT;
            enemyConfComp.m_stuck = false;
        }
        else
        {
            uint32_t modulo = (enemyConfComp.m_meleeOnly || enemyConfComp.m_countTillLastAttack < 2) ? static_cast<uint32_t>(EnemyAttackPhase_e::SHOOT) :
                                                                           static_cast<uint32_t>(EnemyAttackPhase_e::SHOOT) + 1;
            enemyConfComp.m_attackPhase = static_cast<EnemyAttackPhase_e>(std::rand() / ((RAND_MAX + 1u) / modulo));
        }
        enemyConfComp.m_countTillLastAttack =
                (enemyConfComp.m_attackPhase == EnemyAttackPhase_e::SHOOT) ? 0 : ++enemyConfComp.m_countTillLastAttack;

        while(enemyConfComp.m_stuck)
        {
            if((enemyConfComp.m_attackPhase != std::get<0>(enemyConfComp.m_previousMove) &&
                enemyConfComp.m_attackPhase != std::get<1>(enemyConfComp.m_previousMove)))
            {
                enemyConfComp.m_stuck = false;
            }
            else
            {
                if(enemyConfComp.m_attackPhase == EnemyAttackPhase_e::MOVE_TO_TARGET_FRONT)
                {
                    enemyConfComp.m_attackPhase = EnemyAttackPhase_e::MOVE_TO_TARGET_DIAG_LEFT;
                }
                else
                {
                    enemyConfComp.m_attackPhase = static_cast<EnemyAttackPhase_e>(static_cast<uint32_t>(enemyConfComp.m_attackPhase) - 1);
                }
            }
        }
        std::swap(std::get<0>(enemyConfComp.m_previousMove), std::get<1>(enemyConfComp.m_previousMove));
        std::get<0>(enemyConfComp.m_previousMove) = enemyConfComp.m_attackPhase;

        std::swap(enemyConfComp.m_previousMove[2], enemyConfComp.m_previousMove[1]);
        std::swap(enemyConfComp.m_previousMove[1], enemyConfComp.m_previousMove[0]);
        enemyConfComp.m_previousMove[0] = enemyConfComp.m_attackPhase;
        timerComp.m_cycleCountB = 0;
        updateEnemyDirection(enemyConfComp, moveComp, enemyMapComp);
        if(enemyConfComp.m_attackPhase == EnemyAttackPhase_e::SHOOT)
        {
            enemyShoot(enemyConfComp, moveComp, enemyMapComp, distancePlayer);
            activeSound(enemyEntity, static_cast<uint32_t>(EnemySoundEffect_e::ATTACK));
        }
        if(!checkEnemyTriggerAttackMode(radiantAnglePlayerDirection, distancePlayer, enemyMapComp))
        {
            if(++enemyConfComp.m_countPlayerInvisibility > 5)
            {
                enemyConfComp.m_behaviourMode = EnemyBehaviourMode_e::PASSIVE;
                timerComp.m_cycleCountC = 0;
            }
        }
    }
    else if(enemyConfComp.m_attackPhase != EnemyAttackPhase_e::SHOOT && distancePlayer > LEVEL_TILE_SIZE_PX)
    {
        if(enemyConfComp.m_attackPhase != EnemyAttackPhase_e::SHOOTED)
        {
            moveElementFromAngle(moveComp.m_velocity, getRadiantAngle(moveComp.m_degreeOrientation),
                                 enemyMapComp.m_absoluteMapPositionPX);
            numCom = m_newComponentManager.getComponentEmplacement(enemyEntity, Components_e::MAP_COORD_COMPONENT);
            assert(numCom);
            MapCoordComponent &mapComp = m_componentsContainer.m_vectMapCoordComp[*numCom];
            m_mainEngine->addEntityToZone(enemyEntity, *getLevelCoord(mapComp.m_absoluteMapPositionPX));
        }
    }
}

//===================================================================
void IASystem::memPlayerDatas(uint32_t playerEntity)
{
    m_playerEntity = playerEntity;
}

//===================================================================
void IASystem::confVisibleShoot(std::vector<uint32_t> &visibleShots, const PairFloat_t &point, float degreeAngle, CollisionTag_e tag)
{
    uint32_t currentShot = 0;
    assert(!visibleShots.empty());
    OptUint_t numCom;
    numCom = m_newComponentManager.getComponentEmplacement(visibleShots[currentShot],
                                                           Components_e::GENERAL_COLLISION_COMPONENT);
    assert(numCom);
    GeneralCollisionComponent *genComp = &m_componentsContainer.m_vectGeneralCollisionComp[*numCom];
    for(; currentShot < visibleShots.size(); ++currentShot)
    {
        if(!genComp->m_active)
        {
            break;
        }
        //if all shoot active create a new one
        else if(currentShot == (visibleShots.size() - 1))
        {
            visibleShots.push_back(m_mainEngine->createAmmoEntity(tag, true));
            confNewVisibleShot(visibleShots);
            ++currentShot;
            numCom = m_newComponentManager.getComponentEmplacement(visibleShots[currentShot],
                                                                   Components_e::GENERAL_COLLISION_COMPONENT);
            assert(numCom);
            genComp = &m_componentsContainer.m_vectGeneralCollisionComp[*numCom];
            break;
        }
    }
    numCom = m_newComponentManager.getComponentEmplacement(visibleShots[currentShot], Components_e::SHOT_CONF_COMPONENT);
    assert(numCom);
    ShotConfComponent &targetShotConfComp = m_componentsContainer.m_vectShotConfComp[*numCom];
    if(targetShotConfComp.m_ejectMode)
    {
        targetShotConfComp.m_ejectMode = false;
        numCom = m_newComponentManager.getComponentEmplacement(visibleShots[currentShot], Components_e::CIRCLE_COLLISION_COMPONENT);
        assert(numCom);
        CircleCollisionComponent &circleTargetComp = m_componentsContainer.m_vectCircleCollisionComp[*numCom];
        std::swap(targetShotConfComp.m_ejectExplosionRay, circleTargetComp.m_ray);
    }
    numCom = m_newComponentManager.getComponentEmplacement(visibleShots[currentShot], Components_e::MAP_COORD_COMPONENT);
    assert(numCom);
    MapCoordComponent &mapComp = m_componentsContainer.m_vectMapCoordComp[*numCom];
    numCom = m_newComponentManager.getComponentEmplacement(visibleShots[currentShot], Components_e::MOVEABLE_COMPONENT);
    assert(numCom);
    MoveableComponent &ammoMoveComp = m_componentsContainer.m_vectMoveableComp[*numCom];
    numCom = m_newComponentManager.getComponentEmplacement(visibleShots[currentShot], Components_e::TIMER_COMPONENT);
    assert(numCom);
    TimerComponent &ammoTimeComp = m_componentsContainer.m_vectTimerComp[*numCom];
    genComp->m_active = true;
    ammoTimeComp.m_cycleCountA = 0;
    std::optional<PairUI_t> coord = getLevelCoord(point);
    assert(coord);
    mapComp.m_coord = *coord;
    mapComp.m_absoluteMapPositionPX = point;
    m_mainEngine->addEntityToZone(visibleShots[currentShot], mapComp.m_coord);
    moveElementFromAngle(LEVEL_HALF_TILE_SIZE_PX, getRadiantAngle(degreeAngle),
                         mapComp.m_absoluteMapPositionPX);
    ammoMoveComp.m_degreeOrientation = degreeAngle;
    ammoMoveComp.m_currentDegreeMoveDirection = degreeAngle;
}

//===================================================================
void IASystem::confNewVisibleShot(const std::vector<uint32_t> &visibleShots)
{
    assert(visibleShots.size() > 1);
    uint32_t targetIndex = visibleShots.size() - 1, baseIndex = targetIndex - 1;
    OptUint_t numCom = m_newComponentManager.getComponentEmplacement(visibleShots[baseIndex], Components_e::MEM_SPRITE_DATA_COMPONENT);
    assert(numCom);
    MemSpriteDataComponent &baseMemSpriteComp = m_componentsContainer.m_vectMemSpriteDataComp[*numCom];
    numCom = m_newComponentManager.getComponentEmplacement(visibleShots[baseIndex], Components_e::FPS_VISIBLE_STATIC_ELEMENT_COMPONENT);
    assert(numCom);
    FPSVisibleStaticElementComponent &baseFpsStaticComp = m_componentsContainer.m_vectFPSVisibleStaticElementComp[*numCom];
    numCom = m_newComponentManager.getComponentEmplacement(visibleShots[targetIndex], Components_e::SPRITE_TEXTURE_COMPONENT);
    assert(numCom);
    SpriteTextureComponent &targetSpriteComp = m_componentsContainer.m_vectSpriteTextureComp[*numCom];
    numCom = m_newComponentManager.getComponentEmplacement(visibleShots[targetIndex], Components_e::MEM_SPRITE_DATA_COMPONENT);
    assert(numCom);
    MemSpriteDataComponent &targetMemSpriteComp = m_componentsContainer.m_vectMemSpriteDataComp[*numCom];
    numCom = m_newComponentManager.getComponentEmplacement(visibleShots[targetIndex], Components_e::FPS_VISIBLE_STATIC_ELEMENT_COMPONENT);
    assert(numCom);
    FPSVisibleStaticElementComponent &targetFpsStaticComp = m_componentsContainer.m_vectFPSVisibleStaticElementComp[*numCom];
    numCom = m_newComponentManager.getComponentEmplacement(visibleShots[baseIndex], Components_e::SHOT_CONF_COMPONENT);
    assert(numCom);
    ShotConfComponent &baseShotConfComp = m_componentsContainer.m_vectShotConfComp[*numCom];
    numCom = m_newComponentManager.getComponentEmplacement(visibleShots[targetIndex], Components_e::SHOT_CONF_COMPONENT);
    assert(numCom);
    ShotConfComponent &targetShotConfComp = m_componentsContainer.m_vectShotConfComp[*numCom];
    numCom = m_newComponentManager.getComponentEmplacement(visibleShots[baseIndex], Components_e::MOVEABLE_COMPONENT);
    assert(numCom);
    MoveableComponent &baseMoveComp = m_componentsContainer.m_vectMoveableComp[*numCom];
    numCom = m_newComponentManager.getComponentEmplacement(visibleShots[targetIndex], Components_e::MOVEABLE_COMPONENT);
    assert(numCom);
    MoveableComponent &targetMoveComp = m_componentsContainer.m_vectMoveableComp[*numCom];
    numCom = m_newComponentManager.getComponentEmplacement(visibleShots[baseIndex], Components_e::MEM_FPS_GLSIZE_COMPONENT);
    assert(numCom);
    MemFPSGLSizeComponent &memFPSGLSizeCompBase = m_componentsContainer.m_vectMemFPSGLSizeComp[*numCom];
    numCom = m_newComponentManager.getComponentEmplacement(visibleShots[targetIndex], Components_e::MEM_FPS_GLSIZE_COMPONENT);
    assert(numCom);
    MemFPSGLSizeComponent &memFPSGLSizeCompTarget = m_componentsContainer.m_vectMemFPSGLSizeComp[*numCom];
    numCom = m_newComponentManager.getComponentEmplacement(visibleShots[targetIndex], Components_e::AUDIO_COMPONENT);
    assert(numCom);
    AudioComponent &audioCompTarget = m_componentsContainer.m_vectAudioComp[*numCom];
    numCom = m_newComponentManager.getComponentEmplacement(visibleShots[baseIndex], Components_e::AUDIO_COMPONENT);
    assert(numCom);
    AudioComponent &audioCompBase = m_componentsContainer.m_vectAudioComp[*numCom];
    audioCompTarget.m_soundElements.push_back(SoundElement());
    audioCompTarget.m_soundElements[0]->m_toPlay = true;
    audioCompTarget.m_soundElements[0]->m_bufferALID = audioCompBase.m_soundElements[0]->m_bufferALID;
    audioCompTarget.m_soundElements[0]->m_sourceALID = mptrSystemManager->searchSystemByType<SoundSystem>(
                static_cast<uint32_t>(Systems_e::SOUND_SYSTEM))->createSource(audioCompBase.m_soundElements[0]->m_bufferALID);
    memFPSGLSizeCompTarget.m_memGLSizeData = memFPSGLSizeCompBase.m_memGLSizeData;
    targetMemSpriteComp.m_vectSpriteData = baseMemSpriteComp.m_vectSpriteData;
    targetSpriteComp.m_spriteData = targetMemSpriteComp.m_vectSpriteData[0];
    targetFpsStaticComp.m_levelElementType = baseFpsStaticComp.m_levelElementType;
    targetFpsStaticComp.m_inGameSpriteSize = memFPSGLSizeCompBase.m_memGLSizeData[0];
    targetMoveComp.m_velocity = baseMoveComp.m_velocity;
    targetShotConfComp.m_damage = baseShotConfComp.m_damage;
    float maxWidth = EPSILON_FLOAT;
    for(uint32_t i = 1; i < memFPSGLSizeCompTarget.m_memGLSizeData.size(); ++i)
    {
        if(maxWidth < memFPSGLSizeCompTarget.m_memGLSizeData[i].first)
        {
            maxWidth = memFPSGLSizeCompTarget.m_memGLSizeData[i].first;
        }
    }
    targetShotConfComp.m_ejectExplosionRay = maxWidth * LEVEL_HALF_TILE_SIZE_PX;
}
