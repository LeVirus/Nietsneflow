#pragma once

#include <includesLib/BaseECS/system.hpp>

class DoorSystem : public ecs::System
{
public:
    DoorSystem();
    void execSystem()override;
private:
    void setUsedComponents();
};
