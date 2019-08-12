#ifndef PHYSICALENGINE_H
#define PHYSICALENGINE_H

#include <ECS/Systems/InputSystem.hpp>
#include "constants.hpp"

struct MoveableComponent;
struct PositionVertexComponent;
struct MapCoordComponent;

class PhysicalEngine
{
private:
    InputSystem *m_inputSystem = nullptr;
private:
public:
    PhysicalEngine();
    void runIteration();
    void linkSystems(InputSystem *inputSystem);
};

void movePlayer(MoveableComponent &moveComp,
                MapCoordComponent &mapComp, MoveOrientation_e moveDirection);
void updatePlayerOrientation(const MoveableComponent &moveComp,
                              PositionVertexComponent &posComp);
float getRadiantAngle(int32_t angle);
#endif // PHYSICALENGINE_H
