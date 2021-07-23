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

class VisionSystem : public ecs::System
{
public:
    VisionSystem(const ECSManager* memECSManager);
    void memECSManager(const ECSManager *memECSMan);
    void execSystem()override;
private:
    EnemySpriteType_e getOrientationFromAngle(uint32_t observerEntity, uint32_t targetEntity,
                                              float targetDegreeAngle);
    void setUsedComponents();
    void treatVisible(VisionComponent *visionComp, MoveableComponent *moveCompA,
                      uint32_t numEntity);
    void updateSprites(uint32_t observerEntity, const std::vector<uint32_t> &vectEntities);
    void updateWallSprites(const std::vector<uint32_t> &vectEntities);
    void updateVisibleShotSprite(uint32_t shotEntity, MemSpriteDataComponent *memSpriteComp,
                                 SpriteTextureComponent *spriteComp,
                                 TimerComponent *timerComp, GeneralCollisionComponent *genComp);
    void updateEnemySprites(uint32_t enemyEntity, uint32_t observerEntity,
                            MemSpriteDataComponent *memSpriteComp,
                            SpriteTextureComponent *spriteComp,
                            TimerComponent *timerComp, EnemyConfComponent *enemyConfComp);
    void updateImpactSprites(uint32_t entityImpact, MemSpriteDataComponent *memSpriteComp,
                             SpriteTextureComponent *spriteComp,
                             TimerComponent *timerComp, GeneralCollisionComponent *genComp);
private:
    const ECSManager* m_memECSManager;
};

void updateTriangleVisionFromPosition(VisionComponent *visionComp, const MapCoordComponent *mapComp,
                                      const MoveableComponent *movComp);
