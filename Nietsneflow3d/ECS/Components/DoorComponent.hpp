#pragma once

#include <BaseECS/component.hpp>
#include <constants.hpp>

enum class DoorState_e
{
    STATIC_OPEN,
    STATIC_CLOSED,
    MOVE_OPEN,
    MOVE_CLOSE
};

struct DoorComponent : public ecs::Component
{
    DoorComponent()
    {
        muiTypeComponent = Components_e::DOOR_COMPONENT;
    }
    DoorState_e m_currentState = DoorState_e::STATIC_CLOSED;
    bool m_vertical;
    float m_speedMove = 1.0f;
    virtual ~DoorComponent() = default;
};
