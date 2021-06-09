#pragma once

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
public:
    CollisionSystem();
    void execSystem()override;
    inline const std::vector<uint32_t> &getObjectEntityToDelete()const
    {
        return m_vectEntitiesToDelete;
    }
    inline void clearVectObjectToDelete()
    {
        m_vectEntitiesToDelete.clear();
    }
private:
    void treatSegmentShots();
    void confImpactShots(uint32_t iterationNum);
    void rmCollisionMaskEntity(uint32_t numEntity);
    void setUsedComponents();
    void initArrayTag();
    bool checkTag(CollisionTag_e entityTagA, CollisionTag_e entityTagB);
    void treatCollision(uint32_t entityNumA, uint32_t entityNumB,
                        GeneralCollisionComponent *tagCompA,
                        GeneralCollisionComponent *tagCompB);
    //Collisions detection
    void treatCollisionFirstRect(CollisionArgs &args);
    void treatCollisionFirstCircle(CollisionArgs &args);
    void treatPlayerPickObject(CollisionArgs &args);
    void treatCollisionFirstSegment(CollisionArgs &args);

    //Collisions treatment
    void collisionCircleRectEject(CollisionArgs &args,
                                  const CircleCollisionComponent &circleCollA,
                                  const RectangleCollisionComponent &rectCollB);
    float getVerticalCircleRectEject(const EjectYArgs& args);
    float getHorizontalCircleRectEject(const EjectXArgs &args);
    void collisionCircleCircleEject(CollisionArgs &args,
                                    const CircleCollisionComponent &circleCollA,
                                    const CircleCollisionComponent &circleCollB);
//    void treatCollisionCircleSegment(CollisionArgs &args,
//                                     const CircleCollisionComponent &circleCollA,
//                                     const SegmentCollisionComponent &segmCollB);
    void collisionEject(MapCoordComponent &mapComp,
                                     float diffX, float diffY);
    //Components accessors
    CircleCollisionComponent &getCircleComponent(uint32_t entityNum);
    RectangleCollisionComponent &getRectangleComponent(uint32_t entityNum);
    SegmentCollisionComponent &getSegmentComponent(uint32_t entityNum);
    MapCoordComponent &getMapComponent(uint32_t entityNum);
    void checkCollisionFirstSegment(uint32_t numEntityA, uint32_t numEntityB,
                                    GeneralCollisionComponent *tagCompB,
                                    MapCoordComponent &mapCompB);
    void calcBulletSegment(SegmentCollisionComponent &segmentCompA);
    void treatEnemyShooted(uint32_t enemyEntityNum, uint32_t damage = 1);
private:
    std::multimap<CollisionTag_e, CollisionTag_e> m_tagArray;
    std::pair<uint32_t, float> m_memDistCurrentBulletColl;
    //first bullet second target
    std::vector<pairUI_t> m_vectMemShots;
    std::vector<uint32_t> m_vectEntitiesToDelete;
};

struct CollisionArgs
{
    uint32_t entityNumA, entityNumB;
    GeneralCollisionComponent *tagCompA, *tagCompB;
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
