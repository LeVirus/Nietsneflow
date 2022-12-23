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
struct ShotConfComponent;
class MainEngine;

class CollisionSystem : public ecs::System
{
public:
    CollisionSystem();
    void execSystem()override;
    inline const std::vector<uint32_t> &getStaticEntitiesToDelete()const
    {
        return m_vectEntitiesToDelete;
    }
    inline const std::vector<uint32_t> &getBarrelEntitiesDestruct()const
    {
        return m_vectBarrelsEntitiesDestruct;
    }
    inline void clearVectBarrelsDestruct()
    {
        m_vectBarrelsEntitiesDestruct.clear();
    }
    inline void clearVectObjectToDelete()
    {
        m_vectEntitiesToDelete.clear();
    }
    inline void linkMainEngine(MainEngine *mainEngine)
    {
        m_refMainEngine = mainEngine;
    }
    void memPlayerDatas(uint32_t playerEntity);
    void writePlayerInfo(const std::string &info);
private:
    void checkCollisionFirstRect(CollisionArgs &args);
    void treatSegmentShots();
    void treatBarrelShots(uint32_t entityNum, uint32_t damage);
    void confImpactShots(uint32_t numBullet, CollisionTag_e targetTag);
    void rmEnemyCollisionMaskEntity(uint32_t numEntity);
    void setUsedComponents();
    void initArrayTag();
    bool checkTag(CollisionTag_e entityTagA, CollisionTag_e entityTagB);
    //return false if new collision iteration have to be done
    bool treatCollision(uint32_t entityNumA, uint32_t entityNumB, GeneralCollisionComponent *tagCompA,
                        GeneralCollisionComponent *tagCompB, bool shotExplosionEject = false);
    //Collisions detection
    void treatCollisionFirstRect(CollisionArgs &args);
    bool treatDoorCollisionFirstCircle(CollisionArgs &args, const CircleCollisionComponent &circleCompA,
                                       const RectangleCollisionComponent &rectCompB);
    bool treatCollisionFirstCircle(CollisionArgs &args, bool shotExplosionEject = false);
    bool treatCollisionPlayer(CollisionArgs &args, CircleCollisionComponent &circleCompA, RectangleCollisionComponent &rectCompB);
    void treatActionPlayerRect(CollisionArgs &args);
    void treatExplosionColl(CollisionArgs &args);
    //Check if explosion or visible shot have element between attack and target
    bool circleDamageObstructed(const CollisionArgs &args)const;
    void setDamageCircle(uint32_t shotEntity, bool active, uint32_t baseEntity = 0);
    void treatActionPlayerCircle(CollisionArgs &args);
    void treatPlayerPickObject(CollisionArgs &args);
    void treatPlayerTeleport(CollisionArgs &args);
    void treatCollisionFirstSegment(CollisionArgs &args);
    void treatCrushing(uint32_t entityNum);
    //Collisions treatment
    void collisionCircleRectEject(CollisionArgs &args,
                                  float circleRay, const RectangleCollisionComponent &rectCollB, bool visibleShotFirstEject = false);
    float getVerticalCircleRectEject(const EjectYArgs &args, bool &limitEject, bool visibleShot);
    float getHorizontalCircleRectEject(const EjectXArgs &args, bool &limitEject, bool visibleShot);
    void collisionCircleCircleEject(CollisionArgs &args,
                                    const CircleCollisionComponent &circleCollA,
                                    const CircleCollisionComponent &circleCollB);
//    void treatCollisionCircleSegment(CollisionArgs &args,
//                                     const CircleCollisionComponent &circleCollA,
//                                     const SegmentCollisionComponent &segmCollB);
    void collisionEject(MapCoordComponent &mapComp, float diffX, float diffY,
                        bool limitEjectY = false, bool limitEjectX = false, bool crushCase = false);
    //Components accessors
    CircleCollisionComponent &getCircleComponent(uint32_t entityNum);
    RectangleCollisionComponent &getRectangleComponent(uint32_t entityNum);
    SegmentCollisionComponent &getSegmentComponent(uint32_t entityNum);
    MapCoordComponent &getMapComponent(uint32_t entityNum);
    void checkCollisionFirstSegment(uint32_t numEntityA, uint32_t numEntityB,
                                    GeneralCollisionComponent *tagCompB,
                                    MapCoordComponent &mapCompB);
    void calcBulletSegment(SegmentCollisionComponent &segmentCompA);
    void treatEnemyTakeDamage(uint32_t enemyEntityNum, uint32_t damage = 1);
    void confDropedObject(uint32_t objectEntity, uint32_t enemyEntity);
    void activeSound(uint32_t entityNum);
    bool checkEnemyRemoveCollisionMask(uint32_t entityNum);
    void treatGeneralCrushing(uint32_t entityNum);
    void secondEntitiesLoop(uint32_t entityA, uint32_t currentIteration, GeneralCollisionComponent *tagCompA, bool shotExplosionEject = false);
private:
    std::multimap<CollisionTag_e, CollisionTag_e> m_tagArray;
    std::pair<std::optional<uint32_t>, float> m_memDistCurrentBulletColl;
    //first bullet second target
    std::vector<PairUI_t> m_vectMemShots;
    std::vector<uint32_t> m_vectEntitiesToDelete, m_vectBarrelsEntitiesDestruct;
    PlayerConfComponent *m_playerComp = nullptr;
    bool m_memPlayerTeleport;
    //0 movement eject, 1 angle behaviour, 2 Direction,
    //3 if moveable wall current direction
    std::vector<std::tuple<PairFloat_t, bool, Direction_e, std::optional<Direction_e>>> m_memCrush;
    MainEngine *m_refMainEngine;
};

bool opposingDirection(Direction_e dirA, Direction_e dirB);
bool pickUpWeapon(uint32_t numWeapon, WeaponComponent *weaponComp,
                  uint32_t objectContaining);
bool pickUpAmmo(uint32_t numWeapon, WeaponComponent *weaponComp,
                uint32_t objectContaining);
Direction_e getDirection(float diffX, float diffY);

struct CollisionArgs
{
    uint32_t entityNumA, entityNumB;
    GeneralCollisionComponent *tagCompA, *tagCompB;
    MapCoordComponent &mapCompA, &mapCompB;
};

struct EjectXArgs
{
    float circlePosX, circlePosY, elementPosY, elementPosX,
    elementSecondPosX, ray, radiantAngle;
    bool angleMode;
};

struct EjectYArgs
{
    float circlePosX, circlePosY, elementPosX, elementPosY,
    elementSecondPosY, ray, radiantAngle;
    bool angleMode;
};
