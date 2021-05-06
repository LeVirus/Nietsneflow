#pragma once

#include <GraphicEngine.hpp>
#include <PhysicalEngine.hpp>
#include <ECS/ECSManager.hpp>

class LevelManager;
class Level;
class EnemyData;
class FontData;
struct EnemyConfComponent;

class MainEngine
{
public:
    MainEngine() = default;
    void loadGraphicPicture(const PictureData &picData, const FontData &fontData);
    void init();
    void loadLevelEntities(const LevelManager &levelManager);
    void mainLoop();
    void playerShoot(PlayerConfComponent *playerComp, const pairFloat_t &point,
               float degreeAngle);
    void setUnsetPaused();
    inline bool isGamePaused()
    {
        return m_gamePaused;
    }
    void confSystems();
private:
    void loadGroundAndCeilingEntities(const GroundCeilingData &groundData,
                                      const GroundCeilingData &ceilingData);
    void confCeilingComponents(uint32_t entityNum);
    void confMenuCursorEntity(PlayerConfComponent *playerConf);
    void confWriteEntities(PlayerConfComponent *playerConf);
    void confGroundComponents(uint32_t entityNum);
    void linkSystemsToGraphicEngine();
    void linkSystemsToPhysicalEngine();
    void loadPlayerVisibleShotsSprite(const std::vector<SpriteData> &vectSpriteData, const std::vector<uint8_t> &vectSprite,
                                      const ammoContainer_t &ammoEntities);
    void loadPlayerEntity(const std::vector<SpriteData> &vectSpriteData, const Level &level, uint32_t numWeaponEntity);
    void confPlayerEntity(const std::vector<SpriteData> &vectSpriteData, uint32_t entityNum, const Level &level, uint32_t numWeaponEntity);
    uint32_t loadWeaponsEntity(const LevelManager &levelManager);
    void loadWallEntities(const LevelManager &levelManager);
    void loadDoorEntities(const LevelManager &levelManager);
    void loadEnemiesEntities(const LevelManager &levelManager);
    void confVisibleAmmo(const ammoContainer_t &ammoCont);
    void loadStaticElementEntities(const LevelManager &levelManager);
    void createAmmosEntities(ammoContainer_t &ammoCont, CollisionTag_e collTag, bool visibleShot = false);
    uint32_t loadWeaponEntity();
    uint32_t createWallEntity();
    uint32_t createDoorEntity();
    uint32_t createEnemyEntity();
    uint32_t createShotEntity();
    uint32_t createVisibleShotEntity();
    uint32_t createWriteEntity();
    uint32_t createSimpleSpriteEntity();
    void confBaseComponent(uint32_t entityNum, const SpriteData &memSpriteData,
                           const pairUI_t &coordLevel, CollisionShape_e collisionShape);
    uint32_t createStaticEntity();
    void confStaticComponent(uint32_t entityNum, const pairFloat_t &elementSize,
                             bool traversable, LevelStaticElementType_e type);
    void loadEnemySprites(const std::vector<SpriteData> &vectSprite,
                          const std::vector<EnemyData> &enemiesData, uint32_t numEntity, const ammoContainer_t &visibleAmmo);
    void memTimerPausedValue();
    void applyTimerPausedValue();
    void confPlayerShoot(const ammoContainer_t &playerVisibleShots, const pairFloat_t &point, float degreeAngle);
private:
    GraphicEngine m_graphicEngine;
    PhysicalEngine m_physicalEngine;
    ECSManager m_ecsManager;
    std::vector<std::pair<uint32_t, time_t>> m_vectMemPausedTimer;
    bool m_gamePaused = false;
    SpriteData const *m_memCursorSpriteData = nullptr, *m_memVisibleShotA = nullptr;
    pairFloat_t m_menuCornerUpLeft = {-0.5f, 0.5f};
};

void confBullet(GeneralCollisionComponent *genColl, SegmentCollisionComponent *segmentColl,
                CollisionTag_e collTag, const pairFloat_t &point, float degreeAngle);
void setWeaponPlayer();
