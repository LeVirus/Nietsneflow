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
    void activeDoorSound(uint32_t entityNum);
    void treatMoveableWalls();
    void treatTriggers();
    bool triggerMoveableWall(uint32_t wallEntity);
    void switchToNextPhaseMoveWall(uint32_t wallEntity, MapCoordComponent *mapComp,
                                   MoveableWallConfComponent *moveWallComp,
                                   const PairUI_t &previousPos);
private:
    double m_timeDoorClosed = 0.6;
    std::vector<uint32_t> m_vectMoveableWall, m_vectTrigger;
    ECSManager const *m_ECSManager;
};

Direction_e getReverseDirection(Direction_e dir);
void stopMoveWallLevelLimitCase(MapCoordComponent *mapComp, MoveableWallConfComponent *moveWallComp);
void setInitPhaseMoveWall(MapCoordComponent *mapComp, MoveableWallConfComponent *moveWallComp,
                          Direction_e currentDir, uint32_t wallEntity);
void reverseDirection(MoveableWallConfComponent *moveWallComp);
