#pragma once

#include <BaseECS/component.hpp>
#include <constants.hpp>

struct TriggerWallCheckpointData
{
    uint32_t m_wallNum, m_numberOfMove;
};

struct MoveableWallConfComponent : public ecs::Component
{
    MoveableWallConfComponent()
    {
        muiTypeComponent = Components_e::MOVEABLE_WALL_CONF_COMPONENT;
    }
    std::vector<std::pair<Direction_e, uint32_t>> m_directionMove;
    uint32_t m_currentPhase = 0, m_currentMove;
    PairFloat_t m_nextPhasePos;
    TriggerWallMoveType_e m_triggerType;
    TriggerBehaviourType_e m_triggerBehaviour;
    //first shape num, wall num
    std::optional<std::pair<uint32_t, TriggerWallCheckpointData>> m_triggerWallCheckpointData;
    bool m_cycleInMovement = false, m_inMovement = false, m_initPos = false, m_actionned = false,
    m_manualTrigger = false;
    virtual ~MoveableWallConfComponent() = default;
};
