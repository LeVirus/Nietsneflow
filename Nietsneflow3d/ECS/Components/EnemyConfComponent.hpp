#pragma once

#include <BaseECS/component.hpp>
#include <constants.hpp>

enum class EnemyMode_e
{
    NORMAL,
    SHOOTING,
    DYING,
    DEAD
};

struct EnemyConfComponent : public ecs::Component
{
    EnemyConfComponent()
    {
        muiTypeComponent = Components_e::ENEMY_CONF_COMPONENT;
    }
    void takeDamage(uint32_t damage)
    {
        if(m_life <= damage)
        {
            m_life = 0;
        }
        else
        {
            --m_life;
        }
    }
    bool m_enemyShoot = false;
    uint32_t m_weaponEntity, m_life = 1;
    EnemyMode_e m_mode = EnemyMode_e::NORMAL;
    virtual ~EnemyConfComponent() = default;
};

