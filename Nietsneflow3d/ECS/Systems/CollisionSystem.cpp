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
#include <ECS/Components/ImpactShotComponent.hpp>
#include <ECS/Components/MemSpriteDataComponent.hpp>
#include <ECS/Components/AudioComponent.hpp>
#include <ECS/Components/SpriteTextureComponent.hpp>
#include <ECS/Components/WeaponComponent.hpp>
#include <ECS/Components/BarrelComponent.hpp>
#include <ECS/Components/TriggerComponent.hpp>
#include <ECS/Components/MoveableWallConfComponent.hpp>
#include <ECS/Systems/FirstPersonDisplaySystem.hpp>
#include <ECS/Systems/ColorDisplaySystem.hpp>
#include <BaseECS/engine.hpp>
#include <CollisionUtils.hpp>
#include <PhysicalEngine.hpp>
#include <cassert>
#include <Level.hpp>
#include <math.h>

using multiMapTagIt_t = std::multimap<CollisionTag_e, CollisionTag_e>::const_iterator;

//===================================================================
CollisionSystem::CollisionSystem()
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
    SegmentCollisionComponent *segmentCompA;
    System::execSystem();
    m_pair = !m_pair;
    m_memPlayerTeleport = false;
    for(uint32_t i = 0; i < mVectNumEntity.size(); ++i)
    {
        GeneralCollisionComponent *tagCompA = stairwayToComponentManager().
                searchComponentByType<GeneralCollisionComponent>(mVectNumEntity[i],
                                                                 Components_e::GENERAL_COLLISION_COMPONENT);
        assert(tagCompA);
        if(!tagCompA->m_active || tagCompA->m_tagA == CollisionTag_e::WALL_CT || tagCompA->m_tagA == CollisionTag_e::OBJECT_CT ||
                tagCompA->m_tagA == CollisionTag_e::DOOR_CT)
        {
            continue;
        }
        if(tagCompA->m_shape == CollisionShape_e::SEGMENT_C &&
                (tagCompA->m_tagA == CollisionTag_e::BULLET_PLAYER_CT || tagCompA->m_tagA == CollisionTag_e::BULLET_ENEMY_CT))
        {
            m_memDistCurrentBulletColl.second = EPSILON_FLOAT;
            segmentCompA = stairwayToComponentManager().
                    searchComponentByType<SegmentCollisionComponent>(mVectNumEntity[i], Components_e::SEGMENT_COLLISION_COMPONENT);
            assert(segmentCompA);
            calcBulletSegment(*segmentCompA);
            tagCompA->m_active = false;
        }
        else
        {
            segmentCompA = nullptr;
        }
        if(m_pair && tagCompA->m_tagA == CollisionTag_e::PLAYER_CT)
        {
            PlayerConfComponent *playerComp = stairwayToComponentManager().
                    searchComponentByType<PlayerConfComponent>(mVectNumEntity[i], Components_e::PLAYER_CONF_COMPONENT);
            assert(playerComp);
            std::get<0>(playerComp->m_crushMem) = false;
            if(!playerComp->m_crush)
            {
                playerComp->m_frozen = false;
            }
            else
            {
                playerComp->m_crush = false;
            }
        }
        else if(tagCompA->m_tagA == CollisionTag_e::ENEMY_CT)
        {
            if(checkEnemyRemoveCollisionMask(mVectNumEntity[i]))
            {
                rmCollisionMaskEntity(mVectNumEntity[i]);
            }
        }
        else if(tagCompA->m_shape == CollisionShape_e::CIRCLE_C &&
                (tagCompA->m_tagA == CollisionTag_e::BULLET_ENEMY_CT || tagCompA->m_tagA == CollisionTag_e::BULLET_PLAYER_CT))
        {
            ShotConfComponent *shotConfComp = stairwayToComponentManager().
                    searchComponentByType<ShotConfComponent>(mVectNumEntity[i], Components_e::SHOT_CONF_COMPONENT);
            assert(shotConfComp);
            shotConfComp->m_currentLoopEjected = false;
        }
        for(uint32_t j = 0; j < mVectNumEntity.size(); ++j)
        {
            if(i == j)
            {
                continue;
            }
            GeneralCollisionComponent *tagCompB = stairwayToComponentManager().
                    searchComponentByType<GeneralCollisionComponent>(mVectNumEntity[j], Components_e::GENERAL_COLLISION_COMPONENT);
            assert(tagCompB);
            if(!tagCompB->m_active)
            {
                continue;
            }
            if(!checkTag(tagCompA->m_tagA, tagCompB->m_tagA) &&
                    !checkTag(tagCompA->m_tagA, tagCompB->m_tagB))
            {
                continue;
            }
            treatCollision(mVectNumEntity[i], mVectNumEntity[j], tagCompA, tagCompB);
        }
        if(tagCompA->m_tagA == CollisionTag_e::EXPLOSION_CT)
        {
            setDamageCircle(mVectNumEntity[i], false);
        }
        if(tagCompA->m_tagA == CollisionTag_e::PLAYER_ACTION_CT)
        {
            tagCompA->m_active = false;
        }
        if(segmentCompA && m_memDistCurrentBulletColl.second > EPSILON_FLOAT)
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
        m_playerComp->m_teleported = false;
    }
}

//===================================================================
bool CollisionSystem::checkEnemyRemoveCollisionMask(uint32_t entityNum)
{
    EnemyConfComponent *enemyConfComp = stairwayToComponentManager().
            searchComponentByType<EnemyConfComponent>(entityNum, Components_e::ENEMY_CONF_COMPONENT);
    assert(enemyConfComp);
    if(enemyConfComp->m_displayMode == EnemyDisplayMode_e::DEAD)
    {
        MoveableComponent *moveComp = stairwayToComponentManager().
                searchComponentByType<MoveableComponent>(entityNum, Components_e::MOVEABLE_COMPONENT);
        assert(moveComp);
        if(!moveComp->m_ejectData)
        {
            return true;
        }
    }
    return false;
}

//===================================================================
void CollisionSystem::treatEnemyShooted(uint32_t enemyEntityNum, uint32_t damage)
{
    EnemyConfComponent *enemyConfCompB = stairwayToComponentManager().
            searchComponentByType<EnemyConfComponent>(enemyEntityNum, Components_e::ENEMY_CONF_COMPONENT);
    TimerComponent *timerComp = stairwayToComponentManager().
            searchComponentByType<TimerComponent>(enemyEntityNum, Components_e::TIMER_COMPONENT);
    assert(enemyConfCompB);
    assert(timerComp);
    enemyConfCompB->m_touched = true;
    timerComp->m_clockC = std::chrono::system_clock::now();
    timerComp->m_clockB = std::chrono::system_clock::now();
    enemyConfCompB->m_attackPhase = EnemyAttackPhase_e::SHOOTED;
    //if enemy dead
    if(!enemyConfCompB->takeDamage(damage))
    {
        enemyConfCompB->m_behaviourMode = EnemyBehaviourMode_e::DYING;
        enemyConfCompB->m_touched = false;
        if(enemyConfCompB->m_dropedObjectEntity)
        {
            confDropedObject(*enemyConfCompB->m_dropedObjectEntity, enemyEntityNum);
        }
    }
}

//===================================================================
void CollisionSystem::confDropedObject(uint32_t objectEntity, uint32_t enemyEntity)
{
    GeneralCollisionComponent *genComp = stairwayToComponentManager().
            searchComponentByType<GeneralCollisionComponent>(objectEntity, Components_e::GENERAL_COLLISION_COMPONENT);
    assert(genComp);
    MapCoordComponent *objectMapComp = stairwayToComponentManager().
            searchComponentByType<MapCoordComponent>(objectEntity, Components_e::MAP_COORD_COMPONENT);
    assert(objectMapComp);
    MapCoordComponent *enemyMapComp = stairwayToComponentManager().
            searchComponentByType<MapCoordComponent>(enemyEntity, Components_e::MAP_COORD_COMPONENT);
    assert(enemyMapComp);
    genComp->m_active = true;
    objectMapComp->m_coord = enemyMapComp->m_coord;
    objectMapComp->m_absoluteMapPositionPX = enemyMapComp->m_absoluteMapPositionPX;
}

//===================================================================
void CollisionSystem::treatSegmentShots()
{
    GeneralCollisionComponent *tagCompTarget, *tagCompBullet;
    for(uint32_t i = 0; i < m_vectMemShots.size(); ++i)
    {
        tagCompTarget = stairwayToComponentManager().
                searchComponentByType<GeneralCollisionComponent>(
                    m_vectMemShots[i].second, Components_e::GENERAL_COLLISION_COMPONENT);
        assert(tagCompTarget);
        confImpactShots(i, tagCompTarget->m_tagA);
        if(tagCompTarget->m_tagA == CollisionTag_e::WALL_CT ||
                tagCompTarget->m_tagA == CollisionTag_e::DOOR_CT)
        {
            continue;
        }
        tagCompBullet = stairwayToComponentManager().searchComponentByType<GeneralCollisionComponent>(
                    m_vectMemShots[i].first, Components_e::GENERAL_COLLISION_COMPONENT);
        assert(tagCompBullet);
        if(tagCompBullet->m_tagA == CollisionTag_e::BULLET_PLAYER_CT)
        {
            ShotConfComponent *shotConfComp = stairwayToComponentManager().
                    searchComponentByType<ShotConfComponent>(m_vectMemShots[i].first, Components_e::SHOT_CONF_COMPONENT);
            assert(shotConfComp);
            if(tagCompTarget->m_tagA == CollisionTag_e::ENEMY_CT)
            {
                treatEnemyShooted(m_vectMemShots[i].second, shotConfComp->m_damage);
            }
            else if(tagCompTarget->m_tagA == CollisionTag_e::BARREL_CT)
            {
                treatBarrelShots(m_vectMemShots[i].second, shotConfComp->m_damage);
            }
            tagCompBullet->m_active = false;
        }
        else if(tagCompBullet->m_tagA == CollisionTag_e::BULLET_ENEMY_CT)
        {
            ShotConfComponent *shotConfComp = stairwayToComponentManager().
                    searchComponentByType<ShotConfComponent>(m_vectMemShots[i].first, Components_e::SHOT_CONF_COMPONENT);
            assert(shotConfComp);
            if(tagCompTarget->m_tagA == CollisionTag_e::PLAYER_CT)
            {
                PlayerConfComponent *playerConfCompB = stairwayToComponentManager().
                        searchComponentByType<PlayerConfComponent>(m_vectMemShots[i].second, Components_e::PLAYER_CONF_COMPONENT);
                assert(playerConfCompB);
                playerConfCompB->takeDamage(shotConfComp->m_damage);
            }
            else if(tagCompTarget->m_tagA == CollisionTag_e::BARREL_CT)
            {
                treatBarrelShots(m_vectMemShots[i].second, shotConfComp->m_damage);
            }
        }
    }
}

//===================================================================
void CollisionSystem::treatBarrelShots(uint32_t entityNum, uint32_t damage)
{
    BarrelComponent *barrelComp = stairwayToComponentManager().
            searchComponentByType<BarrelComponent>(entityNum, Components_e::BARREL_COMPONENT);
    assert(barrelComp);
    if(barrelComp->m_destructPhase)
    {
        return;
    }
    if(damage > barrelComp->m_life)
    {
        barrelComp->m_life = 0;
        barrelComp->m_destructPhase = true;
        activeSound(entityNum);
    }
    else
    {
        barrelComp->m_life -= damage;
    }
}

//===================================================================
void CollisionSystem::confImpactShots(uint32_t numBullet, CollisionTag_e targetTag)
{
    ShotConfComponent *shotComp = stairwayToComponentManager().
            searchComponentByType<ShotConfComponent>(m_vectMemShots[numBullet].first,
                                                     Components_e::SHOT_CONF_COMPONENT);
    assert(shotComp);
    //remove warning
    GeneralCollisionComponent *genImpact = nullptr;
    uint32_t impactEntity = shotComp->m_impactEntity;
    genImpact = stairwayToComponentManager().
            searchComponentByType<GeneralCollisionComponent>(impactEntity,
                                                             Components_e::GENERAL_COLLISION_COMPONENT);
    assert(genImpact);
    ImpactShotComponent *impactComp = stairwayToComponentManager().
            searchComponentByType<ImpactShotComponent>(impactEntity, Components_e::IMPACT_CONF_COMPONENT);
    assert(impactComp);
    SegmentCollisionComponent *segmentBullet = stairwayToComponentManager().
            searchComponentByType<SegmentCollisionComponent>(m_vectMemShots[numBullet].first,
                                                             Components_e::SEGMENT_COLLISION_COMPONENT);
    assert(segmentBullet);
    MapCoordComponent *mapImpact = stairwayToComponentManager().
            searchComponentByType<MapCoordComponent>(impactEntity,
                                                     Components_e::MAP_COORD_COMPONENT);
    TimerComponent *timerImpact = stairwayToComponentManager().
            searchComponentByType<TimerComponent>(impactEntity, Components_e::TIMER_COMPONENT);
    SpriteTextureComponent *spriteComp= stairwayToComponentManager().
            searchComponentByType<SpriteTextureComponent>(impactEntity, Components_e::SPRITE_TEXTURE_COMPONENT);
    MemSpriteDataComponent *memSpriteComp= stairwayToComponentManager().
            searchComponentByType<MemSpriteDataComponent>(impactEntity, Components_e::MEM_SPRITE_DATA_COMPONENT);
    MapCoordComponent *mapTargetComp = stairwayToComponentManager().
            searchComponentByType<MapCoordComponent>(m_vectMemShots[numBullet].second, Components_e::MAP_COORD_COMPONENT);
    assert(mapTargetComp);
    assert(timerImpact);
    assert(memSpriteComp);
    assert(spriteComp);
    assert(mapImpact);
    impactComp->m_moveUp = EPSILON_FLOAT;
    impactComp->m_touched = (targetTag == CollisionTag_e::ENEMY_CT ||
                             targetTag == CollisionTag_e::PLAYER_CT);
    if(impactComp->m_touched)
    {
        impactComp->m_spritePhase = ImpactPhase_e::TOUCHED;
    }
    else
    {
        impactComp->m_spritePhase = ImpactPhase_e::FIRST;
    }
    spriteComp->m_spriteData = memSpriteComp->m_vectSpriteData[static_cast<uint32_t>(impactComp->m_spritePhase)];
    if(m_memDistCurrentBulletColl.second >= 10000.0f)
    {
        mapImpact->m_absoluteMapPositionPX = segmentBullet->m_points.second;
    }
    else
    {
        mapImpact->m_absoluteMapPositionPX = segmentBullet->m_points.first;
        float radiantAngle = getTrigoAngle(segmentBullet->m_points.first,
                                           segmentBullet->m_points.second, false);
        moveElementFromAngle(m_memDistCurrentBulletColl.second, radiantAngle, (*mapImpact).m_absoluteMapPositionPX);
    }
    genImpact->m_active = true;
    timerImpact->m_clockA = std::chrono::system_clock::now();
}

//===================================================================
void CollisionSystem::rmCollisionMaskEntity(uint32_t numEntity)
{
    GeneralCollisionComponent *tagComp = stairwayToComponentManager().
            searchComponentByType<GeneralCollisionComponent>(numEntity,
                                                             Components_e::GENERAL_COLLISION_COMPONENT);
    assert(tagComp);
    tagComp->m_tagA = CollisionTag_e::GHOST_CT;
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

    m_tagArray.insert({CollisionTag_e::BARREL_CT, CollisionTag_e::BARREL_CT});

    m_tagArray.insert({CollisionTag_e::PLAYER_ACTION_CT, CollisionTag_e::DOOR_CT});
    m_tagArray.insert({CollisionTag_e::PLAYER_ACTION_CT, CollisionTag_e::EXIT_CT});
    m_tagArray.insert({CollisionTag_e::PLAYER_ACTION_CT, CollisionTag_e::TRIGGER_CT});
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
//    m_tagArray.insert({CollisionTag_e::ENEMY_CT, CollisionTag_e::ENEMY_CT});

    m_tagArray.insert({CollisionTag_e::WALL_CT, CollisionTag_e::PLAYER_CT});
    m_tagArray.insert({CollisionTag_e::WALL_CT, CollisionTag_e::ENEMY_CT});

    m_tagArray.insert({CollisionTag_e::DOOR_CT, CollisionTag_e::PLAYER_CT});
    m_tagArray.insert({CollisionTag_e::DOOR_CT, CollisionTag_e::ENEMY_CT});

    //bullets collision with walls and doors are treated by raycasting
    m_tagArray.insert({CollisionTag_e::BULLET_ENEMY_CT, CollisionTag_e::PLAYER_CT});
    m_tagArray.insert({CollisionTag_e::BULLET_ENEMY_CT, CollisionTag_e::WALL_CT});
    m_tagArray.insert({CollisionTag_e::BULLET_ENEMY_CT, CollisionTag_e::DOOR_CT});
    m_tagArray.insert({CollisionTag_e::BULLET_ENEMY_CT, CollisionTag_e::BARREL_CT});
//    m_tagArray.insert({CollisionTag_e::BULLET_ENEMY_CT, CollisionTag_e::ENEMY_CT});

    m_tagArray.insert({CollisionTag_e::BULLET_PLAYER_CT, CollisionTag_e::ENEMY_CT});
    m_tagArray.insert({CollisionTag_e::BULLET_PLAYER_CT, CollisionTag_e::WALL_CT});
    m_tagArray.insert({CollisionTag_e::BULLET_PLAYER_CT, CollisionTag_e::DOOR_CT});
    m_tagArray.insert({CollisionTag_e::BULLET_PLAYER_CT, CollisionTag_e::BARREL_CT});

    //    m_tagArray.insert({CollisionTag_e::OBJECT_CT, CollisionTag_e::PLAYER_CT});
}

//===================================================================
void CollisionSystem::memPlayerDatas(uint32_t playerEntity)
{
    m_playerComp = stairwayToComponentManager().
            searchComponentByType<PlayerConfComponent>(playerEntity,
                                                       Components_e::PLAYER_CONF_COMPONENT);
    assert(m_playerComp);

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
void CollisionSystem::treatCollision(uint32_t entityNumA, uint32_t entityNumB,
                                     GeneralCollisionComponent *tagCompA, GeneralCollisionComponent *tagCompB)
{

//    if(tagCompA->m_shape == CollisionShape_e::RECTANGLE_C)
//    {
//        checkCollisionFirstRect(args);
//    }
    if(tagCompA->m_shape == CollisionShape_e::CIRCLE_C)
    {
        CollisionArgs args = {entityNumA, entityNumB, tagCompA, tagCompB, getMapComponent(entityNumA), getMapComponent(entityNumB)};
        treatCollisionFirstCircle(args);
    }
    else if(tagCompA->m_shape == CollisionShape_e::SEGMENT_C)
    {
        assert(tagCompA->m_tagA == CollisionTag_e::BULLET_PLAYER_CT || tagCompA->m_tagA == CollisionTag_e::BULLET_ENEMY_CT);
        checkCollisionFirstSegment(entityNumA, entityNumB, tagCompB, getMapComponent(entityNumB));
    }
}

//===================================================================
//void CollisionSystem::checkCollisionFirstRect(CollisionArgs &args)
//{
//    bool collision = false;
//    RectangleCollisionComponent &rectCompA = getRectangleComponent(args.entityNumA);
//    switch(args.tagCompB->m_shape)
//    {
//    case CollisionShape_e::RECTANGLE_C:
//    {
//        RectangleCollisionComponent &rectCompB = getRectangleComponent(args.entityNumB);
//        collision = checkRectRectCollision(args.mapCompA.m_absoluteMapPositionPX, rectCompA.m_size,
//                               args.mapCompB.m_absoluteMapPositionPX, rectCompB.m_size);
//    }
//        break;
//    case CollisionShape_e::CIRCLE:
//    {
//        CircleCollisionComponent &circleCompB = getCircleComponent(args.entityNumB);
//        collision = checkCircleRectCollision(args.mapCompB.m_absoluteMapPositionPX, circleCompB.m_ray,
//                                 args.mapCompA.m_absoluteMapPositionPX, rectCompA.m_size);
//    }
//        break;
//    case CollisionShape_e::SEGMENT:
//    {
//        SegmentCollisionComponent &segmentCompB = getSegmentComponent(args.entityNumB);
//        collision = checkSegmentRectCollision(args.mapCompB.m_absoluteMapPositionPX, segmentCompB.m_secondPoint,
//                               args.mapCompA.m_absoluteMapPositionPX, rectCompA.m_size);
//    }
//        break;
//    }
//}

//===================================================================
void CollisionSystem::writePlayerInfo(const std::string &info)
{
    TimerComponent *timerComp = stairwayToComponentManager().
            searchComponentByType<TimerComponent>(m_playerComp->muiGetIdEntityAssociated(), Components_e::TIMER_COMPONENT);
    assert(timerComp);
    timerComp->m_clockA = std::chrono::system_clock::now();
    m_playerComp->m_infoWriteData = {true, info};
}

//===================================================================
void CollisionSystem::treatCollisionFirstCircle(CollisionArgs &args)
{
    if(args.tagCompA->m_tagA == CollisionTag_e::PLAYER_ACTION_CT ||
            args.tagCompA->m_tagA == CollisionTag_e::HIT_PLAYER_CT)
    {
        args.tagCompA->m_active = false;
    }
    CircleCollisionComponent &circleCompA = getCircleComponent(args.entityNumA);
    bool collision = false;
    switch(args.tagCompB->m_shape)
    {
    case CollisionShape_e::RECTANGLE_C:
    {
        RectangleCollisionComponent &rectCompB = getRectangleComponent(args.entityNumB);
        collision = checkCircleRectCollision(args.mapCompA.m_absoluteMapPositionPX, circleCompA.m_ray,
                                 args.mapCompB.m_absoluteMapPositionPX, rectCompB.m_size);
        if(collision)
        {
            if(args.tagCompA->m_tagA == CollisionTag_e::PLAYER_ACTION_CT)
            {
                treatActionPlayerRect(args);
            }
            else if(args.tagCompA->m_tagA == CollisionTag_e::PLAYER_CT)
            {
                if(args.tagCompB->m_tagA == CollisionTag_e::DOOR_CT)
                {
                    DoorComponent *doorComp = stairwayToComponentManager().
                            searchComponentByType<DoorComponent>(args.entityNumB, Components_e::DOOR_COMPONENT);
                    assert(doorComp);
                    if(doorComp->m_currentState == DoorState_e::MOVE_CLOSE)
                    {
                        doorComp->m_currentState = DoorState_e::MOVE_OPEN;
                        activeSound(args.entityNumB);
                    }
                }
                collisionCircleRectEject(args, circleCompA, rectCompB);
            }
            else if(args.tagCompA->m_tagA == CollisionTag_e::ENEMY_CT)
            {
                collisionCircleRectEject(args, circleCompA, rectCompB);
                if(args.tagCompB->m_tagA == CollisionTag_e::DOOR_CT)
                {
                    DoorComponent *doorComp = stairwayToComponentManager().
                            searchComponentByType<DoorComponent>(args.entityNumB, Components_e::DOOR_COMPONENT);
                    assert(doorComp);
                    if(!doorComp->m_cardID)
                    {
                        doorComp->m_currentState = DoorState_e::MOVE_OPEN;
                        activeSound(args.entityNumB);
                    }
                }
            }
        }
    }
        break;
    case CollisionShape_e::CIRCLE_C:
    {
        CircleCollisionComponent &circleCompB = getCircleComponent(args.entityNumB);
        collision = checkCircleCircleCollision(args.mapCompA.m_absoluteMapPositionPX, circleCompA.m_ray,
                                               args.mapCompB.m_absoluteMapPositionPX, circleCompB.m_ray);
        if(collision)
        {
            if(args.tagCompA->m_tagA == CollisionTag_e::PLAYER_ACTION_CT)
            {
                treatActionPlayerCircle(args);
            }
            else if(args.tagCompA->m_tagA == CollisionTag_e::HIT_PLAYER_CT)
            {
                ShotConfComponent *shotConfComp = stairwayToComponentManager().
                        searchComponentByType<ShotConfComponent>(args.entityNumA, Components_e::SHOT_CONF_COMPONENT);
                assert(shotConfComp);
                if(args.tagCompB->m_tagA == CollisionTag_e::ENEMY_CT)
                {
                    activeSound(args.entityNumA);
                    treatEnemyShooted(args.entityNumB, shotConfComp->m_damage);
                }
            }
            else if((args.tagCompA->m_tagA == CollisionTag_e::PLAYER_CT || args.tagCompA->m_tagA == CollisionTag_e::ENEMY_CT ||
                     args.tagCompA->m_tagB == CollisionTag_e::BARREL_CT) &&
                    (args.tagCompB->m_tagA == CollisionTag_e::WALL_CT || args.tagCompB->m_tagA == CollisionTag_e::PLAYER_CT ||
                     args.tagCompB->m_tagA == CollisionTag_e::ENEMY_CT || args.tagCompB->m_tagA == CollisionTag_e::STATIC_SET_CT ||
                     args.tagCompB->m_tagB == CollisionTag_e::BARREL_CT))
            {
                collisionCircleCircleEject(args, circleCompA, circleCompB);
            }
            else if(args.tagCompA->m_tagA == CollisionTag_e::PLAYER_CT)
            {
                if(args.tagCompB->m_tagA == CollisionTag_e::OBJECT_CT)
                {
                    treatPlayerPickObject(args);
                }
                else if(args.tagCompB->m_tagA == CollisionTag_e::TRIGGER_CT)
                {
                    TriggerComponent *triggerComp = stairwayToComponentManager().
                            searchComponentByType<TriggerComponent>(args.entityNumB, Components_e::TRIGGER_COMPONENT);
                    assert(triggerComp);
                    triggerComp->m_actionned = true;
                }
                else if(args.tagCompB->m_tagA == CollisionTag_e::TELEPORT_CT)
                {
                    treatPlayerTeleport(args);
                    m_memPlayerTeleport = true;
                }
            }
            else if(args.tagCompA->m_tagA == CollisionTag_e::EXPLOSION_CT)
            {
                treatExplosionColl(args);
            }
        }
    }
        break;
    case CollisionShape_e::SEGMENT_C:
    {
//        SegmentCollisionComponent &segmentCompB = getSegmentComponent(args.entityNumB);
//        collision = checkCircleSegmentCollision(args.mapCompA.m_absoluteMapPositionPX, circleCompA.m_ray,
//                                                segmentCompB.m_points.first, segmentCompB.m_points.second);
//        if(collision)
//        {
////            treatCollisionCircleSegment(args, circleCompA, segmentCompB);
//        }
    }
        break;
    }
    //TREAT VISIBLE SHOT
    if((args.tagCompA->m_tagA == CollisionTag_e::BULLET_ENEMY_CT) || (args.tagCompA->m_tagA == CollisionTag_e::BULLET_PLAYER_CT))
    {
        bool limitLevelDestruct = false;
        //limit level case
        if(args.mapCompA.m_absoluteMapPositionPX.first < LEVEL_THIRD_TILE_SIZE_PX ||
                args.mapCompA.m_absoluteMapPositionPX.second < LEVEL_THIRD_TILE_SIZE_PX)
        {
            limitLevelDestruct = true;
        }
        if(limitLevelDestruct || collision)
        {
            ShotConfComponent *shotConfComp = stairwayToComponentManager().
                    searchComponentByType<ShotConfComponent>(args.entityNumA, Components_e::SHOT_CONF_COMPONENT);
            assert(shotConfComp);
            if(args.tagCompB->m_shape == CollisionShape_e::RECTANGLE_C)
            {
                if(!shotConfComp->m_ejectMode)
                {
                    shotConfComp->m_ejectMode = true;
                    std::swap(circleCompA.m_ray, shotConfComp->m_ejectExplosionRay);
                    RectangleCollisionComponent &rectCompB = getRectangleComponent(args.entityNumB);
                    collisionCircleRectEject(args, circleCompA, rectCompB);
                    return;
                }
                if(args.tagCompB->m_tagA == CollisionTag_e::WALL_CT && !shotConfComp->m_currentLoopEjected)
                {
                    MoveableWallConfComponent *moveWallComp = stairwayToComponentManager().
                            searchComponentByType<MoveableWallConfComponent>(args.entityNumB, Components_e::MOVEABLE_WALL_CONF_COMPONENT);
                    if(moveWallComp)
                    {
                        shotConfComp->m_currentLoopEjected = true;
                        RectangleCollisionComponent &rectCompB = getRectangleComponent(args.entityNumB);
                        collisionCircleRectEject(args, circleCompA, rectCompB);
                    }
                }
            }
            if(shotConfComp->m_destructPhase)
            {
                return;
            }
            if(shotConfComp->m_damageCircleRayData)
            {
                setDamageCircle(*shotConfComp->m_damageCircleRayData, true, args.entityNumA);
            }
            TimerComponent *timerComp = stairwayToComponentManager().
                    searchComponentByType<TimerComponent>(args.entityNumA, Components_e::TIMER_COMPONENT);
            assert(timerComp);
            activeSound(args.entityNumA);
            timerComp->m_clockB = std::chrono::system_clock::now();
            shotConfComp->m_destructPhase = true;
            shotConfComp->m_spriteShotNum = 1;
            if(limitLevelDestruct || shotConfComp->m_damageCircleRayData)
            {
                return;
            }
            if(args.tagCompA->m_tagA == CollisionTag_e::BULLET_PLAYER_CT && args.tagCompB->m_tagA == CollisionTag_e::ENEMY_CT)
            {
                treatEnemyShooted(args.entityNumB, shotConfComp->m_damage);
            }
            else if(args.tagCompA->m_tagA == CollisionTag_e::BULLET_ENEMY_CT && args.tagCompB->m_tagA == CollisionTag_e::PLAYER_CT)
            {
                PlayerConfComponent * playerConf = stairwayToComponentManager().
                        searchComponentByType<PlayerConfComponent>(args.entityNumB, Components_e::PLAYER_CONF_COMPONENT);
                assert(playerConf);
                playerConf->takeDamage(shotConfComp->m_damage);
            }
            else if(args.tagCompB->m_tagA == CollisionTag_e::BARREL_CT)
            {
                treatBarrelShots(args.entityNumB, shotConfComp->m_damage);
            }
        }
    }
}

//===================================================================
void CollisionSystem::treatExplosionColl(CollisionArgs &args)
{
    ShotConfComponent *shotConfComp = stairwayToComponentManager().
            searchComponentByType<ShotConfComponent>(args.entityNumA, Components_e::SHOT_CONF_COMPONENT);
    assert(shotConfComp);
    MoveableComponent *moveComp = stairwayToComponentManager().
            searchComponentByType<MoveableComponent>(args.entityNumB, Components_e::MOVEABLE_COMPONENT);
    if(moveComp && !(args.tagCompB->m_tagA == CollisionTag_e::BARREL_CT &&
            args.tagCompB->m_tagB != CollisionTag_e::BARREL_CT))
    {
        TimerComponent *timerComp = stairwayToComponentManager().
                searchComponentByType<TimerComponent>(args.entityNumB, Components_e::TIMER_COMPONENT);
        assert(timerComp);
        timerComp->m_clockD = std::chrono::system_clock::now();
        if(!moveComp->m_ejectData && (std::abs(args.mapCompA.m_absoluteMapPositionPX.first - args.mapCompB.m_absoluteMapPositionPX.first) > 0.01f ||
                std::abs(args.mapCompA.m_absoluteMapPositionPX.second - args.mapCompB.m_absoluteMapPositionPX.second) > 0.01f))
        {
            moveComp->m_currentDegreeMoveDirection = getTrigoAngle(args.mapCompA.m_absoluteMapPositionPX, args.mapCompB.m_absoluteMapPositionPX);
            moveComp->m_ejectData = {1.5f, EJECT_TIME};
        }
    }
    if(args.tagCompB->m_tagA == CollisionTag_e::PLAYER_CT)
    {
        //OOOOOK factor
        PlayerConfComponent * playerConf = stairwayToComponentManager().
                searchComponentByType<PlayerConfComponent>(args.entityNumB, Components_e::PLAYER_CONF_COMPONENT);
        assert(playerConf);
        playerConf->takeDamage(shotConfComp->m_damage);
    }
    else if(args.tagCompB->m_tagA == CollisionTag_e::ENEMY_CT)
    {
        treatEnemyShooted(args.entityNumB, shotConfComp->m_damage);
    }
    else if(args.tagCompB->m_tagA == CollisionTag_e::BARREL_CT)
    {
        treatBarrelShots(args.entityNumB, shotConfComp->m_damage);
    }
}

//===================================================================
void CollisionSystem::setDamageCircle(uint32_t damageEntity, bool active, uint32_t baseEntity)
{
    GeneralCollisionComponent *genDam = stairwayToComponentManager().
            searchComponentByType<GeneralCollisionComponent>(damageEntity, Components_e::GENERAL_COLLISION_COMPONENT);
    assert(genDam);
    genDam->m_active = active;
    if(active)
    {
        MapCoordComponent *mapCompDam = stairwayToComponentManager().
                searchComponentByType<MapCoordComponent>(damageEntity, Components_e::MAP_COORD_COMPONENT);
        MapCoordComponent *mapComp = stairwayToComponentManager().
                searchComponentByType<MapCoordComponent>(baseEntity, Components_e::MAP_COORD_COMPONENT);
        assert(mapCompDam);
        assert(mapComp);
        mapCompDam->m_absoluteMapPositionPX = mapComp->m_absoluteMapPositionPX;
    }
}

//===================================================================
void CollisionSystem::treatActionPlayerRect(CollisionArgs &args)
{
    if(args.tagCompB->m_tagA == CollisionTag_e::DOOR_CT)
    {
        DoorComponent *doorComp = stairwayToComponentManager().
                searchComponentByType<DoorComponent>(args.entityNumB, Components_e::DOOR_COMPONENT);
        assert(doorComp);
        //if card door
        if(doorComp->m_cardID)
        {
            if((m_playerComp->m_card.find((*doorComp->m_cardID).first) == m_playerComp->m_card.end()))
            {
                writePlayerInfo((*doorComp->m_cardID).second + " NEEDED");
                return;
            }
            else
            {
                writePlayerInfo((*doorComp->m_cardID).second + " USED");
            }
        }
        if(doorComp->m_currentState == DoorState_e::STATIC_CLOSED || doorComp->m_currentState == DoorState_e::MOVE_CLOSE)
        {
            doorComp->m_currentState = DoorState_e::MOVE_OPEN;
            activeSound(args.entityNumB);
        }
    }
    else if(args.tagCompB->m_tagB == CollisionTag_e::TRIGGER_CT)
    {
        //TREAT MOVEABLE WALL
        if(args.tagCompB->m_tagA == CollisionTag_e::WALL_CT)
        {
            MoveableWallConfComponent *moveWallComp = stairwayToComponentManager().
                    searchComponentByType<MoveableWallConfComponent>(args.entityNumB, Components_e::MOVEABLE_WALL_CONF_COMPONENT);
            assert(moveWallComp);
            moveWallComp->m_manualTrigger = true;
        }
    }
}

//===================================================================
void CollisionSystem::activeSound(uint32_t entityNum)
{
    AudioComponent *audioComp = stairwayToComponentManager().
            searchComponentByType<AudioComponent>(entityNum, Components_e::AUDIO_COMPONENT);
    assert(audioComp);
    audioComp->m_soundElements[0]->m_toPlay = true;
}

//===================================================================
void CollisionSystem::treatActionPlayerCircle(CollisionArgs &args)
{
    if(args.tagCompB->m_tagA == CollisionTag_e::EXIT_CT)
    {
        args.tagCompB->m_active = false;
    }
    else if(args.tagCompB->m_tagB == CollisionTag_e::TRIGGER_CT)
    {

        TriggerComponent *triggerComp = stairwayToComponentManager().
                searchComponentByType<TriggerComponent>(args.entityNumB, Components_e::TRIGGER_COMPONENT);
        assert(triggerComp);
        triggerComp->m_actionned = true;
    }
}

//===================================================================
void CollisionSystem::treatPlayerPickObject(CollisionArgs &args)
{
    PlayerConfComponent *playerComp = stairwayToComponentManager().
            searchComponentByType<PlayerConfComponent>(args.entityNumA, Components_e::PLAYER_CONF_COMPONENT);
    ObjectConfComponent *objectComp = stairwayToComponentManager().
            searchComponentByType<ObjectConfComponent>(args.entityNumB, Components_e::OBJECT_CONF_COMPONENT);
    WeaponComponent *weaponComp = stairwayToComponentManager().
            searchComponentByType<WeaponComponent>(playerComp->m_weaponEntity, Components_e::WEAPON_COMPONENT);
    assert(weaponComp);
    assert(playerComp);
    assert(objectComp);
    switch (objectComp->m_type)
    {
    case ObjectType_e::AMMO_WEAPON:
        if(!pickUpAmmo(*objectComp->m_weaponID, weaponComp, objectComp->m_containing))
        {
            return;
        }
        break;
    case ObjectType_e::WEAPON:
        if(!pickUpWeapon(*objectComp->m_weaponID, weaponComp, objectComp->m_containing))
        {
            return;
        }
        break;
    case ObjectType_e::HEAL:
    {
        if(playerComp->m_life == 100)
        {
            return;
        }
        playerComp->m_life += objectComp->m_containing;
        if(playerComp->m_life > 100)
        {
            playerComp->m_life = 100;
        }
        break;
    }
    case ObjectType_e::CARD:
        playerComp->m_card.insert(*objectComp->m_cardID);
        break;
    case ObjectType_e::TOTAL:
        assert(false);
        break;
    }
    playerComp->m_pickItem = true;
    activeSound(args.entityNumA);
    m_vectEntitiesToDelete.push_back(args.entityNumB);
}

//===================================================================
void CollisionSystem::treatPlayerTeleport(CollisionArgs &args)
{

    PlayerConfComponent *playerComp = stairwayToComponentManager().
            searchComponentByType<PlayerConfComponent>(args.entityNumA, Components_e::PLAYER_CONF_COMPONENT);
    assert(playerComp);
    if(playerComp->m_teleported)
    {
        return;
    }
    MapCoordComponent *mapPlayerComp = stairwayToComponentManager().
            searchComponentByType<MapCoordComponent>(args.entityNumA, Components_e::MAP_COORD_COMPONENT);
    TeleportComponent *teleportComp = stairwayToComponentManager().
            searchComponentByType<TeleportComponent>(args.entityNumB, Components_e::TELEPORT_COMPONENT);
    GeneralCollisionComponent *genTeleportComp = stairwayToComponentManager().
            searchComponentByType<GeneralCollisionComponent>(playerComp->m_displayTeleportEntity,
                                                             Components_e::GENERAL_COLLISION_COMPONENT);
    MapCoordComponent *mapTeleportComp = stairwayToComponentManager().
            searchComponentByType<MapCoordComponent>(playerComp->m_displayTeleportEntity,
                                                     Components_e::MAP_COORD_COMPONENT);
    TimerComponent *timerComp = stairwayToComponentManager().
            searchComponentByType<TimerComponent>(playerComp->m_displayTeleportEntity,
                                                  Components_e::TIMER_COMPONENT);
    MoveableComponent *moveComp = stairwayToComponentManager().
            searchComponentByType<MoveableComponent>(args.entityNumA,
                                                     Components_e::MOVEABLE_COMPONENT);
    activeSound(args.entityNumB);
    assert(moveComp);
    assert(timerComp);
    assert(genTeleportComp);
    assert(mapTeleportComp);
    assert(mapPlayerComp);
    assert(teleportComp);
    timerComp->m_clockA = std::chrono::system_clock::now();
    timerComp->m_clockB = std::chrono::system_clock::now();
    mapPlayerComp->m_coord = teleportComp->m_targetPos;
    mapPlayerComp->m_absoluteMapPositionPX = getCenteredAbsolutePosition(mapPlayerComp->m_coord);
    mapTeleportComp->m_absoluteMapPositionPX = mapPlayerComp->m_absoluteMapPositionPX;
    moveElementFromAngle(LEVEL_TILE_SIZE_PX, getRadiantAngle(moveComp->m_degreeOrientation),
                         mapTeleportComp->m_absoluteMapPositionPX);
    playerComp->m_teleported = true;
    genTeleportComp->m_active = true;
}

//===================================================================
bool CollisionSystem::treatCrushing(const CollisionArgs &args, float diffX, float diffY)
{
    PlayerConfComponent *playerComp = stairwayToComponentManager().
            searchComponentByType<PlayerConfComponent>(args.entityNumA, Components_e::PLAYER_CONF_COMPONENT);
    if(playerComp && !playerComp->m_crush)
    {
        //mem crush if player is eject from a moveable wall
        if(!std::get<0>(playerComp->m_crushMem))
        {
            if(args.tagCompB->m_tagA == CollisionTag_e::WALL_CT)
            {
                MoveableWallConfComponent *moveWallComp = stairwayToComponentManager().
                        searchComponentByType<MoveableWallConfComponent>(args.entityNumA,
                                                                         Components_e::MOVEABLE_WALL_CONF_COMPONENT);
                playerComp->m_crushMem = {true, moveWallComp,
                                          getDirection(diffX, diffY), args.entityNumB};
            }
        }
        //if crush
        else
        {
            MoveableWallConfComponent *moveWallComp = stairwayToComponentManager().
                    searchComponentByType<MoveableWallConfComponent>(args.entityNumB,
                                                                     Components_e::MOVEABLE_WALL_CONF_COMPONENT);
            //check if at least one wall is moveable and 2 distinct walls
            if(args.tagCompB->m_tagA == CollisionTag_e::WALL_CT &&
                    (moveWallComp || args.tagCompB->m_tagB == CollisionTag_e::WALL_CT) &&
                    args.entityNumB != std::get<3>(playerComp->m_crushMem))
            {
                Direction_e dir = getDirection(diffX, diffY);
                if((std::get<2>(playerComp->m_crushMem) == Direction_e::EAST &&
                    dir == Direction_e::WEST) ||
                        (std::get<2>(playerComp->m_crushMem) == Direction_e::WEST &&
                         dir == Direction_e::EAST) ||
                        (std::get<2>(playerComp->m_crushMem) == Direction_e::NORTH &&
                         dir == Direction_e::SOUTH) ||
                        (std::get<2>(playerComp->m_crushMem) == Direction_e::SOUTH &&
                         dir == Direction_e::NORTH))
                {
                    playerComp->m_crush = true;
                    playerComp->m_frozen = true;
                    playerComp->takeDamage(1);
                    return true;
                }
            }
        }
    }
    return false;
}

//===================================================================
bool pickUpAmmo(uint32_t numWeapon, WeaponComponent *weaponComp,
                uint32_t objectContaining)
{
    WeaponData &objectWeapon = weaponComp->m_weaponsData[numWeapon];
    if(objectWeapon.m_ammunationsCount == objectWeapon.m_maxAmmunations)
    {
        return false;
    }
    objectWeapon.m_ammunationsCount += objectContaining;
    if(objectWeapon.m_ammunationsCount > objectWeapon.m_maxAmmunations)
    {
        objectWeapon.m_ammunationsCount = objectWeapon.m_maxAmmunations;
    }
    return true;
}

//===================================================================
bool pickUpWeapon(uint32_t numWeapon, WeaponComponent *weaponComp,
                  uint32_t objectContaining)
{
    WeaponData &objectWeapon = weaponComp->m_weaponsData[numWeapon];
    if(objectWeapon.m_posses &&
            objectWeapon.m_ammunationsCount == objectWeapon.m_maxAmmunations)
    {
        return false;
    }
    if(!objectWeapon.m_posses)
    {
        objectWeapon.m_posses = true;
        if(weaponComp->m_currentWeapon < numWeapon)
        {
            setPlayerWeapon(*weaponComp, numWeapon);
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
                                                 GeneralCollisionComponent *tagCompB,
                                                 MapCoordComponent &mapCompB)
{
    SegmentCollisionComponent &segmentCompA = getSegmentComponent(numEntityA);
    switch(tagCompB->m_shape)
    {
    case CollisionShape_e::RECTANGLE_C:
    case CollisionShape_e::SEGMENT_C:
    {
        //        RectangleCollisionComponent &rectCompB = getRectangleComponent(numEntityB);
        //        collision = checkSegmentRectCollision(segmentCompA.m_points.first ,
        //                                              segmentCompA.m_points.second,
        //                                              mapCompB.m_absoluteMapPositionPX, rectCompB.m_size);
    }
        break;
    case CollisionShape_e::CIRCLE_C:
    {
        CircleCollisionComponent &circleCompB = getCircleComponent(numEntityB);
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
void CollisionSystem::collisionCircleRectEject(CollisionArgs &args, const CircleCollisionComponent &circleCollA,
                                               const RectangleCollisionComponent &rectCollB)
{
    MapCoordComponent &mapComp = getMapComponent(args.entityNumA);
    MoveableComponent *moveComp = stairwayToComponentManager().
            searchComponentByType<MoveableComponent>(args.entityNumA, Components_e::MOVEABLE_COMPONENT);
    assert(moveComp);
    float radiantObserverAngle = getRadiantAngle(moveComp->m_currentDegreeMoveDirection);
    float circlePosX = args.mapCompA.m_absoluteMapPositionPX.first;
    float circlePosY = args.mapCompA.m_absoluteMapPositionPX.second;
    float elementPosX = args.mapCompB.m_absoluteMapPositionPX.first;
    float elementPosY = args.mapCompB.m_absoluteMapPositionPX.second;
    float elementSecondPosX = elementPosX + rectCollB.m_size.first;
    float elementSecondPosY = elementPosY + rectCollB.m_size.second;
    bool angleBehavior = false, limitEjectY = false, limitEjectX = false;
    //collision on angle of rect
    if((circlePosX < elementPosX || circlePosX > elementSecondPosX) &&
            (circlePosY < elementPosY || circlePosY > elementSecondPosY))
    {
        angleBehavior = true;
    }
    float pointElementX = (circlePosX < elementPosX) ? elementPosX : elementSecondPosX;
    float pointElementY = (circlePosY < elementPosY) ? elementPosY : elementSecondPosY;
    float diffY, diffX = EPSILON_FLOAT;
    diffY = getVerticalCircleRectEject({circlePosX, circlePosY, pointElementX, elementPosY,
                                        elementSecondPosY, circleCollA.m_ray, radiantObserverAngle, angleBehavior}, limitEjectY);
    if(!limitEjectY)
    {
        diffX = getHorizontalCircleRectEject({circlePosX, circlePosY, pointElementY, elementPosX, elementSecondPosX,
                                              circleCollA.m_ray, radiantObserverAngle, angleBehavior}, limitEjectX);
    }
    treatCrushing(args, diffX, diffY);
    collisionEject(mapComp, diffX, diffY, limitEjectY, limitEjectX);
}

//===================================================================
float CollisionSystem::getVerticalCircleRectEject(const EjectYArgs& args, bool &limitEject)
{
    float adj, diffYA = EPSILON_FLOAT, diffYB;
    if(std::abs(std::sin(args.radDegree)) < 0.01f &&
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
        diffYA = getRectTriangleSide(adj, args.ray);
        //EJECT UP
        if(std::sin(args.radDegree) < EPSILON_FLOAT)
        {
            diffYA -= (args.elementPosY - args.circlePosY);
            diffYA = -diffYA;
        }
        //EJECT DOWN
        else
        {
            diffYA -= (args.circlePosY - args.elementSecondPosY);
        }
    }
    else
    {
        diffYA = args.elementSecondPosY - (args.circlePosY - args.ray);
        diffYB = args.elementPosY - (args.circlePosY + args.ray);
        diffYA = (std::abs(diffYA) < std::abs(diffYB)) ? diffYA : diffYB;
    }
    return diffYA;
}

//===================================================================
float CollisionSystem::getHorizontalCircleRectEject(const EjectXArgs &args, bool &limitEject)
{
    float adj, diffXA = EPSILON_FLOAT, diffXB;
    if(std::abs(std::cos(args.radDegree)) < 0.01f &&
            (args.angleMode || args.circlePosX < args.elementPosX ||
             args.circlePosX > args.elementSecondPosX))
    {
        float distLeftPoint = std::abs(args.circlePosX - args.elementPosX),
                distRightPoint = std::abs(args.circlePosX - args.elementSecondPosX);
        limitEject = true;
        if(distLeftPoint < distRightPoint )
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
        //RIGHT
        if(std::cos(args.radDegree) > EPSILON_FLOAT)
        {
            diffXA -= (args.elementPosX - args.circlePosX);
            diffXA = -diffXA;
        }
        //LEFT
        else
        {
            diffXA -= (args.circlePosX - args.elementSecondPosX);
        }
    }
    else
    {
        diffXA = args.elementSecondPosX - (args.circlePosX - args.ray) ;
        diffXB = args.elementPosX - (args.circlePosX + args.ray);
        diffXA = (std::abs(diffXA) < std::abs(diffXB)) ? diffXA : diffXB;
    }
    return diffXA;
}

//===================================================================
void CollisionSystem::collisionEject(MapCoordComponent &mapComp, float diffX, float diffY, bool limitEjectY, bool limitEjectX)
{
    if(!limitEjectX && (limitEjectY || std::abs(diffY) < std::abs(diffX)))
    {
        mapComp.m_absoluteMapPositionPX.second += diffY;
    }
    if(!limitEjectY && (limitEjectX || std::abs(diffY) > std::abs(diffX)))
    {
        mapComp.m_absoluteMapPositionPX.first += diffX;
    }
}

//===================================================================
void CollisionSystem::collisionCircleCircleEject(CollisionArgs &args,
                                                 const CircleCollisionComponent &circleCollA,
                                                 const CircleCollisionComponent &circleCollB)
{
    MoveableComponent *moveCompA = stairwayToComponentManager().
            searchComponentByType<MoveableComponent>(args.entityNumA,
                                                     Components_e::MOVEABLE_COMPONENT);
    assert(moveCompA);

    float circleAPosX = args.mapCompA.m_absoluteMapPositionPX.first;
    float circleAPosY = args.mapCompA.m_absoluteMapPositionPX.second;
    float circleBPosX = args.mapCompB.m_absoluteMapPositionPX.first;
    float circleBPosY = args.mapCompB.m_absoluteMapPositionPX.second;
    float distanceX = std::abs(circleAPosX - circleBPosX);
    float distanceY = std::abs(circleAPosY - circleBPosY);
    float hyp = circleCollA.m_ray + circleCollB.m_ray;
    assert(hyp > distanceX && hyp > distanceY);
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
//void CollisionSystem::treatCollisionCircleSegment(CollisionArgs &args,
//                                                  const CircleCollisionComponent &circleCollA,
//                                                  const SegmentCollisionComponent &segmCollB)
//{

//}



//===================================================================
CircleCollisionComponent &CollisionSystem::getCircleComponent(uint32_t entityNum)
{
    CircleCollisionComponent *collComp = stairwayToComponentManager().
            searchComponentByType<CircleCollisionComponent>(entityNum,
                                  Components_e::CIRCLE_COLLISION_COMPONENT);
    assert(collComp);
    return *collComp;
}

//===================================================================
RectangleCollisionComponent &CollisionSystem::getRectangleComponent(uint32_t entityNum)
{
    RectangleCollisionComponent *collComp = stairwayToComponentManager().
            searchComponentByType<RectangleCollisionComponent>(entityNum,
                                  Components_e::RECTANGLE_COLLISION_COMPONENT);
    assert(collComp);
    return *collComp;
}

//===================================================================
SegmentCollisionComponent &CollisionSystem::getSegmentComponent(uint32_t entityNum)
{
    SegmentCollisionComponent *collComp = stairwayToComponentManager().
            searchComponentByType<SegmentCollisionComponent>(entityNum,
                                  Components_e::SEGMENT_COLLISION_COMPONENT);
    assert(collComp);
    return *collComp;
}

//===================================================================
MapCoordComponent &CollisionSystem::getMapComponent(uint32_t entityNum)
{
    MapCoordComponent *mapComp = stairwayToComponentManager().
            searchComponentByType<MapCoordComponent>(entityNum,
                                  Components_e::MAP_COORD_COMPONENT);
    assert(mapComp);
    return *mapComp;
}

//===================================================================
Direction_e getDirection(float diffX, float diffY)
{
    bool vert = (std::abs(diffX) > std::abs(diffY)) ? true : false;
    return vert ? ((diffY < EPSILON_FLOAT) ? Direction_e::SOUTH : Direction_e::NORTH) :
                  ((diffX < EPSILON_FLOAT) ? Direction_e::WEST : Direction_e::EAST);
}
