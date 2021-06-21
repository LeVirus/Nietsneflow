#pragma once

#include <GraphicEngine.hpp>
#include <PhysicalEngine.hpp>
#include <ECS/ECSManager.hpp>
#include <Level.hpp>

class LevelManager;
class Level;
class FontData;
struct EnemyData;
struct EnemyConfComponent;

struct MemPlayerConf
{
    WeaponsType_e m_currentWeapon, m_previousWeapon;
    std::array<uint32_t, static_cast<uint32_t>(WeaponsType_e::TOTAL)> m_ammunationsCount;
    std::array<bool, static_cast<uint32_t>(WeaponsType_e::TOTAL)> m_weapons;
    uint32_t m_life;
};

class MainEngine
{
public:
    MainEngine() = default;
    void loadGraphicPicture(const PictureData &picData, const FontData &fontData);
    void init();
    void loadLevelEntities(const LevelManager &levelManager);
    bool mainLoop(bool &memGameOver);
    void playerAttack(uint32_t playerEntity, PlayerConfComponent *playerComp, const pairFloat_t &point,
               float degreeAngle);
    void setUnsetPaused();
    inline bool isGamePaused()
    {
        return m_gamePaused;
    }
    inline void clearLevel()
    {
        m_ecsManager.getEngine().RmAllEntity();
    }
    void confSystems();
private:
    void clearObjectToDelete();
    void memPlayerGear();
    void loadPlayerGear();
    void displayTransitionMenu();
    void memColorSystemBackgroundEntities(uint32_t ground, uint32_t ceiling);
    void loadDamageEntity();
    void loadTransitionEntity();
    void confUnifiedColorEntity(uint32_t entityNum, const tupleFloat_t &color);
    uint32_t createColorEntity();
    void loadGroundAndCeilingEntities(const GroundCeilingData &groundData,
                                      const GroundCeilingData &ceilingData);
    void confCeilingComponents(uint32_t entityNum);
    void confGroundComponents(uint32_t entityNum);
    void confMenuCursorEntity();
    void confWriteEntities();
    void linkSystemsToGraphicEngine();
    void linkSystemsToPhysicalEngine();
    void loadShotImpactSprite(const std::vector<SpriteData> &vectSpriteData,
                              const std::vector<uint8_t> &vectSprite,
                              std::array<uint32_t, SEGMENT_SHOT_NUMBER> &target);
    void loadPlayerVisibleShotsSprite(const std::vector<SpriteData> &vectSpriteData, const std::vector<uint8_t> &vectSprite,
                                      const AmmoContainer_t &ammoEntities);
    void loadPlayerEntity(const std::vector<SpriteData> &vectSpriteData, const Level &level, uint32_t numWeaponEntity);
    void confPlayerEntity(const std::vector<SpriteData> &vectSpriteData, uint32_t entityNum, const Level &level, uint32_t numWeaponEntity);
    void confActionEntity();
    void confAxeHitEntity();
    void confShotsEntities(const AmmoContainer_t &ammoEntities, uint32_t damageValue);
    uint32_t loadWeaponsEntity(const LevelManager &levelManager);
    uint32_t createBackgroundEntity(GroundCeilingData const *data);
    void loadWallEntities(const LevelManager &levelManager);
    void loadDoorEntities(const LevelManager &levelManager);
    void loadEnemiesEntities(const LevelManager &levelManager);
    void confVisibleAmmo(const AmmoContainer_t &ammoCont);
    void loadStaticElementEntities(const LevelManager &levelManager);
    void loadStaticElementGroup(const LevelManager &levelManager,
                                const std::map<std::string, StaticLevelElementData> &staticData, LevelStaticElementType_e elementType);
    void loadExitElement(const LevelManager &levelManager, const StaticLevelElementData &exit);
    void createAmmosEntities(AmmoContainer_t &ammoCont, CollisionTag_e collTag, bool visibleShot = false);
    void createShotImpactEntities(const std::vector<SpriteData> &vectSpriteData,
                                  const std::vector<uint8_t> &vectSprite,
                                  std::array<uint32_t, SEGMENT_SHOT_NUMBER> &entitiesContainer, const AmmoContainer_t &segmentShotContainer);
    uint32_t loadWeaponEntity();
    uint32_t createWallEntity();
    uint32_t createDoorEntity();
    uint32_t createEnemyEntity();
    uint32_t createShotEntity();
    uint32_t createVisibleShotEntity();
    uint32_t createShotImpactEntity();
    uint32_t createWriteEntity();
    uint32_t createSimpleSpriteEntity();
    uint32_t createStaticEntity();
    uint32_t createObjectEntity();
    void confBaseComponent(uint32_t entityNum, const SpriteData &memSpriteData,
                           const pairUI_t &coordLevel, CollisionShape_e collisionShape,
                           CollisionTag_e tag);
    void confStaticComponent(uint32_t entityNum, const pairFloat_t &elementSize, LevelStaticElementType_e elementType);
    void loadEnemySprites(const std::vector<SpriteData> &vectSprite,
                          const std::map<std::string, EnemyData> &enemiesData, uint32_t numEntity, const AmmoContainer_t &visibleAmmo);
    void loadVisibleShotEnemySprites(const std::vector<SpriteData> &vectSprite,
                                     const AmmoContainer_t &visibleAmmo,
                                     const EnemyData &enemyData);
    void memTimerPausedValue();
    void applyTimerPausedValue();
    void confPlayerShoot(const AmmoContainer_t &playerVisibleShots, const pairFloat_t &point, float degreeAngle);
private:
    GraphicEngine m_graphicEngine;
    PhysicalEngine m_physicalEngine;
    ECSManager m_ecsManager;
    std::vector<std::pair<uint32_t, time_t>> m_vectMemPausedTimer;
    bool m_gamePaused = false, m_playerMem = false;
    SpriteData const *m_memCursorSpriteData = nullptr, *m_memVisibleShotA = nullptr;
    pairFloat_t m_menuCornerUpLeft = {-0.5f, 0.5f};
    GeneralCollisionComponent *m_exitColl = nullptr;
    WriteComponent *m_writeConf = nullptr;
    PlayerConfComponent *m_playerConf = nullptr;
    MemPlayerConf m_memPlayerConf;
};

void confBullet(GeneralCollisionComponent *genColl, SegmentCollisionComponent *segmentColl,
                CollisionTag_e collTag, const pairFloat_t &point, float degreeAngle);
void setWeaponPlayer();
void confActionShape(MapCoordComponent *mapCompAction, const MapCoordComponent *playerMapComp,
                     const MoveableComponent *playerMoveComp, GeneralCollisionComponent *genCompAction);
