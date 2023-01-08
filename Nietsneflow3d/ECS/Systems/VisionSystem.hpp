#pragma once

#include <BaseECS/system.hpp>
#include <constants.hpp>

class ECSManager;
struct VisionComponent;
struct MapCoordComponent;
struct MoveableComponent;
struct EnemyConfComponent;
struct MemSpriteDataComponent;
struct SpriteTextureComponent;
struct TimerComponent;
struct GeneralCollisionComponent;
struct TimerComponent;
struct TimerComponent;
class MainEngine;

using mapEnemySprite_t = std::map<EnemySpriteType_e, PairUI_t>;

class VisionSystem : public ecs::System
{
public:
    VisionSystem(const ECSManager* memECSManager);
    void memECSManager(const ECSManager *memECSMan);
    void execSystem()override;
    inline void clearMemMultiSpritesWall()
    {
        m_memMultiSpritesWallEntities.clear();
    }
    inline void clearVectObjectToDelete()
    {
        m_vectBarrelsEntitiesToDelete.clear();
    }
    inline void memTeleportAnimEntity(uint32_t teleportAnimEntity)
    {
        m_memTeleportAnimEntity = teleportAnimEntity;
    }
    inline const std::vector<uint32_t> &getBarrelEntitiesToDelete()const
    {
        return m_vectBarrelsEntitiesToDelete;
    }
    void memRefMainEngine(MainEngine *mainEngine);
private:
    EnemySpriteType_e getOrientationFromAngle(uint32_t observerEntity, uint32_t targetEntity,
                                              float targetDegreeAngle);
    void setUsedComponents();
    void treatVisible(VisionComponent *visionComp, MoveableComponent *moveCompA,
                      uint32_t numEntity);
    void updateSprites(uint32_t observerEntity, const std::vector<uint32_t> &vectEntities);
    void updateWallSprites();
    void updateTeleportAnimationSprites();
    void memMultiSpritesWallEntities();
    void updateVisibleShotSprite(uint32_t shotEntity, MemSpriteDataComponent *memSpriteComp,
                                 SpriteTextureComponent *spriteComp,
                                 TimerComponent *timerComp, GeneralCollisionComponent *genComp);
    void updateBarrelSprite(uint32_t barrelEntity, MemSpriteDataComponent *memSpriteComp, SpriteTextureComponent *spriteComp,
                            TimerComponent *timerComp, GeneralCollisionComponent *genComp);
    void updateEnemySprites(uint32_t enemyEntity, uint32_t observerEntity,
                            MemSpriteDataComponent *memSpriteComp,
                            SpriteTextureComponent *spriteComp,
                            TimerComponent *timerComp, EnemyConfComponent *enemyConfComp);
    void updateEnemyNormalSprite(EnemyConfComponent *enemyConfComp, TimerComponent *timerComp,
                                 uint32_t enemyEntity, uint32_t observerEntity);
    void updateImpactSprites(uint32_t entityImpact, MemSpriteDataComponent *memSpriteComp,
                             SpriteTextureComponent *spriteComp,
                             TimerComponent *timerComp, GeneralCollisionComponent *genComp);
private:
    const ECSManager* m_memECSManager;
    std::vector<uint32_t> m_memMultiSpritesWallEntities, m_vectBarrelsEntitiesToDelete;
    uint32_t m_defaultInterval = 0.8 / FPS_VALUE, m_memTeleportAnimEntity;
    //first change sprite interval, second interval total time
    PairUI_t m_teleportIntervalTime = {0.1 / FPS_VALUE, 0.4 / FPS_VALUE};
    MainEngine *m_refMainEngine;
};

mapEnemySprite_t::const_reverse_iterator findMapLastElement(const mapEnemySprite_t &map,
                                                            EnemySpriteType_e key);
void updateTriangleVisionFromPosition(VisionComponent *visionComp, const MapCoordComponent *mapComp,
                                      const MoveableComponent *movComp);
void updateEnemyAttackSprite(EnemyConfComponent *enemyConfComp, TimerComponent *timerComp);
