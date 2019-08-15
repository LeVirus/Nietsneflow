#include "CollisionSystem.hpp"
#include "constants.hpp"
#include <ECS/Components/MapCoordComponent.hpp>
#include <ECS/Components/TagComponent.hpp>
#include <ECS/Components/CircleCollisionComponent.hpp>
#include <ECS/Components/RectangleCollisionComponent.hpp>
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
                                     TagComponent *tagCompA, TagComponent *tagCompB)
{
    MapCoordComponent *mapCompA, *mapCompB;
    getMapComponent(entityNumA, entityNumB, mapCompA, mapCompB);
    if(isCircleComp(tagCompA->m_tag))
    {
        CircleCollisionComponent *collCompA = stairwayToComponentManager().
                searchComponentByType<CircleCollisionComponent>(entityNumA,
                                      Components_e::CIRCLE_COLLISION_COMPONENT);
        assert(collCompA);
        if(isCircleComp(tagCompB->m_tag))
        {
            CircleCollisionComponent *collCompB = stairwayToComponentManager().
                    searchComponentByType<CircleCollisionComponent>(entityNumB,
                                          Components_e::CIRCLE_COLLISION_COMPONENT);
            assert(collCompB);
            checkCollision(collCompA, collCompB, mapCompA, mapCompB);
        }
        else
        {
            RectangleCollisionComponent *collCompB = stairwayToComponentManager().
                    searchComponentByType<RectangleCollisionComponent>(entityNumB,
                                          Components_e::RECTANGLE_COLLISION_COMPONENT);
            assert(collCompB);
            checkCollision(collCompA, collCompB, mapCompA, mapCompB);
        }

    }
    else
    {
        RectangleCollisionComponent *collCompA = stairwayToComponentManager().
                searchComponentByType<RectangleCollisionComponent>(entityNumA,
                                      Components_e::RECTANGLE_COLLISION_COMPONENT);
        assert(collCompA);
        if(isCircleComp(tagCompA->m_tag))
        {
            CircleCollisionComponent *collCompB = stairwayToComponentManager().
                    searchComponentByType<CircleCollisionComponent>(entityNumB,
                                          Components_e::CIRCLE_COLLISION_COMPONENT);
            assert(collCompB);
            checkCollision(collCompA, collCompB, mapCompA, mapCompB);
        }
        else
        {
            checkCollision(collCompA, mapCompA, mapCompB);
        }
    }
}

//===================================================================
bool CollisionSystem::isCircleComp(const CollisionTag_e collTag)
{
    return collTag == CollisionTag_e::PLAYER ||
            collTag == CollisionTag_e::ENEMY;
}

//===================================================================
void CollisionSystem::getMapComponent(uint32_t entityNumA,
                                      uint32_t entityNumB,
                                      MapCoordComponent *mapCompA,
                                      MapCoordComponent *mapCompB)
{
    mapCompA = stairwayToComponentManager().
            searchComponentByType<MapCoordComponent>(entityNumA,
                                                     Components_e::MAP_COORD_COMPONENT);
    assert(mapCompA);
    mapCompB = stairwayToComponentManager().
            searchComponentByType<MapCoordComponent>(entityNumB,
                                                     Components_e::MAP_COORD_COMPONENT);
    assert(mapCompB);
}

//===================================================================
void CollisionSystem::execSystem()
{
    for(uint32_t i = 0; i < mVectNumEntity.size(); ++i)
    {
        TagComponent *tagCompA = stairwayToComponentManager().
                searchComponentByType<TagComponent>(mVectNumEntity[i],
                                                         Components_e::TAG_COMPONENT);
        assert(tagCompA);
        for(uint32_t j = i + 1; j < mVectNumEntity.size(); ++j)
        {
            TagComponent *tagCompB = stairwayToComponentManager().
                    searchComponentByType<TagComponent>(mVectNumEntity[j],
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
bool checkCollision(CircleCollisionComponent *circleColl,
                    const RectangleCollisionComponent *rectColl,
                    MapCoordComponent *cicleMapComp,
                    const MapCoordComponent *rectMapComp)
{
    float circleCenterX = cicleMapComp->m_absoluteMapPositionPX.first,
    circleCenterY = cicleMapComp->m_absoluteMapPositionPX.second,
    rectPosX = rectMapComp->m_absoluteMapPositionPX.first,
    rectPosY = rectMapComp->m_absoluteMapPositionPX.second,
    rectSizeX = rectColl->m_size.first,
    rectSizeY = rectColl->m_size.second;
    if(circleCenterX < rectPosX - circleColl->m_ray ||
            circleCenterX > rectPosX + rectSizeX + circleColl->m_ray)
    {
        return false;
    }
    if(circleCenterY < rectPosY - circleColl->m_ray ||
            circleCenterY > rectPosY + rectSizeY + circleColl->m_ray)
    {
        return false;
    }
    return true;
}

//===================================================================
bool checkCollision(const RectangleCollisionComponent *rectColl,
                    CircleCollisionComponent *circleColl,
                    const MapCoordComponent *rectMapComp,
                    MapCoordComponent *circleMapComp)
{
    return checkCollision(circleColl, rectColl, circleMapComp, rectMapComp);
}

//===================================================================
bool checkCollision(CircleCollisionComponent *circleCollA,
                    CircleCollisionComponent *circleCollB,
                    MapCoordComponent *mapCompA,
                    MapCoordComponent *mapCompB)
{
    float posCircleXA = mapCompA->m_absoluteMapPositionPX.first,
    posCircleXB = mapCompB->m_absoluteMapPositionPX.first,
    posCircleYA = mapCompA->m_absoluteMapPositionPX.second,
    posCircleYB = mapCompB->m_absoluteMapPositionPX.second,
    rayCircleA = circleCollA->m_ray,
    rayCircleB = circleCollB->m_ray;
    if((posCircleXA + rayCircleA) < (posCircleXB - rayCircleB) ||
       (posCircleXB + rayCircleB) < (posCircleXA - rayCircleA) ||
       (posCircleYA + rayCircleA) < (posCircleYB - rayCircleB) ||
       (posCircleYB + rayCircleB) < (posCircleYA - rayCircleA))
    {
        return false;
    }
    return true;
}

//===================================================================
bool checkCollision(const RectangleCollisionComponent *rectCollA,
                    const MapCoordComponent *mapCompA,
                    const MapCoordComponent *mapCompB)
{
    float rectPosXA = mapCompA->m_absoluteMapPositionPX.first,
    rectPosYA = mapCompA->m_absoluteMapPositionPX.second,
    rectSizeXA = rectCollA->m_size.first,
    rectSizeYA = rectCollA->m_size.second,
    rectPosXB = mapCompB->m_absoluteMapPositionPX.first,
    rectPosYB = mapCompB->m_absoluteMapPositionPX.second;
    if(rectPosXA + rectSizeXA < rectPosXB ||
       rectPosXB > rectPosXA + rectSizeXA ||
       rectPosYA + rectSizeYA < rectPosYB ||
       rectPosYB > rectPosYA + rectSizeYA)
    {
        return false;
    }
    return true;
}
