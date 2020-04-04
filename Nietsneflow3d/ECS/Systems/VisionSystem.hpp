#pragma once

#include <BaseECS/system.hpp>
#include <constants.hpp>

class ECSManager;

class VisionSystem : public ecs::System
{
public:
    VisionSystem(const ECSManager* memECSManager);
    void memECSManager(const ECSManager *memECSMan);
    void execSystem()override;
private:
    void setUsedComponents();
    void treatVisible(uint32_t observerId, uint32_t checkVisibleId, CollisionShape_e shapeElement);
private:
    const ECSManager* m_memECSManager;
};
