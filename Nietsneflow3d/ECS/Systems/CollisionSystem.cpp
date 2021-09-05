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
#include <ECS/Components/ImpactShotComponent.hpp>
#include <ECS/Components/MemSpriteDataComponent.hpp>
#include <ECS/Components/SpriteTextureComponent.hpp>
#include <ECS/Components/WeaponComponent.hpp>
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
    for(uint32_t i = 0; i < mVectNumEntity.size(); ++i)
    {
        GeneralCollisionComponent *tagCompA = stairwayToComponentManager().
                searchComponentByType<GeneralCollisionComponent>(mVectNumEntity[i],
                                                                 Components_e::GENERAL_COLLISION_COMPONENT);
        assert(tagCompA);
        if(!tagCompA->m_active)
        {
            continue;
        }
        if(tagCompA->m_tagA == CollisionTag_e::WALL_CT ||
                tagCompA->m_tagA == CollisionTag_e::OBJECT_CT ||
                tagCompA->m_tagA == CollisionTag_e::DOOR_CT)
        {
            continue;
        }
        if(tagCompA->m_shape == CollisionShape_e::SEGMENT_C &&
                (tagCompA->m_tagA == CollisionTag_e::BULLET_PLAYER_CT ||
                 tagCompA->m_tagA == CollisionTag_e::BULLET_ENEMY_CT))
        {
            m_memDistCurrentBulletColl.second = EPSILON_FLOAT;
            segmentCompA = stairwayToComponentManager().
                    searchComponentByType<SegmentCollisionComponent>(mVectNumEntity[i],
                                                                     Components_e::SEGMENT_COLLISION_COMPONENT);
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
                    searchComponentByType<PlayerConfComponent>(mVectNumEntity[i],
                                                               Components_e::PLAYER_CONF_COMPONENT);
            assert(playerComp);
            playerComp->m_crushMem.first = false;
            if(!playerComp->m_crush)
            {
                playerComp->m_frozen = false;
            }
            else
            {
                playerComp->m_crush = false;
            }
        }
        for(uint32_t j = 0; j < mVectNumEntity.size(); ++j)
        {
            if(i == j)
            {
                continue;
            }
            GeneralCollisionComponent *tagCompB = stairwayToComponentManager().
                    searchComponentByType<GeneralCollisionComponent>(mVectNumEntity[j],
                                                        Components_e::GENERAL_COLLISION_COMPONENT);
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
        if(tagCompA->m_tagA == CollisionTag_e::PLAYER_ACTION_CT)
        {
            tagCompA->m_active = false;
        }
        if(segmentCompA && m_memDistCurrentBulletColl.second > EPSILON_FLOAT)
        {
            if(m_memDistCurrentBulletColl.first)
            {
                m_vectMemShots.emplace_back(pairUI_t{mVectNumEntity[i],
                                                     (*m_memDistCurrentBulletColl.first)});
            }
        }
        treatSegmentShots();
        m_vectMemShots.clear();
    }
}

//===================================================================
void CollisionSystem::treatEnemyShooted(uint32_t enemyEntityNum, uint32_t damage)
{
    EnemyConfComponent *enemyConfCompB = stairwayToComponentManager().
            searchComponentByType<EnemyConfComponent>(enemyEntityNum,
                                                      Components_e::ENEMY_CONF_COMPONENT);
    TimerComponent *timerComp = stairwayToComponentManager().
            searchComponentByType<TimerComponent>(enemyEntityNum,
                                                      Components_e::TIMER_COMPONENT);
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
        rmCollisionMaskEntity(enemyEntityNum);
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
                tagCompTarget->m_tagA == CollisionTag_e::DOOR_CT ||
                tagCompTarget->m_tagA == CollisionTag_e::STATIC_SET_CT)
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
            treatEnemyShooted(m_vectMemShots[i].second, shotConfComp->m_damage);
            tagCompBullet->m_active = false;
        }
        else if(tagCompBullet->m_tagA == CollisionTag_e::BULLET_ENEMY_CT)
        {
            PlayerConfComponent *playerConfCompB = stairwayToComponentManager().
                    searchComponentByType<PlayerConfComponent>(m_vectMemShots[i].second,
                                                               Components_e::PLAYER_CONF_COMPONENT);
            assert(playerConfCompB);
            ShotConfComponent *shotConfComp = stairwayToComponentManager().
                    searchComponentByType<ShotConfComponent>(m_vectMemShots[i].first,
                                                               Components_e::SHOT_CONF_COMPONENT);
            assert(shotConfComp);
            playerConfCompB->takeDamage(shotConfComp->m_damage);
        }
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

    m_tagArray.insert({CollisionTag_e::PLAYER_ACTION_CT, CollisionTag_e::DOOR_CT});
    m_tagArray.insert({CollisionTag_e::PLAYER_ACTION_CT, CollisionTag_e::EXIT_CT});
    m_tagArray.insert({CollisionTag_e::PLAYER_ACTION_CT, CollisionTag_e::TRIGGER_CT});
    m_tagArray.insert({CollisionTag_e::HIT_PLAYER_CT, CollisionTag_e::ENEMY_CT});

    m_tagArray.insert({CollisionTag_e::ENEMY_CT, CollisionTag_e::PLAYER_CT});
    m_tagArray.insert({CollisionTag_e::ENEMY_CT, CollisionTag_e::WALL_CT});
    m_tagArray.insert({CollisionTag_e::ENEMY_CT, CollisionTag_e::DOOR_CT});
    m_tagArray.insert({CollisionTag_e::ENEMY_CT, CollisionTag_e::DOOR_CT});
    m_tagArray.insert({CollisionTag_e::ENEMY_CT, CollisionTag_e::STATIC_SET_CT});
//    m_tagArray.insert({CollisionTag_e::ENEMY_CT, CollisionTag_e::ENEMY_CT});

    m_tagArray.insert({CollisionTag_e::WALL_CT, CollisionTag_e::PLAYER_CT});
    m_tagArray.insert({CollisionTag_e::WALL_CT, CollisionTag_e::ENEMY_CT});

    m_tagArray.insert({CollisionTag_e::DOOR_CT, CollisionTag_e::PLAYER_CT});
    m_tagArray.insert({CollisionTag_e::DOOR_CT, CollisionTag_e::ENEMY_CT});

    //bullets collision with walls and doors are treated by raycasting
    m_tagArray.insert({CollisionTag_e::BULLET_ENEMY_CT, CollisionTag_e::PLAYER_CT});
    m_tagArray.insert({CollisionTag_e::BULLET_ENEMY_CT, CollisionTag_e::WALL_CT});
    m_tagArray.insert({CollisionTag_e::BULLET_ENEMY_CT, CollisionTag_e::DOOR_CT});
//    m_tagArray.insert({CollisionTag_e::BULLET_ENEMY_CT, CollisionTag_e::ENEMY_CT});

    m_tagArray.insert({CollisionTag_e::BULLET_PLAYER_CT, CollisionTag_e::ENEMY_CT});
    m_tagArray.insert({CollisionTag_e::BULLET_PLAYER_CT, CollisionTag_e::WALL_CT});
    m_tagArray.insert({CollisionTag_e::BULLET_PLAYER_CT, CollisionTag_e::DOOR_CT});

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
                    }
                    else
                    {
                        collisionCircleRectEject(args, circleCompA, rectCompB);
                    }
                }
                else
                {
                    collisionCircleRectEject(args, circleCompA, rectCompB);
                }
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
                    treatEnemyShooted(args.entityNumB, shotConfComp->m_damage);
                }
            }
            else if((args.tagCompA->m_tagA == CollisionTag_e::PLAYER_CT ||
                    args.tagCompA->m_tagA == CollisionTag_e::ENEMY_CT) &&
                    (args.tagCompB->m_tagA == CollisionTag_e::WALL_CT ||
                     args.tagCompB->m_tagA == CollisionTag_e::PLAYER_CT ||
                     args.tagCompB->m_tagA == CollisionTag_e::ENEMY_CT ||
                     args.tagCompB->m_tagA == CollisionTag_e::STATIC_SET_CT))
            {
                collisionCircleCircleEject(args, circleCompA, circleCompB);
            }
            else if(args.tagCompA->m_tagA == CollisionTag_e::PLAYER_CT)
            {
                if(args.tagCompB->m_tagA == CollisionTag_e::OBJECT_CT)
                {
                    treatPlayerPickObject(args);
                }
                if(args.tagCompB->m_tagA == CollisionTag_e::TRIGGER_CT)
                {
                    TriggerComponent *triggerComp = stairwayToComponentManager().
                            searchComponentByType<TriggerComponent>(args.entityNumB, Components_e::TRIGGER_COMPONENT);
                    assert(triggerComp);
                    triggerComp->m_actionned = true;
                }
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
    if((args.tagCompA->m_tagA == CollisionTag_e::BULLET_ENEMY_CT) ||
            (args.tagCompA->m_tagA == CollisionTag_e::BULLET_PLAYER_CT))
    {
        bool destruct = false;
        //limit level case
        if(args.mapCompA.m_absoluteMapPositionPX.first < LEVEL_THIRD_TILE_SIZE_PX ||
                args.mapCompA.m_absoluteMapPositionPX.second < LEVEL_THIRD_TILE_SIZE_PX)
        {
            destruct = true;
        }
        if(destruct || collision)
        {
            ShotConfComponent *shotConfComp = stairwayToComponentManager().
                    searchComponentByType<ShotConfComponent>(args.entityNumA,
                                                               Components_e::SHOT_CONF_COMPONENT);
            assert(shotConfComp);
            if(shotConfComp->m_destructPhase)
            {
                return;
            }
            TimerComponent *timerComp = stairwayToComponentManager().
                    searchComponentByType<TimerComponent>(args.entityNumA, Components_e::TIMER_COMPONENT);
            assert(timerComp);
            timerComp->m_clockB = std::chrono::system_clock::now();
            shotConfComp->m_destructPhase = true;
            shotConfComp->m_spriteShotNum = 1;
            if(destruct)
            {
                return;
            }
            if(args.tagCompA->m_tagA == CollisionTag_e::BULLET_PLAYER_CT &&
                    args.tagCompB->m_tagA == CollisionTag_e::ENEMY_CT)
            {
                treatEnemyShooted(args.entityNumB, shotConfComp->m_damage);
            }
            else if(args.tagCompA->m_tagA == CollisionTag_e::BULLET_ENEMY_CT &&
                    args.tagCompB->m_tagA == CollisionTag_e::PLAYER_CT)
            {
                PlayerConfComponent * playerConf = stairwayToComponentManager().
                        searchComponentByType<PlayerConfComponent>(args.entityNumB,
                                              Components_e::PLAYER_CONF_COMPONENT);
                assert(playerConf);
                playerConf->takeDamage(shotConfComp->m_damage);
            }
        }
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
    m_vectEntitiesToDelete.push_back(args.entityNumB);
}

//===================================================================
bool CollisionSystem::treatCrushing(const CollisionArgs &args, float diffX, float diffY)
{
    PlayerConfComponent *playerComp = stairwayToComponentManager().
            searchComponentByType<PlayerConfComponent>(args.entityNumA, Components_e::PLAYER_CONF_COMPONENT);
    if(playerComp && !playerComp->m_crush)
    {
        //mem crush if player is eject from a moveable wall
        if(!playerComp->m_crushMem.first)
        {
            if(args.tagCompB->m_tagA == CollisionTag_e::WALL_CT)
            {
                playerComp->m_crushMem = {true, getDirection(diffX, diffY)};
            }
        }
        //if crush
        else
        {
            if(args.tagCompB->m_tagA == CollisionTag_e::WALL_CT)
            {
                Direction_e dir = getDirection(diffX, diffY);
                if((playerComp->m_crushMem.second == Direction_e::EAST &&
                    dir == Direction_e::WEST) ||
                        (playerComp->m_crushMem.second == Direction_e::WEST &&
                         dir == Direction_e::EAST) ||
                        (playerComp->m_crushMem.second == Direction_e::NORTH &&
                         dir == Direction_e::SOUTH) ||
                        (playerComp->m_crushMem.second == Direction_e::SOUTH &&
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
void CollisionSystem::collisionCircleRectEject(CollisionArgs &args,
                                               const CircleCollisionComponent &circleCollA,
                                               const RectangleCollisionComponent &rectCollB)
{
    MapCoordComponent &mapComp = getMapComponent(args.entityNumA);
    MoveableComponent *moveComp = stairwayToComponentManager().
            searchComponentByType<MoveableComponent>(args.entityNumA,
                                                     Components_e::MOVEABLE_COMPONENT);
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
    diffY = getVerticalCircleRectEject({circlePosX, circlePosY,
                                        pointElementX, elementPosY,
                                        elementSecondPosY,
                                        circleCollA.m_ray, radiantObserverAngle,
                                        angleBehavior}, limitEjectY);
    if(!limitEjectY)
    {
        diffX = getHorizontalCircleRectEject({circlePosX, circlePosY, pointElementY,
                                              elementPosX, elementSecondPosX,
                                              circleCollA.m_ray, radiantObserverAngle,
                                              angleBehavior}, limitEjectX);
    }
    treatCrushing(args, diffX, diffY);
    collisionEject(mapComp, diffX, diffY, limitEjectY, limitEjectX);
}

//===================================================================
float CollisionSystem::getVerticalCircleRectEject(const EjectYArgs& args, bool &limitEject)
{
    float adj, diffYA = EPSILON_FLOAT, diffYB;
    if(std::abs(std::sin(args.radDegree)) < 0.01f &&
            (args.angleMode || args.circlePosY < args.elementPosY ||
             args.circlePosY > args.elementSecondPosY))
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
        //DOWN
        if(std::sin(args.radDegree) < EPSILON_FLOAT)
        {
            diffYA -= (args.elementPosY - args.circlePosY);
            diffYA = -diffYA;
        }
        //UP
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
void CollisionSystem::collisionEject(MapCoordComponent &mapComp,
                                     float diffX, float diffY, bool limitEjectY, bool limitEjectX)
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
    if(args.tagCompA->m_tagA == CollisionTag_e::PLAYER_CT ||
            args.tagCompA->m_tagA == CollisionTag_e::ENEMY_CT)
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
