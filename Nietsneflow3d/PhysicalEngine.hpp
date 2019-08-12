#ifndef PHYSICALENGINE_H
#define PHYSICALENGINE_H

#include <ECS/Systems/InputSystem.hpp>

struct MoveableComponent;
struct PositionVertexComponent;

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

void updatePlayerOrientation(const MoveableComponent &moveComp,
                              PositionVertexComponent &posComp);
float getRadiantAngle(int32_t angle);
#endif // PHYSICALENGINE_H
