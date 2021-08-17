#pragma once

#include <GraphicEngine.hpp>
#include <PhysicalEngine.hpp>
#include <ECS/ECSManager.hpp>
#include <Level.hpp>

struct WeaponSpriteData;

using mapVisibleData_t = std::map<std::string, std::vector<WeaponSpriteData>>;
using mapEnemySprite_t = std::map<EnemySpriteType_e, pairUI_t>;
using mapUiVectUI_t = std::map<uint32_t, std::vector<uint32_t>>;
class LevelManager;
class Level;
class FontData;
struct EnemyData;
struct EnemyConfComponent;
struct WeaponComponent;

struct MemPlayerConf
{
    uint32_t m_currentWeapon, m_previousWeapon;
    std::vector<uint32_t> m_ammunationsCount;
    std::vector<bool> m_weapons;
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
    void playerAttack(uint32_t playerEntity, PlayerConfComponent *playerComp,
                      const pairFloat_t &point, float degreeAngle);
    void setUnsetPaused();
    inline bool isGamePaused()
    {
        return m_gamePaused;
    }
    inline void clearLevel()
    {
        m_ecsManager.getEngine().RmAllEntity();
        m_graphicEngine.clearSystems();
    }
    void confSystems();
private:
    void clearObjectToDelete();
    void savePlayerGear();
    void loadPlayerGear();
    void displayTransitionMenu();
    void loadDamageEntity();
    void loadTransitionEntity();
    void confUnifiedColorEntity(uint32_t entityNum, const tupleFloat_t &color);
    uint32_t createColorEntity();
    uint32_t createTextureEntity();
    void loadGroundAndCeilingEntities(const GroundCeilingData &groundData,
                                      const GroundCeilingData &ceilingData, const LevelManager &levelManager);
    void confCeilingComponents(uint32_t entityNum, const GroundCeilingData &ceilingData,
                               const std::vector<SpriteData> &vectSprite);
    void confGroundComponents(uint32_t entityNum, const GroundCeilingData &groundData,
                              const std::vector<SpriteData> &vectSprite);
    void confMenuCursorEntity();
    void confWriteEntities();
    void linkSystemsToGraphicEngine();
    void linkSystemsToPhysicalEngine();
    void loadShotImpactSprite(const std::vector<SpriteData> &vectSpriteData,
                              const std::vector<WeaponSpriteData> &shootDisplayData,
                              uint32_t impactEntity);
    void confPlayerVisibleShotsSprite(const std::vector<SpriteData> &vectSpriteData,
                                      const mapVisibleData_t &shootDisplayData,
                                      WeaponComponent *weaponComp);
    void loadPlayerEntity(const LevelManager &levelManager, uint32_t numWeaponEntity);
    void confPlayerEntity(const LevelManager &levelManager,
                          uint32_t entityNum, const Level &level,
                          uint32_t numWeaponEntity);
    void confActionEntity();
    void confFistHitEntity(WeaponComponent *weaponConf);
    uint32_t loadWeaponsEntity(const LevelManager &levelManager);
    uint32_t createBackgroundEntity(GroundCeilingData const *data);
    void loadWallEntities(const LevelManager &levelManager);
    void loadDoorEntities(const LevelManager &levelManager);
    void loadEnemiesEntities(const LevelManager &levelManager);
    void confVisibleAmmo(uint32_t ammoEntity);
    void loadStaticElementEntities(const LevelManager &levelManager);
    void loadStaticElementGroup(const LevelManager &levelManager,
                                const std::map<std::string, StaticLevelElementData> &staticData, LevelStaticElementType_e elementType);
    void loadExitElement(const LevelManager &levelManager, const StaticLevelElementData &exit);
    void createPlayerAmmoEntities(PlayerConfComponent *playerConf, CollisionTag_e collTag);
    void confAmmoEntities(ArrayVisibleShot_t &ammoEntities, CollisionTag_e collTag,
                          bool visibleShot, uint32_t damage, float shotVelocity = 0);
    void createAmmoEntity(uint32_t &ammoNum, CollisionTag_e collTag, bool visibleShot);
    void createPlayerVisibleShotEntity(WeaponComponent *weaponConf);
    void createPlayerImpactEntities(const std::vector<SpriteData> &vectSpriteData,
                                  WeaponComponent *weaponConf,
                                  const mapVisibleData_t &mapImpactData);
    void confShotImpactEntity(const std::vector<SpriteData> &vectSpriteData,
                                  const std::vector<WeaponSpriteData> &shootDisplayData,
                                  uint32_t &impactEntity);
    uint32_t createWeaponEntity();
    uint32_t createWallEntity(bool multiSprite);
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
                          const EnemyData &enemiesData, uint32_t numEntity, EnemyConfComponent *enemyComp, const mapVisibleData_t &visibleShot);
    void loadVisibleShotData(const std::vector<SpriteData> &vectSprite, const ArrayVisibleShot_t &visibleAmmo,
                             const std::string &visibleShootID, const mapVisibleData_t &visibleShot);
    void memTimerPausedValue();
    void applyTimerPausedValue();
    void confPlayerVisibleShoot(const ArrayVisibleShot_t &playerVisibleShots, const pairFloat_t &point, float degreeAngle);
    void confPlayerBullet(PlayerConfComponent *playerComp,
                          const pairFloat_t &point, float degreeAngle, uint32_t numBullet);
    inline void memColorSystemEntity(uint32_t entity)
    {
        m_graphicEngine.memColorSystemEntity(entity);
    }
    inline void memCeilingBackgroundFPSSystemEntity(uint32_t entity, bool simpleTexture)
    {
        m_graphicEngine.memCeilingBackgroundFPSSystemEntity(entity, simpleTexture);
    }
    inline void memGroundBackgroundFPSSystemEntity(uint32_t entity, bool simpleTexture)
    {
        m_graphicEngine.memGroundBackgroundFPSSystemEntity(entity, simpleTexture);
    }
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
    WeaponComponent *m_weaponComp;
    MemPlayerConf m_memPlayerConf;
};

void insertEnemySpriteFromType(const std::vector<SpriteData> &vectSprite,
                               mapEnemySprite_t &mapSpriteAssociate,
                               std::vector<SpriteData const *> &vectSpriteData,
                               const std::vector<uint8_t> &enemyMemArray, EnemySpriteType_e type);
void confBullet(GeneralCollisionComponent *genColl, SegmentCollisionComponent *segmentColl,
                CollisionTag_e collTag, const pairFloat_t &point, float degreeAngle);
void setWeaponPlayer();
void confActionShape(MapCoordComponent *mapCompAction, const MapCoordComponent *playerMapComp,
                     const MoveableComponent *playerMoveComp, GeneralCollisionComponent *genCompAction);
