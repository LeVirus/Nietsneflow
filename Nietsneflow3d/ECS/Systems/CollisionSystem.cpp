#include "CollisionSystem.hpp"
#include "constants.hpp"
#include <ECS/Components/MapCoordComponent.hpp>
#include <ECS/Components/CollisionComponent.hpp>
#include <ECS/Components/CircleCollisionComponent.hpp>
#include <ECS/Components/RectangleCollisionComponent.hpp>
#include <ECS/Components/LineCollisionComponent.hpp>
#include <CollisionUtils.hpp>
#include <cassert>

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
    bAddComponentToSystem(Components_e::TAG_COMPONENT);
    bAddComponentToSystem(Components_e::MAP_COORD_COMPONENT);
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
void CollisionSystem::treatCollision(uint32_t entityNumA, uint32_t entityNumB,
                                     CollisionComponent *tagCompA, CollisionComponent *tagCompB)
{
    MapCoordComponent &mapCompA = getMapComponent(entityNumA),
            &mapCompB = getMapComponent(entityNumB);
    const pairFloat_t &posOriginA = mapCompA.m_absoluteMapPositionPX,
    &posOriginB = mapCompB.m_absoluteMapPositionPX;
    if(tagCompA->m_shape == CollisionShape_e::RECTANGLE_C)
    {
        RectangleCollisionComponent &rectCompA = getRectangleComponent(entityNumA);
        switch(tagCompB->m_shape)
        {
        case CollisionShape_e::RECTANGLE_C:
        {
            RectangleCollisionComponent &rectCompB = getRectangleComponent(entityNumB);
            checkRectRectCollision(posOriginA, rectCompA.m_size,
                                   posOriginB, rectCompB.m_size);
        }
            break;
        case CollisionShape_e::CIRCLE:
        {
            CircleCollisionComponent &circleCompB = getCircleComponent(entityNumB);
            checkCircleRectCollision(posOriginB, circleCompB.m_ray,
                                     posOriginA, rectCompA.m_size);
        }
            break;
        case CollisionShape_e::LINE:
        {
            LineCollisionComponent &lineCompB = getLineComponent(entityNumB);
            checkLineRectCollision(posOriginB, lineCompB.m_secondPoint,
                                   posOriginA, rectCompA.m_size);
        }
            break;
        }
    }
    else if(tagCompA->m_shape == CollisionShape_e::CIRCLE)
    {
        CircleCollisionComponent &circleCompA = getCircleComponent(entityNumA);
        switch(tagCompB->m_shape)
        {
        case CollisionShape_e::RECTANGLE_C:
        {
            RectangleCollisionComponent &rectCompB = getRectangleComponent(entityNumA);
            checkCircleRectCollision(posOriginA, circleCompA.m_ray,
                                     posOriginB, rectCompB.m_size);
        }
            break;
        case CollisionShape_e::CIRCLE:
        {
            CircleCollisionComponent &circleCompB = getCircleComponent(entityNumB);
            checkCircleCircleCollision(posOriginA, circleCompA.m_ray,
                                       posOriginB, circleCompB.m_ray);
        }
            break;
        case CollisionShape_e::LINE:
        {
            LineCollisionComponent &lineCompB = getLineComponent(entityNumB);
            checkCircleLineCollision(posOriginA, circleCompA.m_ray,
                                     posOriginB, lineCompB.m_secondPoint);
        }
            break;
        }
    }
    else if(tagCompA->m_shape == CollisionShape_e::LINE)
    {
        LineCollisionComponent &lineCompA = getLineComponent(entityNumA);
        switch(tagCompB->m_shape)
        {
        case CollisionShape_e::RECTANGLE_C:
        {
            RectangleCollisionComponent &rectCompB = getRectangleComponent(entityNumA);
            checkLineRectCollision(posOriginA, lineCompA.m_secondPoint,
                                   posOriginB, rectCompB.m_size);
        }
            break;
        case CollisionShape_e::CIRCLE:
        {
            CircleCollisionComponent &circleCompB = getCircleComponent(entityNumB);
            checkCircleLineCollision(posOriginB, circleCompB.m_ray,
                                     posOriginA, lineCompA.m_secondPoint);
        }
            break;
        case CollisionShape_e::LINE:
        {
            LineCollisionComponent &lineCompB = getLineComponent(entityNumB);
            checkLineLineCollision(posOriginA, lineCompA.m_secondPoint,
                                   posOriginB, lineCompB.m_secondPoint);
        }
            break;
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
LineCollisionComponent &CollisionSystem::getLineComponent(uint32_t entityNum)
{
    LineCollisionComponent *collComp = stairwayToComponentManager().
            searchComponentByType<LineCollisionComponent>(entityNum,
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

//===================================================================
void CollisionSystem::execSystem()
{
    for(uint32_t i = 0; i < mVectNumEntity.size(); ++i)
    {
        CollisionComponent *tagCompA = stairwayToComponentManager().
                searchComponentByType<CollisionComponent>(mVectNumEntity[i],
                                                         Components_e::TAG_COMPONENT);
        assert(tagCompA);
        for(uint32_t j = i + 1; j < mVectNumEntity.size(); ++j)
        {
            CollisionComponent *tagCompB = stairwayToComponentManager().
                    searchComponentByType<CollisionComponent>(mVectNumEntity[j],
                                                        Components_e::TAG_COMPONENT);
            assert(tagCompB);
            if(checkTag(tagCompA->m_tag, tagCompB->m_tag))
            {
                treatCollision(mVectNumEntity[i], mVectNumEntity[j],
                               tagCompA, tagCompB);
            }
        }
    }
}
