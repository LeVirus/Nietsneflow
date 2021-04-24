#pragma once

#include <BaseECS/component.hpp>
#include <constants.hpp>

enum class EnemyDisplayMode_e
{
    NORMAL,
    SHOOTING,
    DYING,
    DEAD
};

enum class EnemyBehaviourMode_e
{
    PASSIVE,
    ATTACK,
    DEAD
};

struct EnemyConfComponent : public ecs::Component
{
    EnemyConfComponent()
    {
        muiTypeComponent = Components_e::ENEMY_CONF_COMPONENT;
    }
    //return false if dead
    bool takeDamage(uint32_t damage)
    {
        if(m_life <= damage)
        {
            m_life = 0;
            return false;
        }
        else
        {
            m_life -= damage;
            return true;
        }
    }
    bool m_enemyShoot = false, m_staticPhase;
    uint32_t m_weaponEntity, m_life = 1;
    ammoContainer_t m_ammo;
    EnemyDisplayMode_e m_displayMode = EnemyDisplayMode_e::NORMAL;
    EnemySpriteType_e m_visibleOrientation;
    EnemyBehaviourMode_e m_behaviourMode = EnemyBehaviourMode_e::PASSIVE;
    virtual ~EnemyConfComponent() = default;
};

