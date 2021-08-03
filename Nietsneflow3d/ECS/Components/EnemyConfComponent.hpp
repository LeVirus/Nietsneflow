#pragma once

#include <BaseECS/component.hpp>
#include <map>
#include <cassert>
#include <constants.hpp>

using pairEnemySprite_t = std::pair<EnemySpriteType_e, pairUI_t>;
using mapEnemySprite_t = std::map<EnemySpriteType_e, pairUI_t>;

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
    DYING,
    DEAD
};

enum class EnemyAttackPhase_e
{
    MOVE_TO_TARGET_FRONT,
    MOVE_TO_TARGET_RIGHT,
    MOVE_TO_TARGET_LEFT,
    SHOOT,
    SHOOTED//FROZEN
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
    bool m_prevWall = false, m_touched = false, m_visibleShot = true;
    uint32_t m_weaponEntity, m_life = 3, m_countPlayerInvisibility = 0;
    AmmoContainer_t m_stdAmmo, m_visibleAmmo;
    EnemyDisplayMode_e m_displayMode = EnemyDisplayMode_e::NORMAL;
    //give first and last emplacement of sprite from type
    mapEnemySprite_t m_mapSpriteAssociate;
    uint32_t m_currentSprite;
    EnemyBehaviourMode_e m_behaviourMode = EnemyBehaviourMode_e::PASSIVE;
    EnemyAttackPhase_e m_attackPhase;
    float m_dyingInterval = 0.15f, m_attackInterval = 0.15f;
    virtual ~EnemyConfComponent() = default;
};

