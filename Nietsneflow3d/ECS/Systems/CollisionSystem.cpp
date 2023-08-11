#include "CollisionSystem.hpp"
#include "constants.hpp"
#include <ECS/Components/MapCoordComponent.hpp>
#include <ECS/Components/GeneralCollisionComponent.hpp>
#include <ECS/Components/CircleCollisionComponent.hpp>
#include <ECS/Components/RectangleCollisionComponent.hpp>
#include <ECS/Components/SegmentCollisionComponent.hpp>
#include <ECS/Components/MoveableComponent.hpp>
#include <ECS/Components/DoorComponent.hpp>
#include <ECS/Components/PlayerConfComponent.hpp>
#include <ECS/Components/MoveableWallConfComponent.hpp>
#include <ECS/Components/EnemyConfComponent.hpp>
#include <ECS/Components/TimerComponent.hpp>
#include <ECS/Components/ShotConfComponent.hpp>
#include <ECS/Components/TeleportComponent.hpp>
#include <ECS/Components/LogComponent.hpp>
#include <ECS/Components/ImpactShotComponent.hpp>
#include <ECS/Components/MemSpriteDataComponent.hpp>
#include <ECS/Components/AudioComponent.hpp>
#include <ECS/Components/SpriteTextureComponent.hpp>
#include <ECS/Components/WeaponComponent.hpp>
#include <ECS/Components/BarrelComponent.hpp>
#include <ECS/Components/CheckpointComponent.hpp>
#include <ECS/Components/TriggerComponent.hpp>
#include <ECS/Components/MoveableWallConfComponent.hpp>
#include <ECS/Components/FPSVisibleStaticElementComponent.hpp>
#include <ECS/Systems/FirstPersonDisplaySystem.hpp>
#include <ECS/Systems/ColorDisplaySystem.hpp>
#include <BaseECS/engine.hpp>
#include <CollisionUtils.hpp>
#include <PhysicalEngine.hpp>
#include <MainEngine.hpp>
#include <cassert>
#include <Level.hpp>
#include <math.h>

using multiMapTagIt_t = std::multimap<CollisionTag_e, CollisionTag_e>::const_iterator;

//===================================================================
CollisionSystem::CollisionSystem(NewComponentManager &newComponentManager) : m_newComponentManager(newComponentManager),
    m_componentsContainer(m_newComponentManager.getComponentsContainer())
{
    setUsedComponents();
    initArrayTag();
}

//===================================================================
void CollisionSystem::setUsedComponents()
{
    bAddComponentToSystem(Components_e::GENERAL_COLLISION_COMPONENT);
}

//===================================================================
void CollisionSystem::execSystem()
{
    std::optional<uint32_t> numCompNum, moveCompNum, segmentCompNum;
    System::execSystem();
    m_memPlayerTeleport = false;
    for(uint32_t i = 0; i < mVectNumEntity.size(); ++i)
    {
        numCompNum = m_newComponentManager.getComponentEmplacement(mVectNumEntity[i], Components_e::GENERAL_COLLISION_COMPONENT);
        assert(numCompNum);
        GeneralCollisionComponent &tagCompA = m_componentsContainer.m_vectGeneralCollisionComp[*numCompNum];
        //check if entity is moveable
        moveCompNum = m_newComponentManager.getComponentEmplacement(mVectNumEntity[i], Components_e::MOVEABLE_COMPONENT);
        if(!tagCompA.m_active || tagCompA.m_tagA == CollisionTag_e::WALL_CT || tagCompA.m_tagA == CollisionTag_e::OBJECT_CT ||
                tagCompA.m_tagA == CollisionTag_e::DOOR_CT)
        {
            continue;
        }
        m_memCrush.clear();
        if(tagCompA.m_shape == CollisionShape_e::SEGMENT_C &&
                (tagCompA.m_tagA == CollisionTag_e::BULLET_PLAYER_CT || tagCompA.m_tagA == CollisionTag_e::BULLET_ENEMY_CT))
        {
            m_memDistCurrentBulletColl.second = EPSILON_FLOAT;
            segmentCompNum = m_newComponentManager.getComponentEmplacement(mVectNumEntity[i], Components_e::SEGMENT_COLLISION_COMPONENT);
            assert(segmentCompNum);
            SegmentCollisionComponent &segmentCompA = m_componentsContainer.m_vectSegmentCollisionComp[*segmentCompNum];
            calcBulletSegment(segmentCompA);
            addEntityToZone(segmentCompA.m_impactEntity, *getLevelCoord(segmentCompA.m_points.second));
            tagCompA.m_active = false;
        }
        else
        {
            segmentCompNum = std::nullopt;
        }
        if(tagCompA.m_tagA == CollisionTag_e::ENEMY_CT)
        {
            if(checkEnemyRemoveCollisionMask(mVectNumEntity[i]))
            {
                rmEnemyCollisionMaskEntity(mVectNumEntity[i]);
            }
        }
        secondEntitiesLoop(mVectNumEntity[i], i, tagCompA);
        if(tagCompA.m_tagA == CollisionTag_e::EXPLOSION_CT)
        {
            setDamageCircle(mVectNumEntity[i], false);
        }
        else if(tagCompA.m_tagA == CollisionTag_e::PLAYER_ACTION_CT)
        {
            tagCompA.m_active = false;
        }
        if(moveCompNum && (tagCompA.m_tagA == CollisionTag_e::PLAYER_CT || tagCompA.m_tagA == CollisionTag_e::ENEMY_CT))
        {
            treatGeneralCrushing(mVectNumEntity[i]);
        }
        if(segmentCompNum && m_memDistCurrentBulletColl.second > EPSILON_FLOAT)
        {
            if(m_memDistCurrentBulletColl.first)
            {
                m_vectMemShots.emplace_back(PairUI_t{mVectNumEntity[i], (*m_memDistCurrentBulletColl.first)});
            }
        }
        treatSegmentShots();
        m_vectMemShots.clear();
    }
    if(!m_memPlayerTeleport)
    {
        numCompNum = m_newComponentManager.getComponentEmplacement(m_playerEntity, Components_e::PLAYER_CONF_COMPONENT);
        assert(numCompNum);
        PlayerConfComponent &playerComp = m_componentsContainer.m_vectPlayerConfComp[*numCompNum];
        playerComp.m_teleported = false;
    }
}

//===================================================================
void CollisionSystem::updateZonesColl()
{
    m_zoneLevel = std::make_unique<ZoneLevelColl>(Level::getSize());
}

//===================================================================
void CollisionSystem::secondEntitiesLoop(uint32_t entityA, uint32_t currentIteration, GeneralCollisionComponent &tagCompA, bool shotExplosionEject)
{
    if(tagCompA.m_tagA == CollisionTag_e::DETECT_MAP_CT ||
            tagCompA.m_tagA == CollisionTag_e::BULLET_PLAYER_CT || tagCompA.m_tagA == CollisionTag_e::BULLET_ENEMY_CT)
    {
        for(uint32_t i = 0; i < mVectNumEntity.size(); ++i)
        {
            if(!iterationLoop(currentIteration, entityA, mVectNumEntity[i], tagCompA, shotExplosionEject))
            {
                return;
            }
        }
        return;
    }
    SetUi_t set = m_zoneLevel->getEntitiesFromZones(entityA);
    SetUi_t::iterator it = set.begin();
    for(; it != set.end(); ++it)
    {
        if(!iterationLoop(currentIteration, entityA, *it, tagCompA, shotExplosionEject))
        {
            return;
        }
    }
}

//===================================================================
bool CollisionSystem::iterationLoop(uint32_t currentIteration, uint32_t entityA, uint32_t entityB,
                                    GeneralCollisionComponent &tagCompA, bool shotExplosionEject)
{
    if(currentIteration == entityB)
    {
        return true;
    }
    OptUint_t compNum = m_newComponentManager.getComponentEmplacement(entityB, Components_e::GENERAL_COLLISION_COMPONENT);
    assert(compNum);
    GeneralCollisionComponent &tagCompB = m_componentsContainer.m_vectGeneralCollisionComp[*compNum];
    if(!tagCompB.m_active)
    {
        return true;
    }
    if(!checkTag(tagCompA.m_tagA, tagCompB.m_tagA) && !checkTag(tagCompA.m_tagA, tagCompB.m_tagB))
    {
        return true;
    }
    if(!treatCollision(entityA, entityB, tagCompA, tagCompB, shotExplosionEject))
    {
        if(tagCompA.m_tagA == CollisionTag_e::BULLET_PLAYER_CT || tagCompA.m_tagA == CollisionTag_e::BULLET_ENEMY_CT)
        {
            secondEntitiesLoop(entityA, currentIteration, tagCompA, true);
        }
        return false;
    }
    return true;
}

//===================================================================
bool CollisionSystem::checkEnemyRemoveCollisionMask(uint32_t entityNum)
{
    OptUint_t numCom = m_newComponentManager.getComponentEmplacement(entityNum, Components_e::ENEMY_CONF_COMPONENT);
    assert(numCom);
    EnemyConfComponent &enemyConfComp = m_componentsContainer.m_vectEnemyConfComp[*numCom];
    if(enemyConfComp.m_displayMode == EnemyDisplayMode_e::DEAD)
    {
        numCom = m_newComponentManager.getComponentEmplacement(entityNum, Components_e::MOVEABLE_COMPONENT);
        assert(numCom);
        MoveableComponent &moveComp = m_componentsContainer.m_vectMoveableComp[*numCom];
        if(!moveComp.m_ejectData)
        {
            return true;
        }
    }
    return false;
}

//===================================================================
void CollisionSystem::treatGeneralCrushing(uint32_t entityNum)
{
    OptUint_t compNum = m_newComponentManager.getComponentEmplacement(entityNum, Components_e::MAP_COORD_COMPONENT);
    assert(compNum);
    MapCoordComponent &mapComp = m_componentsContainer.m_vectMapCoordComp[*compNum];
    compNum = m_newComponentManager.getComponentEmplacement(entityNum, Components_e::GENERAL_COLLISION_COMPONENT);
    assert(compNum);
    GeneralCollisionComponent &collComp = m_componentsContainer.m_vectGeneralCollisionComp[*compNum];
    bool crush = false;
    for(uint32_t i = 0; i < m_memCrush.size(); ++i)
    {
        //QuickFix
        if(collComp.m_tagA == CollisionTag_e::ENEMY_CT)
        {
            mapComp.m_absoluteMapPositionPX.first += std::get<0>(m_memCrush[i]).first;
            mapComp.m_absoluteMapPositionPX.second += std::get<0>(m_memCrush[i]).second;
        }
        //3 == direction
        if(!crush && !std::get<1>(m_memCrush[i]))
        {
            for(uint32_t j = 0; j < i; ++j)
            {
                if((std::get<3>(m_memCrush[j]) || std::get<3>(m_memCrush[i])) &&
                        opposingDirection(std::get<2>(m_memCrush[j]), std::get<2>(m_memCrush[i])))
                {
                    crush = true;
                    treatCrushing(entityNum);
                    break;
                }
            }
        }
    }
    if(collComp.m_tagA != CollisionTag_e::PLAYER_CT)
    {
        return;
    }
    compNum = m_newComponentManager.getComponentEmplacement(entityNum, Components_e::PLAYER_CONF_COMPONENT);
    assert(compNum);
    PlayerConfComponent &playerComp = m_componentsContainer.m_vectPlayerConfComp[*compNum];
    if(!crush && !playerComp.m_insideWall)
    {
        playerComp.m_crush = false;
        playerComp.m_frozen = false;
    }
    if(playerComp.m_crush)
    {
        playerComp.takeDamage(1);
    }
}

//===================================================================
void CollisionSystem::treatEnemyTakeDamage(uint32_t enemyEntityNum, uint32_t damage)
{
    OptUint_t compNum = m_newComponentManager.getComponentEmplacement(enemyEntityNum, Components_e::ENEMY_CONF_COMPONENT);
    assert(compNum);
    EnemyConfComponent &enemyConfCompB = m_componentsContainer.m_vectEnemyConfComp[*compNum];

    compNum = m_newComponentManager.getComponentEmplacement(enemyEntityNum, Components_e::TIMER_COMPONENT);
    assert(compNum);
    TimerComponent &timerComp = m_componentsContainer.m_vectTimerComp[*compNum];
    if(enemyConfCompB.m_behaviourMode == EnemyBehaviourMode_e::DYING)
    {
        return;
    }
    enemyConfCompB.m_touched = true;
    enemyConfCompB.m_behaviourMode = EnemyBehaviourMode_e::ATTACK;
    if(enemyConfCompB.m_frozenOnAttack)
    {
        timerComp.m_cycleCountC = 0;
        timerComp.m_cycleCountB = 0;
        enemyConfCompB.m_attackPhase = EnemyAttackPhase_e::SHOOTED;
    }

    compNum = m_newComponentManager.getComponentEmplacement(m_playerEntity, Components_e::PLAYER_CONF_COMPONENT);
    assert(compNum);
    PlayerConfComponent &playerComp = m_componentsContainer.m_vectPlayerConfComp[*compNum];
    //if enemy dead
    if(!enemyConfCompB.takeDamage(damage))
    {
        if(!playerComp.m_enemiesKilled)
        {
            playerComp.m_enemiesKilled = 1;
        }
        else
        {
            ++(*playerComp.m_enemiesKilled);
        }
        enemyConfCompB.m_behaviourMode = EnemyBehaviourMode_e::DYING;
        enemyConfCompB.m_touched = false;
        enemyConfCompB.m_playDeathSound = true;
        if(enemyConfCompB.m_dropedObjectEntity)
        {
            confDropedObject(*enemyConfCompB.m_dropedObjectEntity, enemyEntityNum);
        }
    }
}

//===================================================================
void CollisionSystem::confDropedObject(uint32_t objectEntity, uint32_t enemyEntity)
{
    OptUint_t compNum = m_newComponentManager.getComponentEmplacement(objectEntity, Components_e::GENERAL_COLLISION_COMPONENT);
    assert(compNum);
    GeneralCollisionComponent &genComp = m_componentsContainer.m_vectGeneralCollisionComp[*compNum];
    compNum = m_newComponentManager.getComponentEmplacement(objectEntity, Components_e::MAP_COORD_COMPONENT);
    assert(compNum);
    MapCoordComponent &objectMapComp = m_componentsContainer.m_vectMapCoordComp[*compNum];
    compNum = m_newComponentManager.getComponentEmplacement(enemyEntity, Components_e::MAP_COORD_COMPONENT);
    assert(compNum);
    MapCoordComponent &enemyMapComp = m_componentsContainer.m_vectMapCoordComp[*compNum];
    genComp.m_active = true;
    objectMapComp.m_coord = enemyMapComp.m_coord;
    addEntityToZone(objectEntity, objectMapComp.m_coord);
    objectMapComp.m_absoluteMapPositionPX = enemyMapComp.m_absoluteMapPositionPX;
}

//===================================================================
void CollisionSystem::treatSegmentShots()
{
    for(uint32_t i = 0; i < m_vectMemShots.size(); ++i)
    {
        OptUint_t compNum = m_newComponentManager.getComponentEmplacement(m_vectMemShots[i].second, Components_e::GENERAL_COLLISION_COMPONENT);
        assert(compNum);
        GeneralCollisionComponent &tagCompTarget = m_componentsContainer.m_vectGeneralCollisionComp[*compNum];
        confImpactShots(i, tagCompTarget.m_tagA);
        if(tagCompTarget.m_tagA == CollisionTag_e::WALL_CT || tagCompTarget.m_tagA == CollisionTag_e::DOOR_CT)
        {
            continue;
        }
        compNum = m_newComponentManager.getComponentEmplacement(m_vectMemShots[i].first, Components_e::GENERAL_COLLISION_COMPONENT);
        assert(compNum);
        GeneralCollisionComponent &tagCompBullet = m_componentsContainer.m_vectGeneralCollisionComp[*compNum];

        if(tagCompBullet.m_tagA == CollisionTag_e::BULLET_PLAYER_CT)
        {
            compNum = m_newComponentManager.getComponentEmplacement(m_vectMemShots[i].first, Components_e::SHOT_CONF_COMPONENT);
            assert(compNum);
            ShotConfComponent &shotConfComp = m_componentsContainer.m_vectShotConfComp[*compNum];

            if(tagCompTarget.m_tagA == CollisionTag_e::ENEMY_CT)
            {
                treatEnemyTakeDamage(m_vectMemShots[i].second, shotConfComp.m_damage);
            }
            else if(tagCompTarget.m_tagA == CollisionTag_e::BARREL_CT)
            {
                treatBarrelShots(m_vectMemShots[i].second, shotConfComp.m_damage);
            }
            tagCompBullet.m_active = false;
        }
        else if(tagCompBullet.m_tagA == CollisionTag_e::BULLET_ENEMY_CT)
        {
            compNum = m_newComponentManager.getComponentEmplacement(m_vectMemShots[i].first, Components_e::SHOT_CONF_COMPONENT);
            assert(compNum);
            ShotConfComponent &shotConfComp = m_componentsContainer.m_vectShotConfComp[*compNum];

            if(tagCompTarget.m_tagA == CollisionTag_e::PLAYER_CT)
            {
                compNum = m_newComponentManager.getComponentEmplacement(m_playerEntity, Components_e::PLAYER_CONF_COMPONENT);
                assert(compNum);
                PlayerConfComponent &playerComp = m_componentsContainer.m_vectPlayerConfComp[*compNum];
                playerComp.takeDamage(shotConfComp.m_damage);
            }
            else if(tagCompTarget.m_tagA == CollisionTag_e::BARREL_CT)
            {
                treatBarrelShots(m_vectMemShots[i].second, shotConfComp.m_damage);
            }
        }
    }
}

//===================================================================
void CollisionSystem::treatBarrelShots(uint32_t entityNum, uint32_t damage)
{
    OptUint_t compNum = m_newComponentManager.getComponentEmplacement(entityNum, Components_e::BARREL_COMPONENT);
    assert(compNum);
    BarrelComponent &barrelComp = m_componentsContainer.m_vectBarrelComp[*compNum];
    if(barrelComp.m_destructPhase)
    {
        return;
    }
    if(damage > barrelComp.m_life)
    {
        barrelComp.m_life = 0;
        barrelComp.m_destructPhase = true;
        activeSound(entityNum);
        m_vectBarrelsEntitiesDestruct.push_back(entityNum);

    }
    else
    {
        barrelComp.m_life -= damage;
    }
}

//===================================================================
void CollisionSystem::confImpactShots(uint32_t numBullet, CollisionTag_e targetTag)
{

    OptUint_t compNum = m_newComponentManager.getComponentEmplacement(m_vectMemShots[numBullet].first, Components_e::SHOT_CONF_COMPONENT);
    assert(compNum);
    ShotConfComponent &shotComp = m_componentsContainer.m_vectShotConfComp[*compNum];
    //remove warning
    uint32_t impactEntity = shotComp.m_impactEntity;
    compNum = m_newComponentManager.getComponentEmplacement(impactEntity, Components_e::GENERAL_COLLISION_COMPONENT);
    assert(compNum);
    GeneralCollisionComponent &genImpact = m_componentsContainer.m_vectGeneralCollisionComp[*compNum];
    compNum = m_newComponentManager.getComponentEmplacement(impactEntity, Components_e::IMPACT_CONF_COMPONENT);
    assert(compNum);
    ImpactShotComponent &impactComp = m_componentsContainer.m_vectImpactShotComp[*compNum];
    compNum = m_newComponentManager.getComponentEmplacement(m_vectMemShots[numBullet].first, Components_e::SEGMENT_COLLISION_COMPONENT);
    assert(compNum);
    SegmentCollisionComponent &segmentBullet = m_componentsContainer.m_vectSegmentCollisionComp[*compNum];
    compNum = m_newComponentManager.getComponentEmplacement(impactEntity, Components_e::MAP_COORD_COMPONENT);
    assert(compNum);
    MapCoordComponent &mapImpact = m_componentsContainer.m_vectMapCoordComp[*compNum];
    compNum = m_newComponentManager.getComponentEmplacement(impactEntity, Components_e::TIMER_COMPONENT);
    assert(compNum);
    TimerComponent &timerImpact = m_componentsContainer.m_vectTimerComp[*compNum];
    compNum = m_newComponentManager.getComponentEmplacement(impactEntity, Components_e::SPRITE_TEXTURE_COMPONENT);
    assert(compNum);
    SpriteTextureComponent &spriteComp = m_componentsContainer.m_vectSpriteTextureComp[*compNum];
    compNum = m_newComponentManager.getComponentEmplacement(impactEntity, Components_e::MEM_SPRITE_DATA_COMPONENT);
    assert(compNum);
    MemSpriteDataComponent &memSpriteComp = m_componentsContainer.m_vectMemSpriteDataComp[*compNum];
    impactComp.m_moveUp = EPSILON_FLOAT;
    impactComp.m_touched = (targetTag == CollisionTag_e::ENEMY_CT || targetTag == CollisionTag_e::PLAYER_CT);
    if(impactComp.m_touched)
    {
        genImpact.m_tagA = CollisionTag_e::GHOST_CT;
        impactComp.m_spritePhase = ImpactPhase_e::TOUCHED;
    }
    else
    {
        genImpact.m_tagA = CollisionTag_e::IMPACT_CT;
        impactComp.m_spritePhase = ImpactPhase_e::FIRST;
    }
    spriteComp.m_spriteData = memSpriteComp.m_vectSpriteData[static_cast<uint32_t>(impactComp.m_spritePhase)];
    if(m_memDistCurrentBulletColl.second >= 10000.0f)
    {
        mapImpact.m_absoluteMapPositionPX = segmentBullet.m_points.second;
    }
    else
    {
        mapImpact.m_absoluteMapPositionPX = segmentBullet.m_points.first;
        float radiantAngle = getTrigoAngle(segmentBullet.m_points.first,
                                           segmentBullet.m_points.second, false);
        moveElementFromAngle(m_memDistCurrentBulletColl.second, radiantAngle, mapImpact.m_absoluteMapPositionPX);
    }
    genImpact.m_active = true;
    timerImpact.m_cycleCountA = 0;
}

//===================================================================
void CollisionSystem::rmEnemyCollisionMaskEntity(uint32_t numEntity)
{
    OptUint_t compNum = m_newComponentManager.getComponentEmplacement(numEntity, Components_e::GENERAL_COLLISION_COMPONENT);
    assert(compNum);
    GeneralCollisionComponent &tagComp = m_componentsContainer.m_vectGeneralCollisionComp[*compNum];

    tagComp.m_tagA = CollisionTag_e::DEAD_CORPSE_CT;
}

//===================================================================
void CollisionSystem::initArrayTag()
{
    m_tagArray.insert({CollisionTag_e::PLAYER_CT, CollisionTag_e::WALL_CT});
    m_tagArray.insert({CollisionTag_e::PLAYER_CT, CollisionTag_e::DOOR_CT});
    m_tagArray.insert({CollisionTag_e::PLAYER_CT, CollisionTag_e::ENEMY_CT});
    m_tagArray.insert({CollisionTag_e::PLAYER_CT, CollisionTag_e::OBJECT_CT});
    m_tagArray.insert({CollisionTag_e::PLAYER_CT, CollisionTag_e::STATIC_SET_CT});
    m_tagArray.insert({CollisionTag_e::PLAYER_CT, CollisionTag_e::TRIGGER_CT});
    m_tagArray.insert({CollisionTag_e::PLAYER_CT, CollisionTag_e::TELEPORT_CT});
    m_tagArray.insert({CollisionTag_e::PLAYER_CT, CollisionTag_e::BARREL_CT});
    m_tagArray.insert({CollisionTag_e::PLAYER_CT, CollisionTag_e::LOG_CT});
    m_tagArray.insert({CollisionTag_e::PLAYER_CT, CollisionTag_e::CHECKPOINT_CT});
    m_tagArray.insert({CollisionTag_e::PLAYER_CT, CollisionTag_e::SECRET_CT});

    m_tagArray.insert({CollisionTag_e::DETECT_MAP_CT, CollisionTag_e::WALL_CT});
    m_tagArray.insert({CollisionTag_e::DETECT_MAP_CT, CollisionTag_e::DOOR_CT});
    m_tagArray.insert({CollisionTag_e::DETECT_MAP_CT, CollisionTag_e::STATIC_SET_CT});
    m_tagArray.insert({CollisionTag_e::DETECT_MAP_CT, CollisionTag_e::TRIGGER_CT});
//    m_tagArray.insert({CollisionTag_e::DETECT_MAP_CT, CollisionTag_e::TELEPORT_CT});
    m_tagArray.insert({CollisionTag_e::DETECT_MAP_CT, CollisionTag_e::BARREL_CT});
    m_tagArray.insert({CollisionTag_e::DETECT_MAP_CT, CollisionTag_e::EXIT_CT});
    m_tagArray.insert({CollisionTag_e::DETECT_MAP_CT, CollisionTag_e::LOG_CT});

    m_tagArray.insert({CollisionTag_e::BARREL_CT, CollisionTag_e::BARREL_CT});
    m_tagArray.insert({CollisionTag_e::BARREL_CT, CollisionTag_e::WALL_CT});
    m_tagArray.insert({CollisionTag_e::BARREL_CT, CollisionTag_e::DOOR_CT});
    m_tagArray.insert({CollisionTag_e::BARREL_CT, CollisionTag_e::STATIC_SET_CT});
    m_tagArray.insert({CollisionTag_e::BARREL_CT, CollisionTag_e::LOG_CT});

    m_tagArray.insert({CollisionTag_e::PLAYER_ACTION_CT, CollisionTag_e::DOOR_CT});
    m_tagArray.insert({CollisionTag_e::PLAYER_ACTION_CT, CollisionTag_e::EXIT_CT});
    m_tagArray.insert({CollisionTag_e::PLAYER_ACTION_CT, CollisionTag_e::TRIGGER_CT});
    m_tagArray.insert({CollisionTag_e::PLAYER_ACTION_CT, CollisionTag_e::LOG_CT});
    m_tagArray.insert({CollisionTag_e::HIT_PLAYER_CT, CollisionTag_e::ENEMY_CT});

    m_tagArray.insert({CollisionTag_e::EXPLOSION_CT, CollisionTag_e::PLAYER_CT});
    m_tagArray.insert({CollisionTag_e::EXPLOSION_CT, CollisionTag_e::ENEMY_CT});
    m_tagArray.insert({CollisionTag_e::EXPLOSION_CT, CollisionTag_e::BARREL_CT});

    m_tagArray.insert({CollisionTag_e::ENEMY_CT, CollisionTag_e::PLAYER_CT});
    m_tagArray.insert({CollisionTag_e::ENEMY_CT, CollisionTag_e::WALL_CT});
    m_tagArray.insert({CollisionTag_e::ENEMY_CT, CollisionTag_e::DOOR_CT});
    m_tagArray.insert({CollisionTag_e::ENEMY_CT, CollisionTag_e::DOOR_CT});
    m_tagArray.insert({CollisionTag_e::ENEMY_CT, CollisionTag_e::STATIC_SET_CT});
    m_tagArray.insert({CollisionTag_e::ENEMY_CT, CollisionTag_e::BARREL_CT});
    m_tagArray.insert({CollisionTag_e::ENEMY_CT, CollisionTag_e::LOG_CT});
//    m_tagArray.insert({CollisionTag_e::ENEMY_CT, CollisionTag_e::ENEMY_CT});

    m_tagArray.insert({CollisionTag_e::WALL_CT, CollisionTag_e::PLAYER_CT});
    m_tagArray.insert({CollisionTag_e::WALL_CT, CollisionTag_e::ENEMY_CT});

    m_tagArray.insert({CollisionTag_e::DOOR_CT, CollisionTag_e::PLAYER_CT});
    m_tagArray.insert({CollisionTag_e::DOOR_CT, CollisionTag_e::ENEMY_CT});

    //bullets collision with walls and doors are treated by raycasting
    m_tagArray.insert({CollisionTag_e::BULLET_ENEMY_CT, CollisionTag_e::PLAYER_CT});
    m_tagArray.insert({CollisionTag_e::BULLET_ENEMY_CT, CollisionTag_e::BARREL_CT});
    m_tagArray.insert({CollisionTag_e::BULLET_ENEMY_CT, CollisionTag_e::WALL_CT});
    m_tagArray.insert({CollisionTag_e::BULLET_ENEMY_CT, CollisionTag_e::DOOR_CT});
//    m_tagArray.insert({CollisionTag_e::BULLET_ENEMY_CT, CollisionTag_e::ENEMY_CT});

    m_tagArray.insert({CollisionTag_e::BULLET_PLAYER_CT, CollisionTag_e::ENEMY_CT});
    m_tagArray.insert({CollisionTag_e::BULLET_PLAYER_CT, CollisionTag_e::BARREL_CT});
    m_tagArray.insert({CollisionTag_e::BULLET_PLAYER_CT, CollisionTag_e::WALL_CT});
    m_tagArray.insert({CollisionTag_e::BULLET_PLAYER_CT, CollisionTag_e::DOOR_CT});

    m_tagArray.insert({CollisionTag_e::IMPACT_CT, CollisionTag_e::WALL_CT});
    m_tagArray.insert({CollisionTag_e::IMPACT_CT, CollisionTag_e::DOOR_CT});
    m_tagArray.insert({CollisionTag_e::IMPACT_CT, CollisionTag_e::ENEMY_CT});

    m_tagArray.insert({CollisionTag_e::DEAD_CORPSE_CT, CollisionTag_e::DOOR_CT});
    m_tagArray.insert({CollisionTag_e::DEAD_CORPSE_CT, CollisionTag_e::WALL_CT});
    //    m_tagArray.insert({CollisionTag_e::OBJECT_CT, CollisionTag_e::PLAYER_CT});
}

//===================================================================
bool CollisionSystem::checkTag(CollisionTag_e entityTagA, CollisionTag_e entityTagB)
{
    for(multiMapTagIt_t it = m_tagArray.find(entityTagA); it != m_tagArray.end() ; ++it)
    {
        if(it->first == entityTagA && it->second == entityTagB)
        {
            return true;
        }
    }
    return false;
}

//===================================================================
bool CollisionSystem::treatCollision(uint32_t entityNumA, uint32_t entityNumB, GeneralCollisionComponent &tagCompA,
                                     GeneralCollisionComponent &tagCompB, bool shotExplosionEject)
{
    if(tagCompA.m_shape == CollisionShape_e::RECTANGLE_C)
    {
        OptUint_t compNum = m_newComponentManager.getComponentEmplacement(entityNumA, Components_e::MAP_COORD_COMPONENT);
        assert(compNum);
        MapCoordComponent &mapCompA = m_componentsContainer.m_vectMapCoordComp[*compNum];
        compNum = m_newComponentManager.getComponentEmplacement(entityNumB, Components_e::MAP_COORD_COMPONENT);
        assert(compNum);
        MapCoordComponent &mapCompB = m_componentsContainer.m_vectMapCoordComp[*compNum];
        CollisionArgs args = {entityNumA, entityNumB, tagCompA, tagCompB, mapCompA, mapCompB};
        checkCollisionFirstRect(args);
    }
    else if(tagCompA.m_shape == CollisionShape_e::CIRCLE_C)
    {
        OptUint_t compNum = m_newComponentManager.getComponentEmplacement(entityNumA, Components_e::MAP_COORD_COMPONENT);
        assert(compNum);
        MapCoordComponent &mapCompA = m_componentsContainer.m_vectMapCoordComp[*compNum];
        compNum = m_newComponentManager.getComponentEmplacement(entityNumB, Components_e::MAP_COORD_COMPONENT);
        assert(compNum);
        MapCoordComponent &mapCompB = m_componentsContainer.m_vectMapCoordComp[*compNum];
        CollisionArgs args = {entityNumA, entityNumB, tagCompA, tagCompB, mapCompA, mapCompB};
        return treatCollisionFirstCircle(args, shotExplosionEject);
    }
    else if(tagCompA.m_shape == CollisionShape_e::SEGMENT_C)
    {
        OptUint_t compNum = m_newComponentManager.getComponentEmplacement(entityNumB, Components_e::MAP_COORD_COMPONENT);
        assert(compNum);
        MapCoordComponent &mapCompA = m_componentsContainer.m_vectMapCoordComp[*compNum];
        assert(tagCompA.m_tagA == CollisionTag_e::BULLET_PLAYER_CT || tagCompA.m_tagA == CollisionTag_e::BULLET_ENEMY_CT);
        checkCollisionFirstSegment(entityNumA, entityNumB, tagCompB, mapCompA);
    }
    return true;
}

//Detect map only
//===================================================================
void CollisionSystem::checkCollisionFirstRect(CollisionArgs &args)
{
    MapDisplaySystem *mapSystem = mptrSystemManager->searchSystemByType<MapDisplaySystem>(
                    static_cast<uint32_t>(Systems_e::MAP_DISPLAY_SYSTEM));
    assert(mapSystem);
    if(mapSystem->entityAlreadyDiscovered(args.entityNumB))
    {
        return;
    }
    bool collision = false;
    OptUint_t compNum = m_newComponentManager.getComponentEmplacement(args.entityNumA, Components_e::RECTANGLE_COLLISION_COMPONENT);
    assert(compNum);
    RectangleCollisionComponent &rectCompA = m_componentsContainer.m_vectRectangleCollisionComp[*compNum];
    switch(args.tagCompB.m_shape)
    {
    case CollisionShape_e::RECTANGLE_C:
    {
        OptUint_t compNum = m_newComponentManager.getComponentEmplacement(args.entityNumB, Components_e::RECTANGLE_COLLISION_COMPONENT);
        assert(compNum);
        RectangleCollisionComponent &rectCompB = m_componentsContainer.m_vectRectangleCollisionComp[*compNum];
        collision = checkRectRectCollision(args.mapCompA.m_absoluteMapPositionPX, rectCompA.m_size,
                               args.mapCompB.m_absoluteMapPositionPX, rectCompB.m_size);
    }
        break;
    case CollisionShape_e::CIRCLE_C:
    {
        OptUint_t compNum = m_newComponentManager.getComponentEmplacement(args.entityNumB, Components_e::CIRCLE_COLLISION_COMPONENT);
        assert(compNum);
        CircleCollisionComponent &circleCompB = m_componentsContainer.m_vectCircleCollisionComp[*compNum];
        collision = checkCircleRectCollision(args.mapCompB.m_absoluteMapPositionPX, circleCompB.m_ray,
                                 args.mapCompA.m_absoluteMapPositionPX, rectCompA.m_size);
    }
        break;
    case CollisionShape_e::SEGMENT_C:
        break;
    }
    if(collision)
    {
        mapSystem->addDiscoveredEntity(args.entityNumB, *getLevelCoord(args.mapCompB.m_absoluteMapPositionPX));
    }
}

//===================================================================
void CollisionSystem::writePlayerInfo(const std::string &info)
{
    OptUint_t compNumPlayer = m_newComponentManager.getComponentEmplacement(m_playerEntity, Components_e::PLAYER_CONF_COMPONENT);
    assert(compNumPlayer);
    PlayerConfComponent &playerComp = m_componentsContainer.m_vectPlayerConfComp[*compNumPlayer];
    OptUint_t compNum = m_newComponentManager.getComponentEmplacement(playerComp.muiGetIdEntityAssociated(),
                                                                      Components_e::TIMER_COMPONENT);
    assert(compNum);
    TimerComponent &timerComp = m_componentsContainer.m_vectTimerComp[*compNum];
    timerComp.m_cycleCountA = 0;
    playerComp.m_infoWriteData = {true, info};
}

//===================================================================
bool CollisionSystem::treatDoorCollisionFirstCircle(CollisionArgs &args, const CircleCollisionComponent &circleCompA,
                                                    const RectangleCollisionComponent &rectCompB)
{
    OptUint_t compNum = m_newComponentManager.getComponentEmplacement(args.entityNumB, Components_e::DOOR_COMPONENT);
    assert(compNum);
    DoorComponent &doorComp = m_componentsContainer.m_vectDoorComp[*compNum];
    PairFloat_t size = rectCompB.m_size;
    //Fix visible shot issue
    if(args.tagCompA.m_tagA != CollisionTag_e::BULLET_ENEMY_CT &&
            args.tagCompA.m_tagA != CollisionTag_e::BULLET_PLAYER_CT && args.tagCompA.m_tagA != CollisionTag_e::IMPACT_CT)
    {
        if(doorComp.m_vertical)
        {
            size.second = LEVEL_TILE_SIZE_PX;
        }
        else
        {
            size.first = LEVEL_TILE_SIZE_PX;
        }
    }
    return checkCircleRectCollision(args.mapCompA.m_absoluteMapPositionPX, circleCompA.m_ray,
                                    args.mapCompB.m_absoluteMapPositionPX, size);
}

//===================================================================
bool CollisionSystem::treatCollisionFirstCircle(CollisionArgs &args, bool shotExplosionEject)
{
    if(args.tagCompA.m_tagA == CollisionTag_e::PLAYER_ACTION_CT ||
            args.tagCompA.m_tagA == CollisionTag_e::HIT_PLAYER_CT)
    {
        args.tagCompA.m_active = false;
    }
    OptUint_t compNum = m_newComponentManager.getComponentEmplacement(args.entityNumA, Components_e::CIRCLE_COLLISION_COMPONENT);
    assert(compNum);
    CircleCollisionComponent &circleCompA = m_componentsContainer.m_vectCircleCollisionComp[*compNum];
    bool collision = false;
    switch(args.tagCompB.m_shape)
    {
    case CollisionShape_e::RECTANGLE_C:
    {
        OptUint_t compNum = m_newComponentManager.getComponentEmplacement(args.entityNumB, Components_e::RECTANGLE_COLLISION_COMPONENT);
        assert(compNum);
        RectangleCollisionComponent &rectCompB = m_componentsContainer.m_vectRectangleCollisionComp[*compNum];
        if(args.tagCompB.m_tagA == CollisionTag_e::DOOR_CT)
        {
            collision = treatDoorCollisionFirstCircle(args, circleCompA, rectCompB);
        }
        else
        {
            collision = checkCircleRectCollision(args.mapCompA.m_absoluteMapPositionPX, circleCompA.m_ray,
                                                 args.mapCompB.m_absoluteMapPositionPX, rectCompB.m_size);
        }
        if(collision)
        {
            if(args.tagCompA.m_tagA == CollisionTag_e::PLAYER_ACTION_CT)
            {
                treatActionPlayerRect(args);
            }
            else if(args.tagCompA.m_tagA == CollisionTag_e::PLAYER_CT)
            {
                if(treatCollisionPlayer(args, circleCompA, rectCompB))
                {
                    return true;
                }
            }
            else if(args.tagCompA.m_tagA == CollisionTag_e::ENEMY_CT)
            {
                bool checkStuck = (args.tagCompB.m_tagA == CollisionTag_e::BARREL_CT || args.tagCompB.m_tagA == CollisionTag_e::CHECKPOINT_CT ||
                                   args.tagCompB.m_tagA == CollisionTag_e::LOG_CT || args.tagCompB.m_tagA == CollisionTag_e::STATIC_SET_CT ||
                                   args.tagCompB.m_tagA == CollisionTag_e::TRIGGER_CT || args.tagCompB.m_tagA == CollisionTag_e::TELEPORT_CT ||
                                   args.tagCompB.m_tagA == CollisionTag_e::WALL_CT);
                PairFloat_t previousPos;
                if(args.tagCompB.m_tagA == CollisionTag_e::DOOR_CT)
                {
                    OptUint_t compNum = m_newComponentManager.getComponentEmplacement(args.entityNumB, Components_e::DOOR_COMPONENT);
                    assert(compNum);
                    DoorComponent &doorComp = m_componentsContainer.m_vectDoorComp[*compNum];
                    doorComp.m_obstruct = true;
                    if(doorComp.m_currentState == DoorState_e::STATIC_OPEN)
                    {
                        return true;
                    }
                    if(!doorComp.m_cardID)
                    {
                        DoorState_e initState = doorComp.m_currentState;
                        doorComp.m_currentState = DoorState_e::MOVE_OPEN;
                        activeSound(args.entityNumB);
                        if(initState == DoorState_e::MOVE_CLOSE)
                        {
                            return true;
                        }
                    }
                }
                else if(checkStuck)
                {
                    previousPos = args.mapCompA.m_absoluteMapPositionPX;
                }
                collisionCircleRectEject(args, circleCompA.m_ray, rectCompB);
                if(checkStuck && std::abs(previousPos.first - args.mapCompA.m_absoluteMapPositionPX.first) < 3.0f &&
                        std::abs(previousPos.second - args.mapCompA.m_absoluteMapPositionPX.second) < 3.0f)
                {
                    OptUint_t compNum = m_newComponentManager.getComponentEmplacement(args.entityNumA, Components_e::FPS_VISIBLE_STATIC_ELEMENT_COMPONENT);
                    assert(compNum);
                    FPSVisibleStaticElementComponent &fpsComp = m_componentsContainer.m_vectFPSVisibleStaticElementComp[*compNum];
                    if(fpsComp.m_inGameSpriteSize.first < 0.7f)
                    {
                        OptUint_t compNum = m_newComponentManager.getComponentEmplacement(args.entityNumA, Components_e::ENEMY_CONF_COMPONENT);
                        assert(compNum);
                        EnemyConfComponent &enemyComp = m_componentsContainer.m_vectEnemyConfComp[*compNum];
                        enemyComp.m_stuck = true;
                    }
                }
            }
            else if(args.tagCompA.m_tagA == CollisionTag_e::IMPACT_CT ||
                    args.tagCompA.m_tagA == CollisionTag_e::BARREL_CT)
            {
                collisionCircleRectEject(args, circleCompA.m_ray, rectCompB);
            }
            if(args.tagCompA.m_tagA == CollisionTag_e::DEAD_CORPSE_CT)
            {
                //if the wall is static or door
                if(!m_newComponentManager.getComponentEmplacement(args.entityNumB, Components_e::MOVEABLE_WALL_CONF_COMPONENT))
                {
                    collisionCircleRectEject(args, circleCompA.m_ray, rectCompB);
                }
            }
        }
    }
        break;
    case CollisionShape_e::CIRCLE_C:
    {
        OptUint_t compNum = m_newComponentManager.getComponentEmplacement(args.entityNumB, Components_e::CIRCLE_COLLISION_COMPONENT);
        assert(compNum);
        CircleCollisionComponent &circleCompB = m_componentsContainer.m_vectCircleCollisionComp[*compNum];
        collision = checkCircleCircleCollision(args.mapCompA.m_absoluteMapPositionPX, circleCompA.m_ray,
                                               args.mapCompB.m_absoluteMapPositionPX, circleCompB.m_ray);
        if(collision)
        {
            if(args.tagCompA.m_tagA == CollisionTag_e::PLAYER_ACTION_CT)
            {
                treatActionPlayerCircle(args);
            }
            else if(args.tagCompA.m_tagA == CollisionTag_e::HIT_PLAYER_CT)
            {
                OptUint_t compNum = m_newComponentManager.getComponentEmplacement(args.entityNumA, Components_e::SHOT_CONF_COMPONENT);
                assert(compNum);
                ShotConfComponent &shotConfComp = m_componentsContainer.m_vectShotConfComp[*compNum];
                if(args.tagCompB.m_tagA == CollisionTag_e::ENEMY_CT)
                {
                    activeSound(args.entityNumA);
                    treatEnemyTakeDamage(args.entityNumB, shotConfComp.m_damage);
                    return false;
                }
            }
            else if(args.tagCompA.m_tagA == CollisionTag_e::PLAYER_CT)
            {
                if(args.tagCompB.m_tagA == CollisionTag_e::OBJECT_CT)
                {
                    treatPlayerPickObject(args);
                }
                else if(args.tagCompB.m_tagA == CollisionTag_e::TRIGGER_CT)
                {
                    OptUint_t compNum = m_newComponentManager.getComponentEmplacement(args.entityNumB, Components_e::TRIGGER_COMPONENT);
                    assert(compNum);
                    TriggerComponent &triggerComp = m_componentsContainer.m_vectTriggerComp[*compNum];
                    triggerComp.m_actionned = true;
                }
                else if(args.tagCompB.m_tagA == CollisionTag_e::TELEPORT_CT)
                {
                    treatPlayerTeleport(args);
                    m_memPlayerTeleport = true;
                }
            }
            else if(args.tagCompA.m_tagA == CollisionTag_e::EXPLOSION_CT)
            {
                treatExplosionColl(args);
            }
            if((args.tagCompA.m_tagA == CollisionTag_e::PLAYER_CT || args.tagCompA.m_tagA == CollisionTag_e::ENEMY_CT ||
                     args.tagCompA.m_tagB == CollisionTag_e::BARREL_CT || args.tagCompA.m_tagA == CollisionTag_e::IMPACT_CT) &&
                    (args.tagCompB.m_tagA == CollisionTag_e::LOG_CT || args.tagCompB.m_tagA == CollisionTag_e::WALL_CT ||
                     args.tagCompB.m_tagA == CollisionTag_e::PLAYER_CT ||
                     args.tagCompB.m_tagA == CollisionTag_e::ENEMY_CT || args.tagCompB.m_tagA == CollisionTag_e::STATIC_SET_CT ||
                     args.tagCompB.m_tagB == CollisionTag_e::BARREL_CT))
            {
                if(args.tagCompA.m_tagA == CollisionTag_e::PLAYER_CT &&
                        args.tagCompB.m_tagA == CollisionTag_e::ENEMY_CT &&
                        circleCompB.m_ray > 10.0f)
                {
                    if(m_memCrush.empty())
                    {
                        collisionCircleCircleEject(args, circleCompA, circleCompB);
                    }
                }
                else
                {
                    collisionCircleCircleEject(args, circleCompA, circleCompB);
                }
            }
        }
    }
        break;
    case CollisionShape_e::SEGMENT_C:
    {
    }
        break;
    }
    //TREAT VISIBLE SHOT
    if((args.tagCompA.m_tagA == CollisionTag_e::BULLET_ENEMY_CT) ||
            (args.tagCompA.m_tagA == CollisionTag_e::BULLET_PLAYER_CT))
    {
        OptUint_t compNum = m_newComponentManager.getComponentEmplacement(args.entityNumA, Components_e::SHOT_CONF_COMPONENT);
        assert(compNum);
        ShotConfComponent &shotConfComp = m_componentsContainer.m_vectShotConfComp[*compNum];
        bool limitX = (args.mapCompA.m_absoluteMapPositionPX.first < LEVEL_THIRD_TILE_SIZE_PX),
            limitY = (args.mapCompA.m_absoluteMapPositionPX.second < LEVEL_THIRD_TILE_SIZE_PX);
        //limit level case
        if(!shotConfComp.m_destructPhase && (limitX || limitY))
        {
            if(limitX)
            {
                args.mapCompA.m_absoluteMapPositionPX.first = LEVEL_THIRD_TILE_SIZE_PX;
            }
            if(limitY)
            {
                args.mapCompA.m_absoluteMapPositionPX.second = LEVEL_THIRD_TILE_SIZE_PX;
            }
            shotConfComp.m_destructPhase = true;
            if(shotConfComp.m_damageCircleRayData)
            {
                setDamageCircle(*shotConfComp.m_damageCircleRayData, true, args.entityNumA);
            }
            return true;
        }
        if(collision)
        {
            if(args.tagCompB.m_shape == CollisionShape_e::RECTANGLE_C)
            {
                if(shotExplosionEject)
                {
                    OptUint_t compNum = m_newComponentManager.getComponentEmplacement(args.entityNumB, Components_e::RECTANGLE_COLLISION_COMPONENT);
                    assert(compNum);
                    RectangleCollisionComponent &rectCompB = m_componentsContainer.m_vectRectangleCollisionComp[*compNum];
                    collisionCircleRectEject(args, circleCompA.m_ray, rectCompB, shotExplosionEject);
                }
                else if(!shotConfComp.m_ejectMode)
                {
                    shotConfComp.m_ejectMode = true;
                    std::swap(circleCompA.m_ray, shotConfComp.m_ejectExplosionRay);
                    return false;
                }
                else if(args.tagCompB.m_tagA == CollisionTag_e::WALL_CT)
                {
                    OptUint_t compNum = m_newComponentManager.getComponentEmplacement(args.entityNumB, Components_e::MOVEABLE_WALL_CONF_COMPONENT);
                    if(compNum)
                    {
                        OptUint_t compNum = m_newComponentManager.getComponentEmplacement(args.entityNumB, Components_e::RECTANGLE_COLLISION_COMPONENT);
                        assert(compNum);
                        RectangleCollisionComponent &rectCompB = m_componentsContainer.m_vectRectangleCollisionComp[*compNum];
                        collisionCircleRectEject(args, circleCompA.m_ray, rectCompB, shotExplosionEject);
                    }
                }
            }
            if(shotConfComp.m_destructPhase)
            {
                return true;
            }
            if(shotConfComp.m_damageCircleRayData)
            {
                setDamageCircle(*shotConfComp.m_damageCircleRayData, true, args.entityNumA);
            }
            activeSound(args.entityNumA);
            shotConfComp.m_destructPhase = true;
            shotConfComp.m_spriteShotNum = 0;
            if(shotConfComp.m_damageCircleRayData || circleDamageObstructed(args))
            {
                return true;
            }
            if(args.tagCompA.m_tagA == CollisionTag_e::BULLET_PLAYER_CT && args.tagCompB.m_tagA == CollisionTag_e::ENEMY_CT)
            {
                treatEnemyTakeDamage(args.entityNumB, shotConfComp.m_damage);
            }
            else if(args.tagCompA.m_tagA == CollisionTag_e::BULLET_ENEMY_CT && args.tagCompB.m_tagA == CollisionTag_e::PLAYER_CT)
            {
                OptUint_t compNumPlayer = m_newComponentManager.getComponentEmplacement(m_playerEntity, Components_e::PLAYER_CONF_COMPONENT);
                assert(compNumPlayer);
                PlayerConfComponent &playerComp = m_componentsContainer.m_vectPlayerConfComp[*compNumPlayer];
                playerComp.takeDamage(shotConfComp.m_damage);
            }
            else if(args.tagCompB.m_tagA == CollisionTag_e::BARREL_CT)
            {
                treatBarrelShots(args.entityNumB, shotConfComp.m_damage);
            }
        }
    }
    return true;
}

//===================================================================
bool CollisionSystem::treatCollisionPlayer(CollisionArgs &args, CircleCollisionComponent &circleCompA, RectangleCollisionComponent &rectCompB)
{
    if(args.tagCompB.m_tagA == CollisionTag_e::DOOR_CT)
    {
        OptUint_t compNum = m_newComponentManager.getComponentEmplacement(args.entityNumB, Components_e::DOOR_COMPONENT);
        assert(compNum);
        DoorComponent &doorComp = m_componentsContainer.m_vectDoorComp[*compNum];
        doorComp.m_obstruct = true;
        if(doorComp.m_currentState == DoorState_e::STATIC_OPEN)
        {
            return true;
        }
        if(doorComp.m_currentState == DoorState_e::MOVE_CLOSE)
        {
            doorComp.m_currentState = DoorState_e::MOVE_OPEN;
            activeSound(args.entityNumB);
            return true;
        }
    }
    else if(args.tagCompB.m_tagA == CollisionTag_e::CHECKPOINT_CT)
    {
        OptUint_t compNumPlayer = m_newComponentManager.getComponentEmplacement(m_playerEntity, Components_e::PLAYER_CONF_COMPONENT);
        assert(compNumPlayer);
        PlayerConfComponent &playerComp = m_componentsContainer.m_vectPlayerConfComp[*compNumPlayer];
        OptUint_t compNum = m_newComponentManager.getComponentEmplacement(args.entityNumB, Components_e::CHECKPOINT_COMPONENT);
        assert(compNum);
        CheckpointComponent &checkpointComp = m_componentsContainer.m_vectCheckpointComp[*compNum];
        if(!playerComp.m_currentCheckpoint || checkpointComp.m_checkpointNumber > playerComp.m_currentCheckpoint->first)
        {
            OptUint_t compNum = m_newComponentManager.getComponentEmplacement(args.entityNumB, Components_e::MAP_COORD_COMPONENT);
            assert(compNum);
            MapCoordComponent &mapComp = m_componentsContainer.m_vectMapCoordComp[*compNum];

            playerComp.m_checkpointReached = mapComp.m_coord;
            playerComp.m_currentCheckpoint = {checkpointComp.m_checkpointNumber, checkpointComp.m_direction};
            writePlayerInfo("Checkpoint Reached");
        }
        m_vectEntitiesToDelete.push_back(args.entityNumB);
        return true;
    }
    else if(args.tagCompB.m_tagA == CollisionTag_e::SECRET_CT)
    {
        OptUint_t compNumPlayer = m_newComponentManager.getComponentEmplacement(m_playerEntity, Components_e::PLAYER_CONF_COMPONENT);
        assert(compNumPlayer);
        PlayerConfComponent &playerComp = m_componentsContainer.m_vectPlayerConfComp[*compNumPlayer];
        writePlayerInfo("Secret Found");
        if(!playerComp.m_secretsFound)
        {
            playerComp.m_secretsFound = 1;
        }
        else
        {
            ++(*playerComp.m_secretsFound);
        }
        m_vectEntitiesToDelete.push_back(args.entityNumB);
        return true;
    }
    collisionCircleRectEject(args, circleCompA.m_ray, rectCompB);
    return false;
}

//===================================================================
void CollisionSystem::treatExplosionColl(CollisionArgs &args)
{
    if(circleDamageObstructed(args))
    {
        return;
    }
    OptUint_t compNum = m_newComponentManager.getComponentEmplacement(args.entityNumA, Components_e::SHOT_CONF_COMPONENT);
    assert(compNum);
    ShotConfComponent &shotConfComp = m_componentsContainer.m_vectShotConfComp[*compNum];
    compNum = m_newComponentManager.getComponentEmplacement(args.entityNumB, Components_e::MOVEABLE_COMPONENT);
    assert(compNum);
    if(compNum && !(args.tagCompB.m_tagA == CollisionTag_e::BARREL_CT &&
            args.tagCompB.m_tagB != CollisionTag_e::BARREL_CT))
    {
        MoveableComponent &moveComp = m_componentsContainer.m_vectMoveableComp[*compNum];
        OptUint_t compNum = m_newComponentManager.getComponentEmplacement(args.entityNumB, Components_e::TIMER_COMPONENT);
        assert(compNum);
        TimerComponent &timerComp = m_componentsContainer.m_vectTimerComp[*compNum];
        timerComp.m_cycleCountD = 0;
        if(!moveComp.m_ejectData && (std::abs(args.mapCompA.m_absoluteMapPositionPX.first - args.mapCompB.m_absoluteMapPositionPX.first) > 0.01f ||
                std::abs(args.mapCompA.m_absoluteMapPositionPX.second - args.mapCompB.m_absoluteMapPositionPX.second) > 0.01f))
        {
            moveComp.m_currentDegreeMoveDirection = getTrigoAngle(args.mapCompA.m_absoluteMapPositionPX, args.mapCompB.m_absoluteMapPositionPX);
            moveComp.m_ejectData = {1.5f, EJECT_CYCLE_TIME};
        }
    }
    if(args.tagCompB.m_tagA == CollisionTag_e::PLAYER_CT)
    {
        OptUint_t compNumPlayer = m_newComponentManager.getComponentEmplacement(m_playerEntity, Components_e::PLAYER_CONF_COMPONENT);
        assert(compNumPlayer);
        PlayerConfComponent &playerComp = m_componentsContainer.m_vectPlayerConfComp[*compNumPlayer];
        playerComp.takeDamage(shotConfComp.m_damage);
    }
    else if(args.tagCompB.m_tagA == CollisionTag_e::ENEMY_CT)
    {
        treatEnemyTakeDamage(args.entityNumB, shotConfComp.m_damage);
    }
    else if(args.tagCompB.m_tagA == CollisionTag_e::BARREL_CT)
    {
        treatBarrelShots(args.entityNumB, shotConfComp.m_damage);
    }
}

//===================================================================
bool CollisionSystem::circleDamageObstructed(const CollisionArgs &args)const
{
    float radiantAngle = getTrigoAngle(args.mapCompA.m_absoluteMapPositionPX, args.mapCompB.m_absoluteMapPositionPX, false);
    optionalTargetRaycast_t result = mptrSystemManager->searchSystemByType<FirstPersonDisplaySystem>(
                static_cast<uint32_t>(Systems_e::FIRST_PERSON_DISPLAY_SYSTEM))->
            calcLineSegmentRaycast(radiantAngle, args.mapCompA.m_absoluteMapPositionPX, false);
    if(result && std::get<1>(*result) > 0.1f && getDistance(std::get<0>(*result), args.mapCompB.m_absoluteMapPositionPX) <
            getDistance(args.mapCompA.m_absoluteMapPositionPX, args.mapCompB.m_absoluteMapPositionPX))
    {
        return true;
    }
    return false;
}

//===================================================================
void CollisionSystem::setDamageCircle(uint32_t damageEntity, bool active, uint32_t baseEntity)
{
    OptUint_t compNum = m_newComponentManager.getComponentEmplacement(damageEntity, Components_e::GENERAL_COLLISION_COMPONENT);
    assert(compNum);
    GeneralCollisionComponent &genDam = m_componentsContainer.m_vectGeneralCollisionComp[*compNum];
    genDam.m_active = active;
    if(active)
    {
        OptUint_t compNum = m_newComponentManager.getComponentEmplacement(damageEntity, Components_e::MAP_COORD_COMPONENT);
        assert(compNum);
        MapCoordComponent &mapCompDam = m_componentsContainer.m_vectMapCoordComp[*compNum];

        compNum = m_newComponentManager.getComponentEmplacement(baseEntity, Components_e::MAP_COORD_COMPONENT);
        assert(compNum);
        MapCoordComponent &mapComp = m_componentsContainer.m_vectMapCoordComp[*compNum];
        mapCompDam.m_absoluteMapPositionPX = mapComp.m_absoluteMapPositionPX;
        std::optional<PairUI_t> coord = getLevelCoord(mapCompDam.m_absoluteMapPositionPX);
        assert(coord);
        addEntityToZone(damageEntity, *getLevelCoord(mapCompDam.m_absoluteMapPositionPX));
    }
    else
    {
        removeEntityToZone(damageEntity);
    }
}

//===================================================================
void CollisionSystem::treatActionPlayerRect(CollisionArgs &args)
{
    if(args.tagCompB.m_tagA == CollisionTag_e::DOOR_CT)
    {
        OptUint_t compNum = m_newComponentManager.getComponentEmplacement(args.entityNumB, Components_e::DOOR_COMPONENT);
        assert(compNum);
        DoorComponent &doorComp = m_componentsContainer.m_vectDoorComp[*compNum];
        //if card door
        if(doorComp.m_cardID)
        {
            OptUint_t compNumPlayer = m_newComponentManager.getComponentEmplacement(m_playerEntity, Components_e::PLAYER_CONF_COMPONENT);
            assert(compNumPlayer);
            PlayerConfComponent &playerComp = m_componentsContainer.m_vectPlayerConfComp[*compNumPlayer];
            if((playerComp.m_card.find((*doorComp.m_cardID).first) ==
                 playerComp.m_card.end()))
            {
                writePlayerInfo((*doorComp.m_cardID).second + " NEEDED");
                return;
            }
            else
            {
                writePlayerInfo((*doorComp.m_cardID).second + " USED");
            }
        }
        if(doorComp.m_currentState == DoorState_e::STATIC_CLOSED || doorComp.m_currentState == DoorState_e::MOVE_CLOSE)
        {
            doorComp.m_currentState = DoorState_e::MOVE_OPEN;
            activeSound(args.entityNumB);
        }
    }
    else if(args.tagCompB.m_tagB == CollisionTag_e::TRIGGER_CT)
    {
        //TREAT MOVEABLE WALL
        if(args.tagCompB.m_tagA == CollisionTag_e::WALL_CT)
        {
            OptUint_t compNum = m_newComponentManager.getComponentEmplacement(args.entityNumB, Components_e::MOVEABLE_WALL_CONF_COMPONENT);
            assert(compNum);
            MoveableWallConfComponent &moveWallComp = m_componentsContainer.m_vectMoveableWallConfComp[*compNum];
            if(!moveWallComp.m_inMovement)
            {
                moveWallComp.m_manualTrigger = true;
                ++moveWallComp.m_triggerWallCheckpointData->second.m_numberOfMove;
                m_refMainEngine->updateTriggerWallMoveableWallDataCheckpoint(*moveWallComp.m_triggerWallCheckpointData);
            }
            assert(moveWallComp.m_triggerWallCheckpointData);
        }
    }
}

//===================================================================
void CollisionSystem::activeSound(uint32_t entityNum)
{
    OptUint_t compNum = m_newComponentManager.getComponentEmplacement(entityNum, Components_e::AUDIO_COMPONENT);
    assert(compNum);
    AudioComponent &audioComp = m_componentsContainer.m_vectAudioComp[*compNum];
    audioComp.m_soundElements[0]->m_toPlay = true;
}

//===================================================================
void CollisionSystem::treatActionPlayerCircle(CollisionArgs &args)
{
    if(args.tagCompB.m_tagA == CollisionTag_e::EXIT_CT)
    {
        m_refMainEngine->activeEndLevel();
    }
    else if(args.tagCompB.m_tagA == CollisionTag_e::LOG_CT)
    {
        OptUint_t compNumPlayer = m_newComponentManager.getComponentEmplacement(m_playerEntity, Components_e::PLAYER_CONF_COMPONENT);
        assert(compNumPlayer);
        PlayerConfComponent &playerComp = m_componentsContainer.m_vectPlayerConfComp[*compNumPlayer];
        OptUint_t compNum = m_newComponentManager.getComponentEmplacement(args.entityNumB, Components_e::LOG_COMPONENT);
        assert(compNum);
        LogComponent &logComp = m_componentsContainer.m_vectLogComp[*compNum];
        playerComp.m_infoWriteData = {true, logComp.m_message};
        compNum = m_newComponentManager.getComponentEmplacement(playerComp.muiGetIdEntityAssociated(), Components_e::TIMER_COMPONENT);
        assert(compNum);
        TimerComponent &timerComp = m_componentsContainer.m_vectTimerComp[*compNum];
        timerComp.m_cycleCountA = 0;
        timerComp.m_timeIntervalOptional = 4.0 / FPS_VALUE;
    }
    else if(args.tagCompB.m_tagB == CollisionTag_e::TRIGGER_CT)
    {
        OptUint_t compNum = m_newComponentManager.getComponentEmplacement(args.entityNumB, Components_e::TRIGGER_COMPONENT);
        assert(compNum);
        TriggerComponent &triggerComp = m_componentsContainer.m_vectTriggerComp[*compNum];
        triggerComp.m_actionned = true;
    }
}

//===================================================================
void CollisionSystem::treatPlayerPickObject(CollisionArgs &args)
{
    OptUint_t compNum = m_newComponentManager.getComponentEmplacement(args.entityNumB, Components_e::OBJECT_CONF_COMPONENT);
    assert(compNum);
    ObjectConfComponent &objectComp = m_componentsContainer.m_vectObjectConfComp[*compNum];
    OptUint_t compNumPlayer = m_newComponentManager.getComponentEmplacement(m_playerEntity, Components_e::PLAYER_CONF_COMPONENT);
    assert(compNumPlayer);
    PlayerConfComponent &playerComp = m_componentsContainer.m_vectPlayerConfComp[*compNumPlayer];
    compNum = m_newComponentManager.getComponentEmplacement(
        playerComp.m_vectEntities[static_cast<uint32_t>(PlayerEntities_e::WEAPON)],
        Components_e::WEAPON_COMPONENT);
    assert(compNum);
    WeaponComponent &weaponComp = m_componentsContainer.m_vectWeaponComp[*compNum];
    std::string info;
    switch (objectComp.m_type)
    {
    case ObjectType_e::AMMO_WEAPON:
    {
        if(!pickUpAmmo(*objectComp.m_weaponID, weaponComp, objectComp.m_containing))
        {
            return;
        }
        info = weaponComp.m_weaponsData[*objectComp.m_weaponID].m_weaponName + " Ammo";
    }
        break;
    case ObjectType_e::WEAPON:
    {
        if(!pickUpWeapon(*objectComp.m_weaponID, weaponComp, objectComp.m_containing))
        {
            return;
        }
        info = weaponComp.m_weaponsData[*objectComp.m_weaponID].m_weaponName;
    }
        break;
    case ObjectType_e::HEAL:
    {
        if(playerComp.m_life == 100)
        {
            return;
        }
        playerComp.m_life += objectComp.m_containing;
        if(playerComp.m_life > 100)
        {
            playerComp.m_life = 100;
        }
        info = "Heal";
        break;
    }
    case ObjectType_e::CARD:
    {
        playerComp.m_card.insert(*objectComp.m_cardID);
        info = objectComp.m_cardName;
        break;
    }
    case ObjectType_e::TOTAL:
        assert(false);
        break;
    }
    removeEntityToZone(args.entityNumB);
    playerComp.m_infoWriteData = {true, info};
    compNum = m_newComponentManager.getComponentEmplacement(playerComp.muiGetIdEntityAssociated(), Components_e::TIMER_COMPONENT);
    assert(compNum);
    TimerComponent &timerComp = m_componentsContainer.m_vectTimerComp[*compNum];
    timerComp.m_cycleCountA = 0;
    playerComp.m_pickItem = true;
    activeSound(args.entityNumA);
    m_vectEntitiesToDelete.push_back(args.entityNumB);
}

//===================================================================
void CollisionSystem::treatPlayerTeleport(CollisionArgs &args)
{
    OptUint_t compNumPlayer = m_newComponentManager.getComponentEmplacement(m_playerEntity, Components_e::PLAYER_CONF_COMPONENT);
    assert(compNumPlayer);
    PlayerConfComponent &playerComp = m_componentsContainer.m_vectPlayerConfComp[*compNumPlayer];
    if(playerComp.m_teleported)
    {
        return;
    }
    OptUint_t compNum = m_newComponentManager.getComponentEmplacement(args.entityNumA, Components_e::MAP_COORD_COMPONENT);
    assert(compNum);
    MapCoordComponent &mapPlayerComp = m_componentsContainer.m_vectMapCoordComp[*compNum];
    compNum = m_newComponentManager.getComponentEmplacement(args.entityNumB, Components_e::TELEPORT_COMPONENT);
    assert(compNum);
    TeleportComponent &teleportComp = m_componentsContainer.m_vectTeleportComp[*compNum];
    compNum = m_newComponentManager.getComponentEmplacement(
        playerComp.m_vectEntities[static_cast<uint32_t>(PlayerEntities_e::DISPLAY_TELEPORT)],
        Components_e::GENERAL_COLLISION_COMPONENT);
    assert(compNum);
    GeneralCollisionComponent &genTeleportComp = m_componentsContainer.m_vectGeneralCollisionComp[*compNum];

    compNum = m_newComponentManager.getComponentEmplacement(
        playerComp.m_vectEntities[static_cast<uint32_t>(PlayerEntities_e::DISPLAY_TELEPORT)], Components_e::TIMER_COMPONENT);
    assert(compNum);
    TimerComponent &timerComp = m_componentsContainer.m_vectTimerComp[*compNum];
    activeSound(args.entityNumB);
    mapPlayerComp.m_coord = teleportComp.m_targetPos;
    mapPlayerComp.m_absoluteMapPositionPX = getCenteredAbsolutePosition(mapPlayerComp.m_coord);
    playerComp.m_teleported = true;
    genTeleportComp.m_active = true;
    timerComp.m_cycleCountA = 0;
    timerComp.m_cycleCountB = 0;
}

//===================================================================
void CollisionSystem::treatCrushing(uint32_t entityNum)
{
    OptUint_t compNum = m_newComponentManager.getComponentEmplacement(entityNum, Components_e::PLAYER_CONF_COMPONENT);
    if(compNum)
    {
        OptUint_t compNumPlayer = m_newComponentManager.getComponentEmplacement(m_playerEntity, Components_e::PLAYER_CONF_COMPONENT);
        assert(compNumPlayer);
        PlayerConfComponent &playerComp = m_componentsContainer.m_vectPlayerConfComp[*compNumPlayer];
        playerComp.m_crush = true;
        playerComp.m_frozen = true;
    }
    else
    {
        //check if component exist
        if(m_newComponentManager.getComponentEmplacement(entityNum, Components_e::ENEMY_CONF_COMPONENT))
        {
            treatEnemyTakeDamage(entityNum, 1);
        }
    }
}

//===================================================================
bool pickUpAmmo(uint32_t numWeapon, WeaponComponent &weaponComp, uint32_t objectContaining)
{
    WeaponData &objectWeapon = weaponComp.m_weaponsData[numWeapon];
    if(objectWeapon.m_ammunationsCount == objectWeapon.m_maxAmmunations)
    {
        return false;
    }
    if(objectWeapon.m_posses && objectWeapon.m_ammunationsCount == 0)
    {
        if(weaponComp.m_currentWeapon < numWeapon)
        {
            setPlayerWeapon(weaponComp, numWeapon);
        }
    }
    objectWeapon.m_ammunationsCount += objectContaining;
    if(objectWeapon.m_ammunationsCount > objectWeapon.m_maxAmmunations)
    {
        objectWeapon.m_ammunationsCount = objectWeapon.m_maxAmmunations;
    }
    return true;
}

//===================================================================
bool pickUpWeapon(uint32_t numWeapon, WeaponComponent &weaponComp, uint32_t objectContaining)
{
    WeaponData &objectWeapon = weaponComp.m_weaponsData[numWeapon];
    if(objectWeapon.m_posses &&
            objectWeapon.m_ammunationsCount == objectWeapon.m_maxAmmunations)
    {
        return false;
    }
    if(!objectWeapon.m_posses)
    {
        objectWeapon.m_posses = true;
        if(weaponComp.m_currentWeapon < numWeapon)
        {
            setPlayerWeapon(weaponComp, numWeapon);
        }
    }
    objectWeapon.m_ammunationsCount += objectContaining;
    if(objectWeapon.m_ammunationsCount > objectWeapon.m_maxAmmunations)
    {
        objectWeapon.m_ammunationsCount = objectWeapon.m_maxAmmunations;
    }
    return true;
}

//===================================================================
void CollisionSystem::checkCollisionFirstSegment(uint32_t numEntityA, uint32_t numEntityB,
                                                 GeneralCollisionComponent &tagCompB,
                                                 MapCoordComponent &mapCompB)
{
    OptUint_t compNum = m_newComponentManager.getComponentEmplacement(numEntityA, Components_e::SEGMENT_COLLISION_COMPONENT);
    assert(compNum);
    SegmentCollisionComponent &segmentCompA = m_componentsContainer.m_vectSegmentCollisionComp[*compNum];
    switch(tagCompB.m_shape)
    {
    case CollisionShape_e::RECTANGLE_C:
    case CollisionShape_e::SEGMENT_C:
    {
    }
        break;
    case CollisionShape_e::CIRCLE_C:
    {
        compNum = m_newComponentManager.getComponentEmplacement(numEntityB, Components_e::CIRCLE_COLLISION_COMPONENT);
        assert(compNum);
        CircleCollisionComponent &circleCompB = m_componentsContainer.m_vectCircleCollisionComp[*compNum];
        if(checkCircleSegmentCollision(mapCompB.m_absoluteMapPositionPX, circleCompB.m_ray,
                                       segmentCompA.m_points.first,
                                       segmentCompA.m_points.second))
        {
            //Fix impact displayed behind element
            float distance = getDistance(segmentCompA.m_points.first,
                                         mapCompB.m_absoluteMapPositionPX) - 5.0f;
            if(m_memDistCurrentBulletColl.second <= EPSILON_FLOAT ||
                    distance < m_memDistCurrentBulletColl.second)
            {
                m_memDistCurrentBulletColl = {numEntityB, distance};
            }
        }
    }
        break;
    }
}

//===================================================================
void CollisionSystem::calcBulletSegment(SegmentCollisionComponent &segmentCompA)
{
    float radiantAngle = getRadiantAngle(segmentCompA.m_degreeOrientation);
    optionalTargetRaycast_t result = mptrSystemManager->searchSystemByType<FirstPersonDisplaySystem>(
                static_cast<uint32_t>(Systems_e::FIRST_PERSON_DISPLAY_SYSTEM))->
            calcLineSegmentRaycast(radiantAngle, segmentCompA.m_points.first, false);
    segmentCompA.m_points.second = std::get<0>(*result);
    m_memDistCurrentBulletColl = {std::get<2>(*result), 10000.0f};
}

//===================================================================
void CollisionSystem::collisionCircleRectEject(CollisionArgs &args, float circleRay,
                                               const RectangleCollisionComponent &rectCollB, bool visibleShotFirstEject)
{
    OptUint_t compNum = m_newComponentManager.getComponentEmplacement(args.entityNumA, Components_e::MAP_COORD_COMPONENT);
    assert(compNum);
    MapCoordComponent &mapComp = m_componentsContainer.m_vectMapCoordComp[*compNum];
    compNum = m_newComponentManager.getComponentEmplacement(args.entityNumA, Components_e::MOVEABLE_COMPONENT);
    assert(compNum);
    MoveableComponent &moveComp = m_componentsContainer.m_vectMoveableComp[*compNum];
    float radiantEjectedAngle = getRadiantAngle(moveComp.m_currentDegreeMoveDirection);
    float circlePosX = args.mapCompA.m_absoluteMapPositionPX.first;
    float circlePosY = args.mapCompA.m_absoluteMapPositionPX.second;
    float elementPosX = args.mapCompB.m_absoluteMapPositionPX.first;
    float elementPosY = args.mapCompB.m_absoluteMapPositionPX.second;
    float elementSecondPosX = elementPosX + rectCollB.m_size.first;
    float elementSecondPosY = elementPosY + rectCollB.m_size.second;
    bool angleBehavior = false, limitEjectY = false, limitEjectX = false, crushMode = false;
    //collision on angle of rect
    if((circlePosX < elementPosX || circlePosX > elementSecondPosX) &&
            (circlePosY < elementPosY || circlePosY > elementSecondPosY))
    {
        angleBehavior = true;
    }
    float pointElementX = (circlePosX < elementPosX) ? elementPosX : elementSecondPosX;
    float pointElementY = (circlePosY < elementPosY) ? elementPosY : elementSecondPosY;
    float diffY, diffX = EPSILON_FLOAT;
    bool visibleShot = (args.tagCompA.m_tagA == CollisionTag_e::BULLET_ENEMY_CT || args.tagCompA.m_tagA == CollisionTag_e::BULLET_PLAYER_CT);
    diffY = getVerticalCircleRectEject({circlePosX, circlePosY, pointElementX, elementPosY,
                                        elementSecondPosY, circleRay, radiantEjectedAngle, angleBehavior}, limitEjectY, visibleShot);
    diffX = getHorizontalCircleRectEject({circlePosX, circlePosY, pointElementY, elementPosX, elementSecondPosX,
                                          circleRay, radiantEjectedAngle, angleBehavior}, limitEjectX, visibleShot);
    if(!visibleShotFirstEject && args.tagCompA.m_tagA != CollisionTag_e::PLAYER_CT &&
            std::min(std::abs(diffX), std::abs(diffY)) > LEVEL_THIRD_TILE_SIZE_PX)
    {
        return;
    }
    if(args.tagCompA.m_tagA == CollisionTag_e::PLAYER_CT || args.tagCompA.m_tagA == CollisionTag_e::ENEMY_CT)
    {
        crushMode = args.tagCompB.m_tagA == CollisionTag_e::WALL_CT;
    }
    collisionEject(mapComp, diffX, diffY, limitEjectY, limitEjectX, crushMode);
    addEntityToZone(args.entityNumA, *getLevelCoord(mapComp.m_absoluteMapPositionPX));
    if(crushMode)
    {
        std::get<1>(m_memCrush.back()) = angleBehavior;
        std::get<2>(m_memCrush.back()) = getDirection(diffX, diffY);
        OptUint_t compNum = m_newComponentManager.getComponentEmplacement(args.entityNumB, Components_e::MOVEABLE_WALL_CONF_COMPONENT);
        if(compNum)
        {
            MoveableWallConfComponent &moveWallComp = m_componentsContainer.m_vectMoveableWallConfComp[*compNum];
            if(moveWallComp.m_inMovement)
            {
                std::get<3>(m_memCrush.back()) = moveWallComp.m_directionMove[moveWallComp.m_currentMove].first;
            }
        }
    }
}

//===================================================================
float CollisionSystem::getVerticalCircleRectEject(const EjectYArgs& args, bool &limitEject, bool visibleShot)
{
    float adj, diffYA = EPSILON_FLOAT, diffYB;
    if(std::abs(std::sin(args.radiantAngle)) < 0.01f &&
            (args.angleMode || args.circlePosY < args.elementPosY || args.circlePosY > args.elementSecondPosY))
    {
        float distUpPoint = std::abs(args.circlePosY - args.elementPosY),
                distDownPoint = std::abs(args.circlePosY - args.elementSecondPosY);
        limitEject = true;
        if(distUpPoint < distDownPoint)
        {
            --diffYA;
        }
        else
        {
            ++diffYA;
        }
        if(limitEject)
        {
            return diffYA;
        }
    }
    if(args.angleMode)
    {
        adj = std::abs(args.circlePosX - args.elementPosX);
        //args.elementPosX == 150 ??
        diffYA = getRectTriangleSide(adj, args.ray);
        //EJECT UP
        if(args.circlePosY < args.elementPosY ||
                std::abs(args.circlePosY - args.elementPosY) < std::abs(args.circlePosY - args.elementSecondPosY))
        {
            diffYA -= (args.elementPosY - args.circlePosY);
            if(diffYA > EPSILON_FLOAT)
            {
                diffYA = -diffYA;
            }
        }
        //EJECT DOWN
        else
        {
            diffYA -= (args.circlePosY - args.elementSecondPosY);
            if(diffYA < EPSILON_FLOAT)
            {
                diffYA = -diffYA;
            }
        }
    }
    else
    {
        diffYA = args.elementPosY - (args.circlePosY + args.ray);
        diffYB = args.elementSecondPosY - (args.circlePosY - args.ray);
        if(visibleShot)
        {
            diffYA = (std::sin(args.radiantAngle) < EPSILON_FLOAT) ? diffYA : diffYB;
        }
        else
        {
            diffYA = (std::abs(diffYA) < std::abs(diffYB)) ? diffYA : diffYB;
        }
    }
    return diffYA;
}

//===================================================================
float CollisionSystem::getHorizontalCircleRectEject(const EjectXArgs &args, bool &limitEject, bool visibleShot)
{
    float adj, diffXA = EPSILON_FLOAT, diffXB;
    if(std::abs(std::cos(args.radiantAngle)) < 0.01f && (args.angleMode || args.circlePosX < args.elementPosX ||
             args.circlePosX > args.elementSecondPosX))
    {
        float distLeftPoint = std::abs(args.circlePosX - args.elementPosX),
                distRightPoint = std::abs(args.circlePosX - args.elementSecondPosX);
        limitEject = true;
        if(distLeftPoint < distRightPoint)
        {
            --diffXA;
        }
        else
        {
            ++diffXA;
        }
        if(limitEject)
        {
            return diffXA;
        }
    }
    if(args.angleMode)
    {
        adj = std::abs(args.circlePosY - args.elementPosY);
        diffXA = getRectTriangleSide(adj, args.ray);
        //EJECT LEFT
        if(args.circlePosX < args.elementPosX ||
                std::abs(args.circlePosX - args.elementPosX) < std::abs(args.circlePosX - args.elementSecondPosX))
        {
            diffXA -= std::abs(args.elementPosX - args.circlePosX);
            if(diffXA > EPSILON_FLOAT)
            {
                diffXA = -diffXA;
            }
        }
        //EJECT RIGHT
        else
        {
            diffXA -= (args.circlePosX - args.elementSecondPosX);
            if(diffXA < EPSILON_FLOAT)
            {
                diffXA = -diffXA;
            }
        }
    }
    else
    {
        diffXA = args.elementPosX - (args.circlePosX + args.ray);
        diffXB = args.elementSecondPosX - (args.circlePosX - args.ray);
        if(visibleShot)
        {
            diffXA = (std::cos(args.radiantAngle) > EPSILON_FLOAT) ? diffXA : diffXB;
        }
        else
        {
            diffXA = (std::abs(diffXA) < std::abs(diffXB)) ? diffXA : diffXB;
        }
    }
    return diffXA;
}

//===================================================================
void CollisionSystem::collisionEject(MapCoordComponent &mapComp, float diffX, float diffY, bool limitEjectY, bool limitEjectX, bool crushCase)
{
    float minEject = std::min(std::abs(diffY), std::abs(diffX));
    if(minEject >= LEVEL_TILE_SIZE_PX)
    {
        return;
    }
    if(crushCase)
    {
        m_memCrush.push_back({{EPSILON_FLOAT, EPSILON_FLOAT}, false, {}, {}});
    }
    if(!limitEjectX && (limitEjectY || std::abs(diffY) < std::abs(diffX)))
    {
        if(crushCase)
        {
            std::get<0>(m_memCrush.back()).second = diffY;
        }
        mapComp.m_absoluteMapPositionPX.second += diffY;
    }
    if(!limitEjectY && (limitEjectX || std::abs(diffY) > std::abs(diffX)))
    {
        if(crushCase)
        {
            std::get<0>(m_memCrush.back()).first = diffX;
        }
        mapComp.m_absoluteMapPositionPX.first += diffX;
    }
}

//===================================================================
void CollisionSystem::collisionCircleCircleEject(CollisionArgs &args,
                                                 const CircleCollisionComponent &circleCollA,
                                                 const CircleCollisionComponent &circleCollB)
{
    float circleAPosX = args.mapCompA.m_absoluteMapPositionPX.first;
    float circleAPosY = args.mapCompA.m_absoluteMapPositionPX.second;
    float circleBPosX = args.mapCompB.m_absoluteMapPositionPX.first;
    float circleBPosY = args.mapCompB.m_absoluteMapPositionPX.second;
    float distanceX = std::abs(circleAPosX - circleBPosX);
    float distanceY = std::abs(circleAPosY - circleBPosY);
    float hyp = circleCollA.m_ray + circleCollB.m_ray;
    float diffX = getRectTriangleSide(distanceY, hyp);
    float diffY = getRectTriangleSide(distanceX, hyp);
    diffX -= distanceX;
    diffY -= distanceY;
    if(circleAPosX < circleBPosX)
    {
        diffX = -diffX;
    }
    if(circleAPosY < circleBPosY)
    {
        diffY = -diffY;
    }
    collisionEject(args.mapCompA, diffX, diffY);
}

//===================================================================
Direction_e getDirection(float diffX, float diffY)
{
    bool vert = (std::abs(diffX) > std::abs(diffY));
    return vert ? ((diffY < EPSILON_FLOAT) ? Direction_e::NORTH : Direction_e::SOUTH) :
                  ((diffX < EPSILON_FLOAT) ? Direction_e::WEST : Direction_e::EAST);
}

//===================================================================
bool opposingDirection(Direction_e dirA, Direction_e dirB)
{
    std::bitset<4> bitset;
    bitset[static_cast<uint32_t>(dirA)] = true;
    bitset[static_cast<uint32_t>(dirB)] = true;
    return (bitset[static_cast<uint32_t>(Direction_e::EAST)] && bitset[static_cast<uint32_t>(Direction_e::WEST)]) ||
            (bitset[static_cast<uint32_t>(Direction_e::NORTH)] && bitset[static_cast<uint32_t>(Direction_e::SOUTH)]);
}
