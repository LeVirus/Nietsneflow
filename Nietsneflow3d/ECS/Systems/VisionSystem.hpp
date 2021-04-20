#pragma once

#include <BaseECS/system.hpp>
#include <constants.hpp>

class ECSManager;
struct VisionComponent;
struct MapCoordComponent;
struct MoveableComponent;

class VisionSystem : public ecs::System
{
public:
    VisionSystem(const ECSManager* memECSManager);
    void memECSManager(const ECSManager *memECSMan);
    void execSystem()override;
private:
    void setUsedComponents();
    void treatVisible(VisionComponent *visionComp, uint32_t checkVisibleId, CollisionShape_e shapeElement);
    void updateSprites(MoveableComponent *moveComp, const std::vector<uint32_t> &vectEntities);
private:
    const ECSManager* m_memECSManager;
};

void updateTriangleVisionFromPosition(VisionComponent *visionComp, const MapCoordComponent *mapComp,
                                      const MoveableComponent *movComp);
