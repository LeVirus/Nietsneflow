#ifndef COLLISIONSYSTEM_H
#define COLLISIONSYSTEM_H

#include <includesLib/BaseECS/system.hpp>
#include "constants.hpp"
#include <map>
struct CircleCollisionComponent;
struct RectangleCollisionComponent;
struct MapCoordComponent;
struct TagComponent;

class CollisionSystem : public ecs::System
{
private:
    std::multimap<CollisionTag_e, CollisionTag_e> m_tagArray;
private:
    void setUsedComponents();
    void initArrayTag();
    bool checkTag(CollisionTag_e entityTagA, CollisionTag_e entityTagB);
    void treatCollision(uint32_t entityNumA, uint32_t entityNumB,
                        TagComponent *tagCompA, TagComponent *tagCompB);
    bool isCircleComp(const CollisionTag_e collTag);
    void getMapComponent(uint32_t entityNumA, uint32_t entityNumB,
                         MapCoordComponent *mapCompA,
                         MapCoordComponent *mapCompB);
public:
    CollisionSystem();
    void execSystem()override;
};

bool checkCollision(CircleCollisionComponent *circleColl,
                    const RectangleCollisionComponent *rectColl,
                    MapCoordComponent *cicleMapComp,
                    const MapCoordComponent *rectMapComp);
bool checkCollision(const RectangleCollisionComponent *rectColl,
                    CircleCollisionComponent *circleColl,
                    const MapCoordComponent *rectMapComp,
                    MapCoordComponent *circleMapComp);
bool checkCollision(CircleCollisionComponent *circleCollA,
                    CircleCollisionComponent *circleCollB,
                    MapCoordComponent *mapCompA,
                    MapCoordComponent *mapCompB);
//rect rect coll
bool checkCollision(const RectangleCollisionComponent *rectCollA,
                    const MapCoordComponent *mapCompA,
                    const MapCoordComponent *mapCompB);

#endif // COLLISIONSYSTEM_H
