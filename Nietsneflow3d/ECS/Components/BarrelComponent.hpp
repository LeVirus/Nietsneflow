#pragma once

#include <BaseECS/component.hpp>
#include <constants.hpp>

struct BarrelComponent : public ecs::Component
{
    BarrelComponent()
    {
        muiTypeComponent = Components_e::BARREL_COMPONENT;
    }
    uint32_t m_life, m_damageZoneEntity;
    PairUI_t m_memPosSprite;
    virtual ~BarrelComponent() = default;
};
