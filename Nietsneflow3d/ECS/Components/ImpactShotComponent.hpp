#pragma once

#include <BaseECS/component.hpp>
#include <constants.hpp>

enum class ImpactPhase_e
{
    FIRST,
    SECOND,
    TOUCHED
};

struct ImpactShotComponent : public ecs::Component
{
    ImpactShotComponent()
    {
        muiTypeComponent = Components_e::IMPACT_CONF_COMPONENT;
    }
    bool m_touched;
    ImpactPhase_e m_spritePhase = ImpactPhase_e::FIRST;
    float m_moveUp = EPSILON_FLOAT;
    virtual ~ImpactShotComponent() = default;
};
