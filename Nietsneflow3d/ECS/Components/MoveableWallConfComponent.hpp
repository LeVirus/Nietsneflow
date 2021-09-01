#pragma once

#include <BaseECS/component.hpp>
#include <constants.hpp>

struct MoveableWallConfComponent : public ecs::Component
{
    MoveableWallConfComponent()
    {
        muiTypeComponent = Components_e::MOVEABLE_WALL_CONF_COMPONENT;
    }
    std::vector<std::pair<Direction_e, uint32_t>> m_directionMove;
    uint32_t m_currentPhase = 0, m_currentMove;
    pairFloat_t m_nextPhasePos;
    TriggerWallMoveType_e m_triggerType;
    TriggerBehaviourType_e m_triggerBehaviour;
    bool m_inMovement = false, m_initPos = false;
    virtual ~MoveableWallConfComponent() = default;
};
