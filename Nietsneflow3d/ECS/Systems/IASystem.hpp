#pragma once

#include <includesLib/BaseECS/system.hpp>
#include "constants.hpp"

struct MapCoordComponent;
struct EnemyConfComponent;
struct MoveableComponent;
struct PlayerConfComponent;

class IASystem : public ecs::System
{
public:
    IASystem();
    void execSystem()override;
    void loadPlayerDatas(uint32_t playerEntity);
    void confVisibleShoot(const ArrayWeaponVisibleShot_t &visibleShots,
                          const pairFloat_t &point, float degreeAngle);
private:
    bool checkEnemyTriggerAttackMode(float radiantAngle, float distancePlayer,
                                     MapCoordComponent *enemyMapComp);
    void treatEnemyBehaviourAttack(uint32_t enemyEntity, MapCoordComponent *enemyMapComp, float radiantAnglePlayerDirection,
                                   EnemyConfComponent *enemyConfComp, float distancePlayer);
    void updateEnemyDirection(EnemyConfComponent *enemyConfComp, MoveableComponent *moveComp,
                              MapCoordComponent *enemyMapComp);
    void enemyShoot(EnemyConfComponent *enemyConfComp, MoveableComponent *moveComp,
                    MapCoordComponent *enemyMapComp);
    void treatVisibleShots(const std::array<uint32_t, TOTAL_SHOT_NUMBER> &stdAmmo);
    void treatEnemyVisibleShots(const ArrayWeaponVisibleShot_t &stdAmmo);
    void treatVisibleShot(uint32_t numEntity);
private:
    uint32_t m_playerEntity;
    MapCoordComponent *m_playerMapComp = nullptr;
    PlayerConfComponent *m_playerComp = nullptr;
    float m_distanceEnemyBehaviour = LEVEL_TILE_SIZE_PX * 15.0f;
};

