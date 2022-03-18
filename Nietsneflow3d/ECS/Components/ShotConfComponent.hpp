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
    bool m_destructPhase = false, m_ejectMode = false;
    uint32_t m_spriteShotNum = 0, m_impactEntity;
    std::optional<uint32_t> m_damageCircleRayData;
    float m_ejectExplosionRay = 10.0f;
    virtual ~ShotConfComponent() = default;
};
