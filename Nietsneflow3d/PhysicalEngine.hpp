#ifndef PHYSICALENGINE_H
#define PHYSICALENGINE_H

#include <ECS/Systems/InputSystem.hpp>

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

#endif // PHYSICALENGINE_H
