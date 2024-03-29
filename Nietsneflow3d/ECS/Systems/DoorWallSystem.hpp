#pragma once

#include <includesLib/BaseECS/system.hpp>
#include <constants.hpp>
#include <ECS/NewComponentManager.hpp>

class ECSManager;
struct DoorComponent;
struct MapCoordComponent;
struct MoveableWallConfComponent;
class MainEngine;

class DoorWallSystem : public ecs::System
{
public:
    DoorWallSystem(NewComponentManager &newComponentManager, const ECSManager* memECSManager);
    void execSystem()override;
    void clearSystem();
    void memRefMainEngine(MainEngine *mainEngine);
private:
    void treatDoorMovementSize(DoorComponent &doorComp, uint32_t entityNum);
    void updateEntities();
    void treatDoors();
    void activeDoorSound(uint32_t entityNum);
    void treatMoveableWalls();
    void treatTriggers();
    void triggerMoveableWall(uint32_t wallEntity);
    void switchToNextPhaseMoveWall(uint32_t wallEntity, MapCoordComponent &mapComp,
                                   MoveableWallConfComponent &moveWallComp,
                                   const PairUI_t &previousPos);
private:
    NewComponentManager &m_newComponentManager;
    ComponentsGroup &m_componentsContainer;
    double m_doorCyclesForClose;
    std::vector<uint32_t> m_vectMoveableWall, m_vectTrigger;
    ECSManager const *m_ECSManager;
    MainEngine *m_refMainEngine;
};

Direction_e getReverseDirection(Direction_e dir);
void stopMoveWallLevelLimitCase(MapCoordComponent &mapComp, MoveableWallConfComponent &moveWallComp);
void setInitPhaseMoveWall(MapCoordComponent &mapComp, MoveableWallConfComponent &moveWallComp,
                          Direction_e currentDir, uint32_t wallEntity);
void reverseDirection(MoveableWallConfComponent &moveWallComp);
