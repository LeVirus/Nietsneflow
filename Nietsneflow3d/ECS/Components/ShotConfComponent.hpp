#pragma once

#include <BaseECS/component.hpp>
#include <constants.hpp>
#include <optional>

struct ShotConfComponent : public ecs::Component
{
    ShotConfComponent()
    {
        muiTypeComponent = Components_e::SHOT_CONF_COMPONENT;
    }
    uint32_t m_damage;
    bool m_destructPhase = false;
    uint32_t m_spriteShotNum = 0, m_impactEntity;
    std::optional<uint32_t> m_damageCircleRayData;
    virtual ~ShotConfComponent() = default;
};
