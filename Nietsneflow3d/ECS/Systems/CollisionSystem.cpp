#include "CollisionSystem.hpp"
#include "constants.hpp"
#include <ECS/Components/MapCoordComponent.hpp>
#include <ECS/Components/CollisionComponent.hpp>
#include <ECS/Components/CircleCollisionComponent.hpp>
#include <ECS/Components/RectangleCollisionComponent.hpp>
#include <ECS/Components/LineCollisionComponent.hpp>
#include <ECS/Components/MoveableComponent.hpp>
#include <CollisionUtils.hpp>
#include <PhysicalEngine.hpp>
#include <cassert>
#include <math.h>

using multiMapTagIt = std::multimap<CollisionTag_e, CollisionTag_e>::const_iterator;

//===================================================================
CollisionSystem::CollisionSystem()
{
    setUsedComponents();
    initArrayTag();
}

//===================================================================
void CollisionSystem::setUsedComponents()
{
    bAddComponentToSystem(Components_e::COLLISION_COMPONENT);
    bAddComponentToSystem(Components_e::MAP_COORD_COMPONENT);
}

//===================================================================
void CollisionSystem::execSystem()
{
    System::execSystem();
    for(uint32_t i = 0; i < mVectNumEntity.size(); ++i)
    {
        m_memPosActive = false;
        CollisionComponent *tagCompA = stairwayToComponentManager().
                searchComponentByType<CollisionComponent>(mVectNumEntity[i],
                                                         Components_e::COLLISION_COMPONENT);
        if(tagCompA->m_tag == CollisionTag_e::WALL_C ||
                tagCompA->m_tag == CollisionTag_e::OBJECT_C)
        {
            continue;
        }
        assert(tagCompA);
        for(uint32_t j = 0; j < mVectNumEntity.size(); ++j)
        {
            CollisionComponent *tagCompB = stairwayToComponentManager().
                    searchComponentByType<CollisionComponent>(mVectNumEntity[j],
                                                        Components_e::COLLISION_COMPONENT);
            assert(tagCompB);
            if(checkTag(tagCompA->m_tag, tagCompB->m_tag))
            {
                checkCollision(mVectNumEntity[i], mVectNumEntity[j],
                               tagCompA, tagCompB);
            }
        }
        postProcessBehavior();
    }
}

//===================================================================
void CollisionSystem::initArrayTag()
{
    m_tagArray.insert({PLAYER, WALL_C});
    m_tagArray.insert({PLAYER, ENEMY});
    m_tagArray.insert({PLAYER, BULLET_ENEMY});
    m_tagArray.insert({PLAYER, OBJECT_C});

    m_tagArray.insert({ENEMY, BULLET_PLAYER});
    m_tagArray.insert({ENEMY, PLAYER});
    m_tagArray.insert({ENEMY, WALL_C});

    m_tagArray.insert({WALL_C, PLAYER});
    m_tagArray.insert({WALL_C, ENEMY});
    m_tagArray.insert({WALL_C, BULLET_ENEMY});
    m_tagArray.insert({WALL_C, BULLET_PLAYER});

    m_tagArray.insert({BULLET_ENEMY, PLAYER});
    m_tagArray.insert({BULLET_ENEMY, WALL_C});

    m_tagArray.insert({BULLET_PLAYER, ENEMY});
    m_tagArray.insert({BULLET_PLAYER, WALL_C});

    m_tagArray.insert({OBJECT_C, PLAYER});
}

//===================================================================
void CollisionSystem::postProcessBehavior()
{
    if(m_memPosActive)
    {
        collisionEjectCircleRect(*m_memMapComp, m_memPosX,
                                 m_memPosY, m_memVelocity, false);
    }
}

//===================================================================
bool CollisionSystem::checkTag(CollisionTag_e entityTagA,
                               CollisionTag_e entityTagB)
{
    for(multiMapTagIt it = m_tagArray.find(entityTagA);
        it != m_tagArray.end() || it->first != entityTagA; ++it)
    {
        if(it->second == entityTagB)
        {
            return true;
        }
    }
    return false;
}

//===================================================================
void CollisionSystem::checkCollision(uint32_t entityNumA, uint32_t entityNumB,
                                     CollisionComponent *tagCompA, CollisionComponent *tagCompB)
{
    MapCoordComponent &mapCompA = getMapComponent(entityNumA),
            &mapCompB = getMapComponent(entityNumB);
    CollisionArgs args = {entityNumA, entityNumB,
                          tagCompA, tagCompB,
                          mapCompA, mapCompB};
    if(tagCompA->m_shape == CollisionShape_e::RECTANGLE_C)
    {
        checkCollisionFirstRect(args);
    }
    else if(tagCompA->m_shape == CollisionShape_e::CIRCLE)
    {
        checkCollisionFirstCircle(args);
    }
    else if(tagCompA->m_shape == CollisionShape_e::SEGMENT)
    {
        checkCollisionFirstLine(args);
    }
}

//===================================================================
void CollisionSystem::checkCollisionFirstRect(CollisionArgs &args)
{
    RectangleCollisionComponent &rectCompA = getRectangleComponent(args.entityNumA);
    bool collision = false;
    switch(args.tagCompB->m_shape)
    {
    case CollisionShape_e::RECTANGLE_C:
    {
        RectangleCollisionComponent &rectCompB = getRectangleComponent(args.entityNumB);
        collision = checkRectRectCollision(args.mapCompA.m_absoluteMapPositionPX, rectCompA.m_size,
                               args.mapCompB.m_absoluteMapPositionPX, rectCompB.m_size);
    }
        break;
    case CollisionShape_e::CIRCLE:
    {
        CircleCollisionComponent &circleCompB = getCircleComponent(args.entityNumB);
        collision = checkCircleRectCollision(args.mapCompB.m_absoluteMapPositionPX, circleCompB.m_ray,
                                 args.mapCompA.m_absoluteMapPositionPX, rectCompA.m_size);
    }
        break;
    case CollisionShape_e::SEGMENT:
    {
        SegmentCollisionComponent &segmentCompB = getSegmentComponent(args.entityNumB);
        collision = checkSegmentRectCollision(args.mapCompB.m_absoluteMapPositionPX, segmentCompB.m_secondPoint,
                               args.mapCompA.m_absoluteMapPositionPX, rectCompA.m_size);
    }
        break;
    }
}

//===================================================================
void CollisionSystem::checkCollisionFirstCircle(CollisionArgs &args)
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
    case CollisionShape_e::CIRCLE:
    {
        CircleCollisionComponent &circleCompB = getCircleComponent(args.entityNumB);
        collision = checkCircleCircleCollision(args.mapCompA.m_absoluteMapPositionPX, circleCompA.m_ray,
                                   args.mapCompB.m_absoluteMapPositionPX, circleCompB.m_ray);
        if(collision)
        {
            treatCollisionCircleCircle(args, circleCompA, circleCompB);
        }
    }
        break;
    case CollisionShape_e::SEGMENT:
    {
        SegmentCollisionComponent &segmentCompB = getSegmentComponent(args.entityNumB);
        collision = checkCircleSegmentCollision(args.mapCompA.m_absoluteMapPositionPX, circleCompA.m_ray,
                                 args.mapCompB.m_absoluteMapPositionPX, segmentCompB.m_secondPoint);
        if(collision)
        {
            treatCollisionCircleSegment(args, circleCompA, segmentCompB);
        }
    }
        break;
    }
}

//===================================================================
void CollisionSystem::checkCollisionFirstLine(CollisionArgs &args)
{
    bool collision = false;
    SegmentCollisionComponent &lineCompA = getSegmentComponent(args.entityNumA);
    switch(args.tagCompB->m_shape)
    {
    case CollisionShape_e::RECTANGLE_C:
    {
        RectangleCollisionComponent &rectCompB = getRectangleComponent(args.entityNumB);
        collision = checkSegmentRectCollision(args.mapCompA.m_absoluteMapPositionPX, lineCompA.m_secondPoint,
                               args.mapCompB.m_absoluteMapPositionPX, rectCompB.m_size);
    }
        break;
    case CollisionShape_e::CIRCLE:
    {
        CircleCollisionComponent &circleCompB = getCircleComponent(args.entityNumB);
        collision = checkCircleSegmentCollision(args.mapCompB.m_absoluteMapPositionPX, circleCompB.m_ray,
                                 args.mapCompA.m_absoluteMapPositionPX, lineCompA.m_secondPoint);
    }
        break;
    case CollisionShape_e::SEGMENT:
    {
//        LineCollisionComponent &lineCompB = getLineComponent(args.entityNumB);
//        checkLineLineCollision(args.mapCompA.m_absoluteMapPositionPX, lineCompA.m_secondPoint,
//                               args.mapCompB.m_absoluteMapPositionPX, lineCompB.m_secondPoint);
    }
        break;
    }
}

//===================================================================
void CollisionSystem::treatCollisionCircleRect(CollisionArgs &args,
                                               const CircleCollisionComponent &circleCollA,
                                               const RectangleCollisionComponent &rectCollB)
{
    MapCoordComponent &mapComp = getMapComponent(args.entityNumA);
    if(args.tagCompA->m_tag == CollisionTag_e::PLAYER ||
            args.tagCompA->m_tag == CollisionTag_e::ENEMY)
    {
        MoveableComponent *moveComp = stairwayToComponentManager().
                searchComponentByType<MoveableComponent>(args.entityNumA,
                                      Components_e::MOVEABLE_COMPONENT);
        assert(moveComp);
        float radDegree = getRadiantAngle(moveComp->m_currentDegreeDirection);
        float circlePosX = args.mapCompA.m_absoluteMapPositionPX.first;
        float circlePosY = args.mapCompA.m_absoluteMapPositionPX.second;
        float elementPosX = args.mapCompB.m_absoluteMapPositionPX.first;
        float elementPosY = args.mapCompB.m_absoluteMapPositionPX.second;
        float elementSecondPosX = elementPosX + rectCollB.m_size.first;
        float elementSecondPosY = elementPosY + rectCollB.m_size.second;
        float diffX, diffY;

        bool angleBehavior = false;
        //collision on angle of rect
        if(circlePosX < elementPosX || circlePosX > elementSecondPosX &&
                circlePosY < elementPosY || circlePosY > elementSecondPosY)
        {
            std::cerr << "elementPosX " << elementPosX
                      << " circlePosX " << circlePosX
                      << " elementSecondPosX " << elementSecondPosX
                      << "\n elementPosY " << elementPosY
                      << " circlePosY " << circlePosY
                      << " elementSecondPosY " << elementSecondPosY << "\n\n";
            //memorize initial position
            m_memPosX = circlePosX;
            m_memPosY = circlePosY;
            m_memVelocity = moveComp->m_velocity;
            m_memMapComp = &mapComp;
            angleBehavior = true;
            m_memPosActive = true;
        }
        if(std::cos(radDegree) > 0.0f)
        {
            diffX = -circleCollA.m_ray + (elementPosX - circlePosX);
        }
        else
        {
            diffX = circleCollA.m_ray - (circlePosX - elementSecondPosX);
        }
        if(std::sin(radDegree) < 0.0f)
        {
            diffY = -circleCollA.m_ray + (elementPosY - circlePosY);
        }
        else
        {
            diffY = circleCollA.m_ray - (circlePosY - elementSecondPosY);
        }
//        if(!angleBehavior)
        {
            collisionEjectCircleRect(mapComp, diffX, diffY,
                                     moveComp->m_velocity, angleBehavior);
        }
    }
}

//===================================================================
void CollisionSystem::treatCollisionCircleCircle(CollisionArgs &args,
                                                 const CircleCollisionComponent &circleCollA,
                                                 const CircleCollisionComponent &circleCollB)
{

}

//===================================================================
void CollisionSystem::treatCollisionCircleSegment(CollisionArgs &args,
                                                  const CircleCollisionComponent &circleCollA,
                                                  const SegmentCollisionComponent &segmCollB)
{

}

//===================================================================
void CollisionSystem::collisionEjectCircleRect(MapCoordComponent &mapComp,
                                               float diffX, float diffY,
                                               float velocity, bool angleBehavior)
{
    //if previous collision angle rect
    if(!angleBehavior && m_memPosActive)
    {
        //reinit position
//        mapComp.m_absoluteMapPositionPX = {m_memPosX, m_memPosY};
    }
    m_memPosActive = false;
    if(std::abs(diffX) < std::abs(diffY))
    {
        //quick fix
        std::cerr << diffX << " diffX\n";
        mapComp.m_absoluteMapPositionPX.first += diffX;
    }
    else
    {
        if(std::abs(diffY) <= velocity)
        {
            std::cerr << diffY << " diffY\n";
            mapComp.m_absoluteMapPositionPX.second += diffY;
        }
    }
}

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
                                  Components_e::LINE_COLLISION_COMPONENT);
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
