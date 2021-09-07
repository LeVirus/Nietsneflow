#pragma once

#include <BaseECS/component.hpp>
#include <constants.hpp>

struct TeleportComponent : public ecs::Component
{
    TeleportComponent()
    {
        muiTypeComponent = Components_e::TELEPORT_COMPONENT;
    }
    pairUI_t m_targetPos;
    virtual ~TeleportComponent() = default;
};
