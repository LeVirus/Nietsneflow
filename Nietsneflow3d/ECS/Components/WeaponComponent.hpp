#pragma once

#include <BaseECS/component.hpp>
#include <constants.hpp>


struct WeaponData
{
    uint32_t m_ammunationsCount, m_simultaneousShots, m_maxAmmunations, m_lastAnimNum, m_weaponPower, m_currentBullet = 0;
    AttackType_e m_attackType;
    AnimationMode_e m_animMode;
    PairUI_t m_memPosSprite;
    bool m_posses = false;
    std::optional<std::vector<uint32_t>> m_visibleShootEntities, m_segmentShootEntities;
    std::string m_visibleShotID, m_impactID, m_weaponName;
    std::optional<float> m_damageRay;
    float m_latency, m_shotVelocity;
};

struct WeaponComponent : public ecs::Component
{
    WeaponComponent()
    {
        muiTypeComponent = Components_e::WEAPON_COMPONENT;
    }
    inline uint32_t getStdCurrentWeaponSprite()
    {
        return m_weaponsData[m_currentWeapon].m_memPosSprite.first;
    }
    std::map<uint32_t, uint32_t> m_reloadSoundAssociated;
    std::vector<WeaponData> m_weaponsData;
    uint32_t m_numWeaponSprite, m_currentWeapon = 10000, m_previousWeapon;
    PairFloat_t m_currentWeaponMove = {-0.02f, -0.01f};
    bool m_timerShootActive = false, m_shootFirstPhase, m_weaponChange = false,
    m_spritePositionCorrected = true;
    virtual ~WeaponComponent() = default;
};
