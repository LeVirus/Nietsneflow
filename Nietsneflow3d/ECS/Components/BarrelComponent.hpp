#pragma once

#include <BaseECS/component.hpp>
#include <constants.hpp>

struct BarrelComponent : public ecs::Component
{
    BarrelComponent()
    {
        muiTypeComponent = Components_e::BARREL_COMPONENT;
    }
    uint32_t m_life, m_damageZoneEntity, m_memPosExplosionSprite;
    bool m_destructPhase = false;
    virtual ~BarrelComponent() = default;
};
