#pragma once

#include <BaseECS/component.hpp>
#include <constants.hpp>

struct PlayerConfComponent : public ecs::Component
{
    PlayerConfComponent()
    {
        muiTypeComponent = Components_e::PLAYER_CONF_COMPONENT;
    }
    bool m_playerAction = false;
    virtual ~PlayerConfComponent() = default;
};
