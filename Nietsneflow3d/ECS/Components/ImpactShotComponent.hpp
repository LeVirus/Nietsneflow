#pragma once

#include <BaseECS/component.hpp>
#include <constants.hpp>

enum class ImpactPhase_e
{
    FIRST,
    SECOND
};

struct ImpactShotComponent : public ecs::Component
{
    ImpactShotComponent()
    {
        muiTypeComponent = Components_e::IMPACT_CONF_COMPONENT;
    }
    ImpactPhase_e m_spritePhase = ImpactPhase_e::FIRST;
    virtual ~ImpactShotComponent() = default;
};
