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
    uint32_t m_damage = 5;
    static std::array<uint32_t, 4> m_impactEntities;
    bool m_destructPhase = false;
    ShotPhase_e m_spritePhaseShot = ShotPhase_e::NORMAL;
    virtual ~ShotConfComponent() = default;
};
