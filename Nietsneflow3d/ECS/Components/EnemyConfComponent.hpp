#pragma once

#include <BaseECS/component.hpp>
#include <map>
#include <cassert>
#include <functional>
#include <constants.hpp>

using pairEnemySprite_t = std::pair<EnemySpriteType_e, PairUI_t>;
using mapEnemySprite_t = std::map<EnemySpriteType_e, PairUI_t>;

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
    MOVE_TO_TARGET_FRONT,//MUST BE FIRST
    MOVE_TO_TARGET_RIGHT,
    MOVE_TO_TARGET_LEFT,
    MOVE_TO_TARGET_DIAG_RIGHT,
    MOVE_TO_TARGET_DIAG_LEFT,
    SHOOT,
    SHOOTED,//FROZEN MUST BE BEFORE TOTAL
    TOTAL
};

enum class EnemySoundEffect_e
{
    NORMAL,
    DETECT,
    ATTACK,
    DEATH
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
    bool m_stuck = false, m_touched = false, m_visibleShot, m_endLevel = false, m_meleeOnly, m_frozenOnAttack, m_playDeathSound = false;
    uint32_t m_weaponEntity, m_life, m_countPlayerInvisibility = 0, m_currentSprite, m_cycleNumberSpriteUpdate = 0.2 / FPS_VALUE,
    m_cycleNumberDyingInterval = 0.11 / FPS_VALUE, m_cycleNumberAttackInterval = 0.15f / FPS_VALUE, m_standardSpriteInterval = 0.5 / FPS_VALUE,
    m_countTillLastAttack = 0;
    std::vector<uint32_t> m_stdAmmo, m_visibleAmmo;
    EnemyDisplayMode_e m_displayMode = EnemyDisplayMode_e::NORMAL;
    //give first and last emplacement of sprite from type
    mapEnemySprite_t m_mapSpriteAssociate;
    std::optional<uint32_t> m_dropedObjectEntity;
    EnemyBehaviourMode_e m_behaviourMode = EnemyBehaviourMode_e::PASSIVE;
    //fist previous -1, second previous -2
    std::array<EnemyAttackPhase_e, 3> m_previousMove = {EnemyAttackPhase_e::TOTAL, EnemyAttackPhase_e::TOTAL, EnemyAttackPhase_e::TOTAL};
    EnemyAttackPhase_e m_attackPhase;
    std::optional<uint32_t> m_meleeAttackDamage;
    virtual ~EnemyConfComponent() = default;
};

