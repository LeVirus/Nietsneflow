#pragma once

#include <BaseECS/component.hpp>
#include <constants.hpp>

struct WeaponComponent : public ecs::Component
{
    WeaponComponent()
    {
        muiTypeComponent = Components_e::WEAPON_COMPONENT;
    }
    std::vector<pairUI_t> m_memPosSprite;
    std::vector<uint32_t> m_maxAmmoWeapons;
    virtual ~WeaponComponent() = default;
};
