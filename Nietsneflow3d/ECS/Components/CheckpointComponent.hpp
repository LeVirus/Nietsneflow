#pragma once

#include <BaseECS/component.hpp>
#include <constants.hpp>

struct CheckpointComponent : public ecs::Component
{
    CheckpointComponent()
    {
        muiTypeComponent = Components_e::CHECKPOINT_COMPONENT;
    }
    uint32_t m_checkpointNumber;
    Direction_e m_direction;
    virtual ~CheckpointComponent() = default;
};
