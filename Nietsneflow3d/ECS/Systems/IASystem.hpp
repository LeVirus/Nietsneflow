#pragma once

#include <includesLib/BaseECS/system.hpp>
#include "constants.hpp"
#include <ECS/Components/EnemyConfComponent.hpp>
#include <ECS/Components/AudioComponent.hpp>
#include <ECS/NewComponentManager.hpp>

struct MapCoordComponent;
struct EnemyConfComponent;
struct MoveableComponent;
struct PlayerConfComponent;
class MainEngine;
class ECSManager;

class IASystem : public ecs::System
{
public:
    IASystem(NewComponentManager &newComponentManager, ECSManager *memECSManager);
    void execSystem()override;
    void memPlayerDatas(uint32_t playerEntity);
    void confVisibleShoot(std::vector<uint32_t> &visibleShots, const PairFloat_t &point, float degreeAngle, CollisionTag_e tag);
    inline void linkMainEngine(MainEngine *mainEngine)
    {
        m_mainEngine = mainEngine;
    }
private:
    void treatEject();
    void confNewVisibleShot(const std::vector<uint32_t> &visibleShots);
    bool checkEnemyTriggerAttackMode(float radiantAngle, float distancePlayer, MapCoordComponent &enemyMapComp);
    void treatEnemyBehaviourAttack(uint32_t enemyEntity, MapCoordComponent &enemyMapComp, float radiantAnglePlayerDirection,
                                   EnemyConfComponent &enemyConfComp, float distancePlayer);
    void updateEnemyDirection(EnemyConfComponent &enemyConfComp, MoveableComponent &moveComp, MapCoordComponent &enemyMapComp);
    void enemyShoot(EnemyConfComponent &enemyConfComp, MoveableComponent &moveComp, MapCoordComponent &enemyMapComp, float distancePlayer);
    void treatVisibleShots(const std::vector<uint32_t> &stdAmmo);
    void treatVisibleShot(uint32_t numEntity);
    void activeSound(uint32_t entityNum, uint32_t soundNum);
private:
    NewComponentManager &m_newComponentManager;
    ComponentsGroup &m_componentsContainer;
    uint32_t m_playerEntity, m_intervalEnemyBehaviour = 0.4 / FPS_VALUE, m_intervalVisibleShotLifeTime = 5.0 / FPS_VALUE,
    m_intervalEnemyPlayPassiveSound = 5.0 / FPS_VALUE;
    float m_distanceEnemyBehaviour = LEVEL_TILE_SIZE_PX * 9.0f;
    MainEngine *m_mainEngine;
    std::vector<uint32_t> m_vectMoveableEntities;
    ECSManager *m_memECSManager;
    std::vector<SoundElement> m_memPlayerVisibleShot;
};

