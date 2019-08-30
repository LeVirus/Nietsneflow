#ifndef COLLISIONSYSTEM_H
#define COLLISIONSYSTEM_H

#include <includesLib/BaseECS/system.hpp>
#include "constants.hpp"
#include <map>

struct CircleCollisionComponent;
struct RectangleCollisionComponent;
struct SegmentCollisionComponent;
struct MapCoordComponent;
struct GeneralCollisionComponent;
struct CollisionArgs;
struct EjectYArgs;
struct EjectXArgs;

class CollisionSystem : public ecs::System
{
private:
    std::multimap<CollisionTag_e, CollisionTag_e> m_tagArray;
private:
    void setUsedComponents();
    void initArrayTag();
    bool checkTag(CollisionTag_e entityTagA, CollisionTag_e entityTagB);
    void checkCollision(uint32_t entityNumA, uint32_t entityNumB,
                        GeneralCollisionComponent *tagCompA,
                        GeneralCollisionComponent *tagCompB);
    //Collisions detection
    void checkCollisionFirstRect(CollisionArgs &args);
    void checkCollisionFirstCircle(CollisionArgs &args);
    void checkCollisionFirstSegment(CollisionArgs &args);

    //Collisions treatment
    void treatCollisionCircleRect(CollisionArgs &args,
                                const CircleCollisionComponent &circleCollA,
                                const RectangleCollisionComponent &rectCollB);
    float getVerticalCircleRectEject(const EjectYArgs& args);
    float getHorizontalCircleRectEject(const EjectXArgs &args);
    void treatCollisionCircleCircle(CollisionArgs &args,
                                const CircleCollisionComponent &circleCollA,
                                const CircleCollisionComponent &circleCollB);
    void treatCollisionCircleSegment(CollisionArgs &args,
                                const CircleCollisionComponent &circleCollA,
                                const SegmentCollisionComponent &segmCollB);
    void collisionEject(MapCoordComponent &mapComp,
                                     float diffX, float diffY);
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
    const GeneralCollisionComponent *tagCompA, *tagCompB;
    MapCoordComponent &mapCompA, &mapCompB;
};

struct EjectXArgs
{
    float circlePosX, circlePosY, elementPosY, elementPosX,
    elementSecondPosX, ray, radDegree;
    bool angleMode;
};

struct EjectYArgs
{
    float circlePosX, circlePosY, elementPosX, elementPosY,
    elementSecondPosY, ray, radDegree;
    bool angleMode;
};

#endif // COLLISIONSYSTEM_H
