#ifndef COLLISIONSYSTEM_H
#define COLLISIONSYSTEM_H

#include <includesLib/BaseECS/system.hpp>

class CollisionSystem : public ecs::System
{
private:
    void setUsedComponents();
public:
    CollisionSystem();
    void execSystem()override;
};

#endif // COLLISIONSYSTEM_H
