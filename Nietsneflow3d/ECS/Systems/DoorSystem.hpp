#pragma once

#include <includesLib/BaseECS/system.hpp>

struct DoorComponent;

class DoorSystem : public ecs::System
{
public:
    DoorSystem();
    void execSystem()override;
private:
    void setUsedComponents();
    void treatDoorMovementSize(DoorComponent *doorComp, uint32_t entityNum);
private:
    double m_timeDoorClosed = 0.3;
};
