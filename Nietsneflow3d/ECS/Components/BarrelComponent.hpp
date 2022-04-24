#pragma once

#include <BaseECS/component.hpp>
#include <constants.hpp>

struct BarrelComponent : public ecs::Component
{
    BarrelComponent()
    {
        muiTypeComponent = Components_e::BARREL_COMPONENT;
    }
    uint32_t m_life, m_damageZoneEntity, m_memPosExplosionSprite,
    m_timeStaticPhase = 0.15 / FPS_VALUE;
    bool m_destructPhase = false;
    virtual ~BarrelComponent() = default;
};
