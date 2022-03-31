#pragma once


#include <ECS/Systems/InputSystem.hpp>
#include <ECS/Systems/CollisionSystem.hpp>
#include "constants.hpp"

struct MoveableComponent;
struct PositionVertexComponent;
struct MapCoordComponent;
struct VisionComponent;
class DoorWallSystem;
class IASystem;

class PhysicalEngine
{
public:
    PhysicalEngine();
    void runIteration(bool gamePaused);
    void linkSystems(InputSystem *inputSystem, CollisionSystem * collisionSystem,
                     DoorWallSystem *doorSystem, IASystem *iaSystem);
    void memPlayerEntity(uint32_t playerEntity);
    void confPlayerVisibleShoot(std::vector<uint32_t> &visibleShots,
                                const PairFloat_t &point, float degreeAngle);
    void setModeTransitionMenu(bool transition);
    void clearSystems();
    void setKeyboardKey(const std::array<uint32_t, static_cast<uint32_t>(ControlKey_e::TOTAL)> &keyboardArray);
    void setGamepadKey(const std::array<GamepadInputState, static_cast<uint32_t>(ControlKey_e::TOTAL)> &gamepadArray);
    inline const std::vector<uint32_t> &getStaticEntitiesToDelete()const
    {
        return m_collisionSystem->getStaticEntitiesToDelete();
    }
    inline const std::vector<uint32_t> &getBarrelEntitiesDestruct()const
    {
        return m_collisionSystem->getBarrelEntitiesDestruct();
    }
    inline void clearVectBarrelsDestruct()
    {
        m_collisionSystem->clearVectBarrelsDestruct();
    }
    inline void clearVectObjectToDelete()
    {
        m_collisionSystem->clearVectObjectToDelete();
    }
    inline void reinitToggleFullScreen()
    {
        m_inputSystem->reinitToggleFullScreen();
    }
    inline bool toogledFullScreenSignal()const
    {
        return m_inputSystem->toggleFullScreenSignal();
    }
private:
    InputSystem *m_inputSystem = nullptr;
    CollisionSystem *m_collisionSystem = nullptr;
    DoorWallSystem *m_doorSystem = nullptr;
    IASystem *m_iaSystem = nullptr;
};

void moveElementFromAngle(float distanceMove, float radiantAngle, PairFloat_t &point, bool playerMove = false);
void updatePlayerOrientation(const MoveableComponent &moveComp,
                             PositionVertexComponent &posComp, VisionComponent &visionComp);
void updatePlayerArrow(const MoveableComponent &moveComp,
                             PositionVertexComponent &posComp);
void updatePlayerConeVision(const MoveableComponent &moveComp,
                             VisionComponent &visionComp);
float getRadiantAngle(float angle);
float getDegreeAngle(float angle);
