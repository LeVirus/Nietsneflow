#ifndef COLLISIONSYSTEM_H
#define COLLISIONSYSTEM_H

#include <includesLib/BaseECS/system.hpp>
#include "constants.hpp"
#include <map>

struct CircleCollisionComponent;
struct RectangleCollisionComponent;
struct SegmentCollisionComponent;
struct MapCoordComponent;
struct CollisionComponent;
struct CollisionArgs;

class CollisionSystem : public ecs::System
{
private:
    std::multimap<CollisionTag_e, CollisionTag_e> m_tagArray;
private:
    void setUsedComponents();
    void initArrayTag();
    bool checkTag(CollisionTag_e entityTagA, CollisionTag_e entityTagB);
    void checkCollision(uint32_t entityNumA, uint32_t entityNumB,
                        CollisionComponent *tagCompA,
                        CollisionComponent *tagCompB);
    //Collisions detection
    void checkCollisionFirstRect(CollisionArgs &args);
    void checkCollisionFirstCircle(CollisionArgs &args);
    void checkCollisionFirstLine(CollisionArgs &args);

    //Collisions treatment
    void treatCollisionCircleRect(CollisionArgs &args,
                                const CircleCollisionComponent &circleCollA,
                                const RectangleCollisionComponent &rectCollB);
    void treatCollisionCircleCircle(CollisionArgs &args,
                                const CircleCollisionComponent &circleCollA,
                                const CircleCollisionComponent &circleCollB);
    void treatCollisionCircleSegment(CollisionArgs &args,
                                const CircleCollisionComponent &circleCollA,
                                const SegmentCollisionComponent &segmCollB);
    //Components accessors
    CircleCollisionComponent &getCircleComponent(uint32_t entityNum);
    RectangleCollisionComponent &getRectangleComponent(uint32_t entityNum);
    SegmentCollisionComponent &getSegmentComponent(uint32_t entityNum);
    MapCoordComponent &getMapComponent(uint32_t entityNum);
public:
    CollisionSystem();
    void execSystem()override;
};

struct CollisionArgs
{
    uint32_t entityNumA, entityNumB;
    const CollisionComponent *tagCompA, *tagCompB;
    MapCoordComponent &mapCompA, &mapCompB;
};

#endif // COLLISIONSYSTEM_H
