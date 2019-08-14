#ifndef PHYSICALENGINE_H
#define PHYSICALENGINE_H

#include <ECS/Systems/InputSystem.hpp>
#include <ECS/Systems/CollisionSystem.hpp>
#include "constants.hpp"

struct MoveableComponent;
struct PositionVertexComponent;
struct MapCoordComponent;

class PhysicalEngine
{
private:
    InputSystem *m_inputSystem = nullptr;
    CollisionSystem *m_collisionSystem = nullptr;
private:
public:
    PhysicalEngine();
    void runIteration();
    void linkSystems(InputSystem *inputSystem,
                     CollisionSystem * collisionSystem);
};

void movePlayer(MoveableComponent &moveComp,
                MapCoordComponent &mapComp, MoveOrientation_e moveDirection);
void updatePlayerOrientation(const MoveableComponent &moveComp,
                              PositionVertexComponent &posComp);
float getRadiantAngle(int32_t angle);
#endif // PHYSICALENGINE_H
