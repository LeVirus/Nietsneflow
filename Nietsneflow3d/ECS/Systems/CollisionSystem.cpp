#include "CollisionSystem.hpp"
#include "constants.hpp"
#include <ECS/Components/MapCoordComponent.hpp>
#include <ECS/Components/CollisionComponent.hpp>
#include <ECS/Components/CircleCollisionComponent.hpp>
#include <ECS/Components/RectangleCollisionComponent.hpp>
#include <ECS/Components/LineCollisionComponent.hpp>
#include <cassert>
#include <cmath>

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
    if(tagCompA->m_shape == CollisionShape_e::RECTANGLE_C)
    {
        RectangleCollisionComponent &rectCompA = getRectangleComponent(entityNumA);
        switch(tagCompB->m_shape)
        {
        case CollisionShape_e::RECTANGLE_C:
            checkCollision(rectCompA, getRectangleComponent(entityNumB),
                           mapCompA, mapCompB);
            break;
        case CollisionShape_e::CIRCLE:
            checkCollision(getCircleComponent(entityNumB), rectCompA,
                           mapCompB, mapCompA);
            break;
        case CollisionShape_e::LINE:
            checkCollision(getLineComponent(entityNumB), rectCompA,
                           mapCompA, mapCompB);
            break;
        }
    }
    else if(tagCompA->m_shape == CollisionShape_e::CIRCLE)
    {
        CircleCollisionComponent &circleCompA = getCircleComponent(entityNumA);
        switch(tagCompB->m_shape)
        {
        case CollisionShape_e::RECTANGLE_C:
            checkCollision(circleCompA, getRectangleComponent(entityNumB),
                           mapCompA, mapCompB);
            break;
        case CollisionShape_e::CIRCLE:
            checkCollision(circleCompA, getCircleComponent(entityNumB),
                           mapCompA, mapCompB);
            break;
        case CollisionShape_e::LINE:
            checkCollision(circleCompA, getLineComponent(entityNumB),
                           mapCompA, mapCompB);
            break;
        }
    }
    else if(tagCompA->m_shape == CollisionShape_e::LINE)
    {
        LineCollisionComponent &lineCompA = getLineComponent(entityNumA);
        switch(tagCompB->m_shape)
        {
        case CollisionShape_e::RECTANGLE_C:
            checkCollision(lineCompA, getRectangleComponent(entityNumB),
                           mapCompA, mapCompB);
            break;
        case CollisionShape_e::CIRCLE:
            checkCollision(getCircleComponent(entityNumB), lineCompA,
                           mapCompA, mapCompB);
            break;
        case CollisionShape_e::LINE:
            checkCollision(lineCompA, getLineComponent(entityNumB),
                           mapCompA, mapCompB);
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

//===================================================================
bool checkCollision(CircleCollisionComponent &circleColl,
                    const RectangleCollisionComponent &rectColl,
                    MapCoordComponent &cicleMapComp,
                    const MapCoordComponent &rectMapComp)
{
    float circleCenterX = cicleMapComp.m_absoluteMapPositionPX.first,
    circleCenterY = cicleMapComp.m_absoluteMapPositionPX.second,
    rectPosX = rectMapComp.m_absoluteMapPositionPX.first,
    rectPosY = rectMapComp.m_absoluteMapPositionPX.second,
    rectSizeX = rectColl.m_size.first,
    rectSizeY = rectColl.m_size.second;
    if(circleCenterX + circleColl.m_ray < rectPosX ||
       circleCenterX - circleColl.m_ray > rectPosX + rectSizeX ||
       circleCenterY + circleColl.m_ray < rectPosY ||
       circleCenterY - circleColl.m_ray> rectPosY + rectSizeY)
    {
        return false;
    }
    pairFloat_t rectCenterPoint = {rectPosX + rectSizeX / 2,
                                   rectPosY + rectSizeY / 2};
    float rectDiagonal = getDistance(rectMapComp.m_absoluteMapPositionPX,
                                     rectCenterPoint);
    //Add test
    if(getDistance(rectCenterPoint, cicleMapComp.m_absoluteMapPositionPX) >
            (circleColl.m_ray + rectDiagonal))
    {
        return false;
    }
    return true;
}

//===================================================================
bool checkCollision(CircleCollisionComponent &circleCollA,
                    CircleCollisionComponent &circleCollB,
                    MapCoordComponent &mapCompA,
                    MapCoordComponent &mapCompB)
{
    float posCircleXA = mapCompA.m_absoluteMapPositionPX.first,
    posCircleXB = mapCompB.m_absoluteMapPositionPX.first,
    posCircleYA = mapCompA.m_absoluteMapPositionPX.second,
    posCircleYB = mapCompB.m_absoluteMapPositionPX.second,
    rayCircleA = circleCollA.m_ray,
    rayCircleB = circleCollB.m_ray;
    if((posCircleXA + rayCircleA) < (posCircleXB - rayCircleB) ||
       (posCircleXB + rayCircleB) < (posCircleXA - rayCircleA) ||
       (posCircleYA + rayCircleA) < (posCircleYB - rayCircleB) ||
       (posCircleYB + rayCircleB) < (posCircleYA - rayCircleA))
    {
        return false;
    }
    if(getDistance(mapCompA.m_absoluteMapPositionPX,
                   mapCompB.m_absoluteMapPositionPX) > rayCircleA + rayCircleB)
    {
        return false;
    }
    return true;
}

//===================================================================
bool checkCollision(const RectangleCollisionComponent &rectCollA,
                    const RectangleCollisionComponent &rectCollB,
                    const MapCoordComponent &mapCompA,
                    const MapCoordComponent &mapCompB)
{
    float rectPosXA = mapCompA.m_absoluteMapPositionPX.first,
    rectPosYA = mapCompA.m_absoluteMapPositionPX.second,
    rectSizeXA = rectCollA.m_size.first,
    rectSizeYA = rectCollA.m_size.second,
    rectPosXB = mapCompB.m_absoluteMapPositionPX.first,
    rectPosYB = mapCompB.m_absoluteMapPositionPX.second,
    rectSizeXB = rectCollB.m_size.first,
    rectSizeYB = rectCollB.m_size.second;
    if(rectPosXA + rectSizeXA < rectPosXB ||
       rectPosXB + rectSizeXB < rectPosXA ||
       rectPosYA + rectSizeYA < rectPosYB ||
       rectPosYB + rectSizeYB < rectPosYA)
    {
        return false;
    }
    return true;
}

//===================================================================
bool checkCollision(LineCollisionComponent &lineColl,
                    const RectangleCollisionComponent &rectColl,
                    MapCoordComponent &cicleMapComp,
                    const MapCoordComponent &rectMapComp)
{

}

//===================================================================
bool checkCollision(CircleCollisionComponent &circleColl,
                    const LineCollisionComponent &lineColl,
                    MapCoordComponent &cicleMapComp,
                    const MapCoordComponent &rectMapComp)
{

}

//===================================================================
bool checkCollision(LineCollisionComponent &lineCollA,
                    const LineCollisionComponent &lineCollB,
                    MapCoordComponent &cicleMapComp,
                    const MapCoordComponent &rectMapComp)
{

}

//===================================================================
float getDistance(const pairFloat_t &pairA, const pairFloat_t &pairB)
{
    float distanceX = std::abs(pairA.first - pairB.first),
    distanceY = std::abs(pairA.second - pairB.second);
    return std::sqrt(distanceX * distanceX + distanceY * distanceY);
}
