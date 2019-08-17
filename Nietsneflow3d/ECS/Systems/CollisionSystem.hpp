#ifndef COLLISIONSYSTEM_H
#define COLLISIONSYSTEM_H

#include <includesLib/BaseECS/system.hpp>
#include "constants.hpp"
#include <map>
struct CircleCollisionComponent;
struct RectangleCollisionComponent;
struct LineCollisionComponent;
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
    void treatCollision(uint32_t entityNumA, uint32_t entityNumB,
                        CollisionComponent *tagCompA,
                        CollisionComponent *tagCompB);
    void treatCollisionFirstRect(const CollisionArgs &args);
    void treatCollisionFirstCircle(const CollisionArgs &args);
    void treatCollisionFirstLine(const CollisionArgs &args);
    //Components accessors
    CircleCollisionComponent &getCircleComponent(uint32_t entityNum);
    RectangleCollisionComponent &getRectangleComponent(uint32_t entityNum);
    LineCollisionComponent &getLineComponent(uint32_t entityNum);
    MapCoordComponent &getMapComponent(uint32_t entityNum);
public:
    CollisionSystem();
    void execSystem()override;
};

struct CollisionArgs
{
    uint32_t entityNumA, entityNumB;
    CollisionShape_e tag;
    const pairFloat_t &originA, &originB;
};

#endif // COLLISIONSYSTEM_H
