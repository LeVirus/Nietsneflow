#pragma once

#include <includesLib/BaseECS/system.hpp>
#include <constants.hpp>

class ECSManager;
struct DoorComponent;
struct MapCoordComponent;
struct MoveableWallConfComponent;

class DoorWallSystem : public ecs::System
{
public:
    DoorWallSystem(const ECSManager* memECSManager);
    void execSystem()override;
    void clearSystem();
private:
    void treatDoorMovementSize(DoorComponent *doorComp, uint32_t entityNum);
    void updateEntities();
    void treatDoors();
    void treatWalls();
private:
    double m_timeDoorClosed = 0.6;
    std::vector<uint32_t> m_vectMoveableWall;
    ECSManager const *m_ECSManager;
};

void setInitPhaseMoveWall(MapCoordComponent *mapComp, MoveableWallConfComponent *moveWallComp,
                          Direction_e currentDir, uint32_t wallEntity);
void switchToNextPhaseMoveWall(MapCoordComponent *mapComp, MoveableWallConfComponent *moveWallComp, const pairUI_t &previousPos);
