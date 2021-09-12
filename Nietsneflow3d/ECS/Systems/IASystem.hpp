#pragma once

#include <includesLib/BaseECS/system.hpp>
#include "constants.hpp"

struct MapCoordComponent;
struct EnemyConfComponent;
struct MoveableComponent;
struct PlayerConfComponent;
class MainEngine;

class IASystem : public ecs::System
{
public:
    IASystem();
    void execSystem()override;
    void memPlayerDatas(uint32_t playerEntity);
    void confVisibleShoot(std::vector<uint32_t> &visibleShots,
                          const PairFloat_t &point, float degreeAngle, CollisionTag_e tag);
    inline void linkMainEngine(MainEngine *mainEngine)
    {
        m_mainEngine = mainEngine;
    }
private:
    void confNewVisibleShot(const std::vector<uint32_t> &visibleShots);
    bool checkEnemyTriggerAttackMode(float radiantAngle, float distancePlayer,
                                     MapCoordComponent *enemyMapComp);
    void treatEnemyBehaviourAttack(uint32_t enemyEntity, MapCoordComponent *enemyMapComp, float radiantAnglePlayerDirection,
                                   EnemyConfComponent *enemyConfComp, float distancePlayer);
    void updateEnemyDirection(EnemyConfComponent *enemyConfComp, MoveableComponent *moveComp,
                              MapCoordComponent *enemyMapComp);
    void enemyShoot(EnemyConfComponent *enemyConfComp, MoveableComponent *moveComp,
                    MapCoordComponent *enemyMapComp, float distancePlayer);
    void treatVisibleShots(const std::vector<uint32_t> &stdAmmo);
    void treatVisibleShot(uint32_t numEntity);
private:
    uint32_t m_playerEntity;
    MapCoordComponent *m_playerMapComp = nullptr;
    PlayerConfComponent *m_playerComp = nullptr;
    float m_distanceEnemyBehaviour = LEVEL_TILE_SIZE_PX * 15.0f;
    MainEngine *m_mainEngine;
};

