#pragma once

#include <GraphicEngine.hpp>
#include <PhysicalEngine.hpp>
#include <ECS/ECSManager.hpp>

class LevelManager;
class Level;
class EnemyData;

class MainEngine
{
public:
    MainEngine() = default;
    void loadGraphicPicture(const PictureData &picData);
    void init();
    void loadLevelEntities(const LevelManager &levelManager);
    void mainLoop();
    void createShotFromPosition(const pairFloat_t &point, float degreeAngle,
                                CollisionTag_e collTag);
private:
    void loadGroundAndCeilingEntities(const GroundCeilingData &groundData,
                                      const GroundCeilingData &ceilingData);
    void confCeilingComponents(uint32_t entityNum);
    void confGroundComponents(uint32_t entityNum);
    void linkSystemsToGraphicEngine();
    void linkSystemsToPhysicalEngine();
    void loadPlayerEntity(const Level &level, uint32_t numWeaponEntity);
    void confPlayerEntity(uint32_t entityNum, const Level &level, uint32_t numWeaponEntity);
    uint32_t loadWeaponsEntity(const LevelManager &levelManager);
    void loadWallEntities(const LevelManager &levelManager);
    void loadDoorEntities(const LevelManager &levelManager);
    void loadEnemiesEntities(const LevelManager &levelManager);
    void loadStaticElementEntities(const LevelManager &levelManager);
    uint32_t loadWeaponEntity();
    uint32_t createWallEntity();
    uint32_t createDoorEntity();
    uint32_t createEnemyEntity();
    uint32_t createShotEntity();
    void confBaseComponent(uint32_t entityNum, const SpriteData &memSpriteData,
                           const pairUI_t &coordLevel, CollisionShape_e collisionShape);
    uint32_t createStaticEntity();
    void confStaticComponent(uint32_t entityNum, const pairFloat_t &elementSize,
                             bool traversable, LevelStaticElementType_e type);
    void loadEnemySprites(const std::vector<SpriteData> &vectSprite,
                         const std::vector<EnemyData> &enemiesData, uint32_t numEntity);
    void deleteEntities();
private:
    GraphicEngine m_graphicEngine;
    PhysicalEngine m_physicalEngine;
    ECSManager m_ecsManager;
    std::vector<uint32_t> m_vectEntitiesToDelete;
};

void setWeaponPlayer();
