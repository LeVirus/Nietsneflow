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
#include <ECS/Components/EnemyConfComponent.hpp>
#include <ECS/Systems/FirstPersonDisplaySystem.hpp>
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
    m_vectMemShots.clear();
    for(uint32_t i = 0; i < mVectNumEntity.size(); ++i)
    {
        GeneralCollisionComponent *tagCompA = stairwayToComponentManager().
                searchComponentByType<GeneralCollisionComponent>(mVectNumEntity[i],
                                                                 Components_e::GENERAL_COLLISION_COMPONENT);
        if(!tagCompA->m_active)
        {
            continue;
        }
        if(tagCompA->m_tag == CollisionTag_e::WALL_CT ||
                tagCompA->m_tag == CollisionTag_e::OBJECT_CT ||
                tagCompA->m_tag == CollisionTag_e::DOOR_CT)
        {
            continue;
        }
        if(tagCompA->m_shape == CollisionShape_e::SEGMENT_C &&
                (tagCompA->m_tag == CollisionTag_e::BULLET_PLAYER_CT ||
                 tagCompA->m_tag == CollisionTag_e::BULLET_ENEMY_CT))
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
        assert(tagCompA);
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
            if(!checkTag(tagCompA->m_tag, tagCompB->m_tag))
            {
                continue;
            }
            treatCollision(mVectNumEntity[i], mVectNumEntity[j],
                           tagCompA, tagCompB);
            if(m_memDistCurrentBulletColl.second > EPSILON_FLOAT)
            {
                m_vectMemShots.emplace_back(tupleShot_t{segmentCompA, tagCompA,
                                                        m_memDistCurrentBulletColl.first});
            }
        }
        treatShots();
    }
}

//===================================================================
void CollisionSystem::treatShots()
{
    for(uint32_t i = 0; i < m_vectMemShots.size(); ++i)
    {
        if(std::get<1>(m_vectMemShots[i])->m_tag == CollisionTag_e::BULLET_PLAYER_CT)
        {
            EnemyConfComponent *enemyConfCompB = stairwayToComponentManager().
                    searchComponentByType<EnemyConfComponent>(std::get<2>(m_vectMemShots[i]),
                                                              Components_e::ENEMY_CONF_COMPONENT);
            assert(enemyConfCompB);
            //if enemy dead
            if(!enemyConfCompB->takeDamage(1))
            {
                enemyConfCompB->m_behaviourMode = EnemyBehaviourMode_e::DEAD;
                rmCollisionMaskEntity(std::get<2>(m_vectMemShots[i]));
            }
            std::get<1>(m_vectMemShots[i])->m_active = false;
        }
        else if(std::get<1>(m_vectMemShots[i])->m_tag == CollisionTag_e::BULLET_ENEMY_CT)
        {
            PlayerConfComponent *playerConfCompB = stairwayToComponentManager().
                    searchComponentByType<PlayerConfComponent>(std::get<2>(m_vectMemShots[i]),
                                                               Components_e::PLAYER_CONF_COMPONENT);
            assert(playerConfCompB);
            //if player is dead
            if(!playerConfCompB->takeDamage(5))
            {
                //player dead
            }
        }
    }
}

//===================================================================
void CollisionSystem::rmCollisionMaskEntity(uint32_t numEntity)
{
    GeneralCollisionComponent *tagComp = stairwayToComponentManager().
            searchComponentByType<GeneralCollisionComponent>(numEntity,
                                                             Components_e::GENERAL_COLLISION_COMPONENT);
    assert(tagComp);
    tagComp->m_tag = CollisionTag_e::GHOST_CT;
}

//===================================================================
void CollisionSystem::initArrayTag()
{
    m_tagArray.insert({CollisionTag_e::PLAYER_CT, CollisionTag_e::WALL_CT});
    m_tagArray.insert({CollisionTag_e::PLAYER_CT, CollisionTag_e::DOOR_CT});
    m_tagArray.insert({CollisionTag_e::PLAYER_CT, CollisionTag_e::ENEMY_CT});
    m_tagArray.insert({CollisionTag_e::PLAYER_CT, CollisionTag_e::OBJECT_CT});

    m_tagArray.insert({CollisionTag_e::ENEMY_CT, CollisionTag_e::PLAYER_CT});
    m_tagArray.insert({CollisionTag_e::ENEMY_CT, CollisionTag_e::WALL_CT});
    m_tagArray.insert({CollisionTag_e::ENEMY_CT, CollisionTag_e::DOOR_CT});
//    m_tagArray.insert({CollisionTag_e::ENEMY_CT, CollisionTag_e::ENEMY_CT});

    m_tagArray.insert({CollisionTag_e::WALL_CT, CollisionTag_e::PLAYER_CT});
    m_tagArray.insert({CollisionTag_e::WALL_CT, CollisionTag_e::ENEMY_CT});

    m_tagArray.insert({CollisionTag_e::DOOR_CT, CollisionTag_e::PLAYER_CT});
    m_tagArray.insert({CollisionTag_e::DOOR_CT, CollisionTag_e::ENEMY_CT});

    //bullets collision with walls and doors are treated by raycasting
    m_tagArray.insert({CollisionTag_e::BULLET_ENEMY_CT, CollisionTag_e::PLAYER_CT});
//    m_tagArray.insert({CollisionTag_e::BULLET_ENEMY_CT, CollisionTag_e::ENEMY_CT});

    m_tagArray.insert({CollisionTag_e::BULLET_PLAYER_CT, CollisionTag_e::ENEMY_CT});

    m_tagArray.insert({CollisionTag_e::OBJECT_CT, CollisionTag_e::PLAYER_CT});
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
                                     GeneralCollisionComponent *tagCompA,
                                     GeneralCollisionComponent *tagCompB)
{
//    if(tagCompA->m_shape == CollisionShape_e::RECTANGLE_C)
//    {
//        checkCollisionFirstRect(args);
//    }
    if(tagCompA->m_shape == CollisionShape_e::CIRCLE_C)
    {
        CollisionArgs args = {entityNumA, entityNumB,
                              tagCompA, tagCompB,
                              getMapComponent(entityNumA), getMapComponent(entityNumB)};
        treatCollisionFirstCircle(args);
    }
    else if(tagCompA->m_shape == CollisionShape_e::SEGMENT_C)
    {
        assert(tagCompA->m_tag == CollisionTag_e::BULLET_PLAYER_CT ||
               tagCompA->m_tag == CollisionTag_e::BULLET_ENEMY_CT);
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
void CollisionSystem::treatCollisionFirstCircle(CollisionArgs &args)
{
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
            treatCollisionCircleRect(args, circleCompA, rectCompB);
        }
    }
        break;
    case CollisionShape_e::CIRCLE_C:
    {
        CircleCollisionComponent &circleCompB = getCircleComponent(args.entityNumB);
        collision = checkCircleCircleCollision(args.mapCompA.m_absoluteMapPositionPX, circleCompA.m_ray,
                                               args.mapCompB.m_absoluteMapPositionPX,
                                               circleCompB.m_ray);
        if(collision)
        {
            treatCollisionCircleCircle(args, circleCompA, circleCompB);
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
            float distance = getDistance(segmentCompA.m_points.first,
                                         mapCompB.m_absoluteMapPositionPX);
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
    std::optional<float> verticalLeadCoef = getLeadCoef(radiantAngle, false);
    std::optional<float> lateralLeadCoef = getLeadCoef(radiantAngle, true);
    bool lateral, textLateral, textFace;
    std::optional<pairUI_t> currentCoord;
    std::optional<ElementRaycast> element;
    segmentCompA.m_points.second = segmentCompA.m_points.first;
    for(uint32_t k = 0; k < 20; ++k)//limit distance
    {
        segmentCompA.m_points.second = getLimitPointRayCasting(segmentCompA.m_points.second,
                                                               radiantAngle, lateralLeadCoef,
                                                               verticalLeadCoef, lateral);
        currentCoord = getCorrectedCoord(segmentCompA.m_points.second, lateral, radiantAngle);
        if(!currentCoord)
        {
            break;
        }
        element = Level::getElementCase(*currentCoord);
        if(element)
        {
            if(element->m_type == LevelCaseType_e::WALL_LC)
            {
                break;
            }
            else if(element->m_type == LevelCaseType_e::DOOR_LC)
            {
                DoorComponent *doorComp = stairwayToComponentManager().
                        searchComponentByType<DoorComponent>(element->m_numEntity, Components_e::DOOR_COMPONENT);
                assert(doorComp);
                RectangleCollisionComponent *rectComp = stairwayToComponentManager().
                        searchComponentByType<RectangleCollisionComponent>(element->m_numEntity, Components_e::RECTANGLE_COLLISION_COMPONENT);
                assert(rectComp);
                MapCoordComponent *mapComp = stairwayToComponentManager().
                        searchComponentByType<MapCoordComponent>(element->m_numEntity, Components_e::MAP_COORD_COMPONENT);
                assert(mapComp);
                //first case x pos limit second y pos limit
                pairFloat_t doorPos[2] = {{mapComp->m_absoluteMapPositionPX.first,
                                           mapComp->m_absoluteMapPositionPX.first +
                                           rectComp->m_size.first},
                                          {mapComp->m_absoluteMapPositionPX.second,
                                           mapComp->m_absoluteMapPositionPX.second +
                                           rectComp->m_size.second}};
                if(treatDisplayDoor(radiantAngle, doorComp->m_vertical,
                                      segmentCompA.m_points.second,
                                      doorPos, verticalLeadCoef, lateralLeadCoef,
                                      textLateral, textFace))
                {
                    break;
                }
            }
        }
    }
}

//===================================================================
void CollisionSystem::treatCollisionCircleRect(CollisionArgs &args,
                                               const CircleCollisionComponent &circleCollA,
                                               const RectangleCollisionComponent &rectCollB)
{
    if(args.tagCompA->m_tag == CollisionTag_e::PLAYER_CT ||
            args.tagCompA->m_tag == CollisionTag_e::ENEMY_CT)
    {
        MapCoordComponent &mapComp = getMapComponent(args.entityNumA);
        MoveableComponent *moveComp = stairwayToComponentManager().
                searchComponentByType<MoveableComponent>(args.entityNumA,
                                      Components_e::MOVEABLE_COMPONENT);
        assert(moveComp);
        float radDegree = getRadiantAngle(moveComp->m_currentDegreeMoveDirection);
        float circlePosX = args.mapCompA.m_absoluteMapPositionPX.first;
        float circlePosY = args.mapCompA.m_absoluteMapPositionPX.second;
        float elementPosX = args.mapCompB.m_absoluteMapPositionPX.first;
        float elementPosY = args.mapCompB.m_absoluteMapPositionPX.second;
        float elementSecondPosX = elementPosX + rectCollB.m_size.first;
        float elementSecondPosY = elementPosY + rectCollB.m_size.second;

        bool angleBehavior = false;
        //collision on angle of rect
        if((circlePosX < elementPosX || circlePosX > elementSecondPosX) &&
                (circlePosY < elementPosY || circlePosY > elementSecondPosY))
        {
            angleBehavior = true;
        }

        float pointElementX = circlePosX < elementPosX ?
                    elementPosX : elementSecondPosX;
        float pointElementY = circlePosY < elementPosY ?
                    elementPosY : elementSecondPosY;

        float diffY = getVerticalCircleRectEject({circlePosX, circlePosY, pointElementX,
                                  elementPosY,
                                  elementSecondPosY,
                                  circleCollA.m_ray, radDegree,
                                  angleBehavior});
        float diffX = getHorizontalCircleRectEject({circlePosX, circlePosY, pointElementY,
                                  elementPosX,
                                  elementSecondPosX,
                                  circleCollA.m_ray, radDegree,
                                  angleBehavior});
        collisionEject(mapComp, diffX, diffY);
        if(args.tagCompA->m_tag == CollisionTag_e::ENEMY_CT)
        {
            EnemyConfComponent *enemyComp = stairwayToComponentManager().
                    searchComponentByType<EnemyConfComponent>(args.entityNumA,
                                                             Components_e::ENEMY_CONF_COMPONENT);
            assert(enemyComp);
            enemyComp->m_wallTouch.first = true;
            if(std::abs(diffX) > std::abs(diffY))
            {
                enemyComp->m_wallTouch.second = (diffY < EPSILON_FLOAT) ?
                            Direction_e::NORTH : Direction_e::SOUTH;
            }
            else
            {
                enemyComp->m_wallTouch.second = (diffX < EPSILON_FLOAT) ?
                            Direction_e::EAST : Direction_e::WEST;
            }
            return;
        }
        PlayerConfComponent *playerComp = stairwayToComponentManager().
                searchComponentByType<PlayerConfComponent>(args.entityNumA,
                                                         Components_e::PLAYER_CONF_COMPONENT);
        assert(playerComp);
        if(args.tagCompB->m_tag == CollisionTag_e::DOOR_CT)
        {
            if(playerComp->m_playerAction)
            {
                DoorComponent *doorComp = stairwayToComponentManager().
                        searchComponentByType<DoorComponent>(args.entityNumB,
                                                             Components_e::DOOR_COMPONENT);
                assert(doorComp);
                if(doorComp->m_currentState == DoorState_e::STATIC_CLOSED ||
                        doorComp->m_currentState == DoorState_e::MOVE_CLOSE)
                {
                    doorComp->m_currentState = DoorState_e::MOVE_OPEN;
                }
            }
        }
    }
}

//===================================================================
float CollisionSystem::getVerticalCircleRectEject(const EjectYArgs& args)
{
    float adj, diffY = 0.0f;//avoid warning
    if(args.angleMode)
    {
        adj = std::abs(args.circlePosX - args.elementPosX);
        diffY = getRectTriangleSide(adj, args.ray);
    }
    //DOWN
    if(std::sin(args.radDegree) < 0.0f)
    {
        if(args.angleMode)
        {
            diffY -= (args.elementPosY - args.circlePosY);
            diffY = -diffY;
        }
        else
        {
            diffY = -args.ray + (args.elementPosY - args.circlePosY);
        }
    }
    //UP
    else
    {
        if(args.angleMode)
        {
            diffY -= (args.circlePosY - args.elementSecondPosY);
        }
        else
        {
            diffY = args.ray - (args.circlePosY - args.elementSecondPosY);
        }
    }
    return diffY;
}

//===================================================================
float CollisionSystem::getHorizontalCircleRectEject(const EjectXArgs &args)
{
    float adj, diffX = 0.0f;//avoid warning
    if(args.angleMode)
    {
        adj = std::abs(args.circlePosY - args.elementPosY);
        diffX = getRectTriangleSide(adj, args.ray);
    }
    //RIGHT
    if(std::cos(args.radDegree) > 0.0f)
    {
        if(args.angleMode)
        {
            diffX -= (args.elementPosX - args.circlePosX);
            diffX = -diffX;
        }
        else
        {
            diffX = -args.ray + (args.elementPosX - args.circlePosX);
        }
    }
    //LEFT
    else
    {
        if(args.angleMode)
        {
            diffX -= (args.circlePosX - args.elementSecondPosX);
        }
        else
        {
            diffX = args.ray - (args.circlePosX - args.elementSecondPosX);
        }
    }
    return diffX;
}

//===================================================================
void CollisionSystem::collisionEject(MapCoordComponent &mapComp,
                                     float diffX, float diffY)
{
    if(std::abs(diffX) < std::abs(diffY))
    {
        mapComp.m_absoluteMapPositionPX.first += diffX;
    }
    else
    {
        mapComp.m_absoluteMapPositionPX.second += diffY;
    }
}

//===================================================================
void CollisionSystem::treatCollisionCircleCircle(CollisionArgs &args,
                                                 const CircleCollisionComponent &circleCollA,
                                                 const CircleCollisionComponent &circleCollB)
{
    if(args.tagCompA->m_tag == CollisionTag_e::PLAYER_CT)
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
