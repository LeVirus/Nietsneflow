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
struct PlayerConfComponent;
struct WeaponComponent;
struct MoveableWallConfComponent;

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
    void memPlayerDatas(uint32_t playerEntity);
    void writePlayerInfo(const std::string &info);
private:
    void treatSegmentShots();
    void confImpactShots(uint32_t numBullet, CollisionTag_e targetTag);
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
    void treatActionPlayerRect(CollisionArgs &args);
    void treatActionPlayerCircle(CollisionArgs &args);
    void treatPlayerPickObject(CollisionArgs &args);
    void treatCollisionFirstSegment(CollisionArgs &args);

    //Collisions treatment
    void collisionCircleRectEject(CollisionArgs &args,
                                  const CircleCollisionComponent &circleCollA,
                                  const RectangleCollisionComponent &rectCollB);
    float getVerticalCircleRectEject(const EjectYArgs &args, bool &limitEject);
    float getHorizontalCircleRectEject(const EjectXArgs &args, bool &limitEject);
    void collisionCircleCircleEject(CollisionArgs &args,
                                    const CircleCollisionComponent &circleCollA,
                                    const CircleCollisionComponent &circleCollB);
//    void treatCollisionCircleSegment(CollisionArgs &args,
//                                     const CircleCollisionComponent &circleCollA,
//                                     const SegmentCollisionComponent &segmCollB);
    void collisionEject(MapCoordComponent &mapComp, float diffX, float diffY,
                        bool limitEjectY = false, bool limitEjectX = false);
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
    void confDropedObject(uint32_t objectEntity, uint32_t enemyEntity);
private:
    std::multimap<CollisionTag_e, CollisionTag_e> m_tagArray;
    std::pair<std::optional<uint32_t>, float> m_memDistCurrentBulletColl;
    //first bullet second target
    std::vector<pairUI_t> m_vectMemShots;
    std::vector<uint32_t> m_vectEntitiesToDelete;
    PlayerConfComponent *m_playerComp = nullptr;
};

bool pickUpWeapon(uint32_t numWeapon, WeaponComponent *weaponComp,
                  uint32_t objectContaining);
bool pickUpAmmo(uint32_t numWeapon, WeaponComponent *weaponComp,
                uint32_t objectContaining);

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
