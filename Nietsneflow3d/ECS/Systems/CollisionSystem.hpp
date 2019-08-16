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

class CollisionSystem : public ecs::System
{
private:
    std::multimap<CollisionTag_e, CollisionTag_e> m_tagArray;
private:
    void setUsedComponents();
    void initArrayTag();
    bool checkTag(CollisionTag_e entityTagA, CollisionTag_e entityTagB);
    void treatCollision(uint32_t entityNumA, uint32_t entityNumB,
                        CollisionComponent *tagCompA, CollisionComponent *tagCompB);
    //Components accessors
    CircleCollisionComponent &getCircleComponent(uint32_t entityNum);
    RectangleCollisionComponent &getRectangleComponent(uint32_t entityNum);
    LineCollisionComponent &getLineComponent(uint32_t entityNum);
    MapCoordComponent &getMapComponent(uint32_t entityNum);
public:
    CollisionSystem();
    void execSystem()override;
};

//circle rect coll
bool checkCollision(CircleCollisionComponent &circleColl,
                    const RectangleCollisionComponent &rectColl,
                    MapCoordComponent &cicleMapComp,
                    const MapCoordComponent &rectMapComp);
//circle circle coll
bool checkCollision(CircleCollisionComponent &circleCollA,
                    CircleCollisionComponent &circleCollB,
                    MapCoordComponent &mapCompA,
                    MapCoordComponent &mapCompB);
//rect rect coll
bool checkCollision(const RectangleCollisionComponent &rectCollA, const RectangleCollisionComponent &rectCollB,
                    const MapCoordComponent &mapCompA,
                    const MapCoordComponent &mapCompB);
//line rect coll
bool checkCollision(LineCollisionComponent &lineColl,
                    const RectangleCollisionComponent &rectColl,
                    MapCoordComponent &cicleMapComp,
                    const MapCoordComponent &rectMapComp);
//circle line coll
bool checkCollision(CircleCollisionComponent &circleColl,
                    const LineCollisionComponent &lineColl,
                    MapCoordComponent &cicleMapComp,
                    const MapCoordComponent &rectMapComp);
//circle line line
bool checkCollision(LineCollisionComponent &lineCollA,
                    const LineCollisionComponent &lineCollB,
                    MapCoordComponent &cicleMapComp,
                    const MapCoordComponent &rectMapComp);
float getDistance(const pairFloat_t &pairA, const pairFloat_t &pairB);

#endif // COLLISIONSYSTEM_H
