#pragma once

#include <includesLib/BaseECS/system.hpp>
#include "constants.hpp"

struct MapCoordComponent;
struct EnemyConfComponent;

class IASystem : public ecs::System
{
public:
    IASystem();
    void execSystem()override;
    void loadPlayerDatas(uint32_t playerEntity);
private:
    void treatEnemyBehaviourAttack(uint32_t enemyEntity, MapCoordComponent *enemyMapComp,
                                   EnemyConfComponent *enemyConfComp);
private:
    uint32_t m_playerEntity;
    MapCoordComponent *m_playerMapComp = nullptr;
    float m_distanceEnemyBehaviour = LEVEL_TILE_SIZE_PX * 4.0f;
};
