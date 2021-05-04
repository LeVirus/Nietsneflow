#pragma once

#include <ECS/Systems/InputSystem.hpp>
#include <ECS/Systems/CollisionSystem.hpp>
#include "constants.hpp"

struct MoveableComponent;
struct PositionVertexComponent;
struct MapCoordComponent;
struct VisionComponent;
class DoorSystem;
class IASystem;

class PhysicalEngine
{
private:
    InputSystem *m_inputSystem = nullptr;
    CollisionSystem *m_collisionSystem = nullptr;
    DoorSystem *m_doorSystem = nullptr;
    IASystem *m_iaSystem = nullptr;
public:
    PhysicalEngine();
    void runIteration(bool gamePaused);
    void linkSystems(InputSystem *inputSystem, CollisionSystem * collisionSystem,
                     DoorSystem *doorSystem, IASystem *iaSystem);
    void memPlayerEntity(uint32_t playerEntity);
};

void moveElement(MoveableComponent &moveComp,
                MapCoordComponent &mapComp, MoveOrientation_e moveDirection);
void updatePlayerOrientation(const MoveableComponent &moveComp,
                             PositionVertexComponent &posComp, VisionComponent &visionComp);
void updatePlayerArrow(const MoveableComponent &moveComp,
                             PositionVertexComponent &posComp);
void updatePlayerConeVision(const MoveableComponent &moveComp,
                             VisionComponent &visionComp);
float getRadiantAngle(float angle);
float getDegreeAngle(float angle);
