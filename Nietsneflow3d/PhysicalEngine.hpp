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
    void setKeyboardKey(const std::array<MouseKeyboardInputState, static_cast<uint32_t>(ControlKey_e::TOTAL)> &keyboardArray);
    void setGamepadKey(const std::array<GamepadInputState, static_cast<uint32_t>(ControlKey_e::TOTAL)> &gamepadArray);
    inline void updateZonesColl()
    {
        m_collisionSystem->updateZonesColl();
    }
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
    inline void updateMousePos()
    {
        m_inputSystem->updateMousePos();
    }
    inline uint32_t getTurnSensitivity()const
    {
        return m_inputSystem->getTurnSensitivity();
    }
    inline void updateTurnSensitivity(uint32_t turnSensitivity)
    {
        return m_inputSystem->setTurnSensitivity(turnSensitivity);
    }
    inline void addEntityToZone(uint32_t entity, const PairUI_t &coord)
    {
        m_collisionSystem->addEntityToZone(entity, coord);
    }
    inline void removeEntityToZone(uint32_t entity)
    {
        m_collisionSystem->removeEntityToZone(entity);
    }
private:
    InputSystem *m_inputSystem = nullptr;
    CollisionSystem *m_collisionSystem = nullptr;
    DoorWallSystem *m_doorSystem = nullptr;
    IASystem *m_iaSystem = nullptr;
};

void moveElementFromAngle(float distanceMove, float radiantAngle, PairFloat_t &point, bool playerMove = false);
void updatePlayerArrow(const MoveableComponent &moveComp, PositionVertexComponent &posComp);
float getRadiantAngle(float angle);
float getDegreeAngle(float angle);
