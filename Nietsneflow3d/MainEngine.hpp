#pragma once

#include <GraphicEngine.hpp>
#include <PhysicalEngine.hpp>
#include <AudioEngine.hpp>
#include <ECS/ECSManager.hpp>
#include <Level.hpp>
#include <ECS/Components/AudioComponent.hpp>

struct MemSpriteData;
struct WallData;
struct MoveableWallData;
struct AssociatedTriggerData;

using mapEnemySprite_t = std::map<EnemySpriteType_e, PairUI_t>;
using mapUiVectUI_t = std::map<uint32_t, std::vector<uint32_t>>;
using PairImpactData_t = std::pair<std::vector<MemSpriteData>, MemSpriteData>;
using MapImpactData_t = std::map<std::string, PairImpactData_t>;
using MapVisibleShotData_t = std::map<std::string, std::pair<std::string ,std::vector<MemSpriteData>>>;

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
    void loadLevel(const LevelManager &levelManager);
    bool mainLoop(bool &memGameOver);
    void playerAttack(uint32_t playerEntity, PlayerConfComponent *playerComp,
                      const PairFloat_t &point, float degreeAngle);
    void setUnsetPaused();
    inline bool isGamePaused()
    {
        return m_gamePaused;
    }
    void clearLevel();
    void confSystems();
    uint32_t createAmmoEntity(CollisionTag_e collTag, bool visibleShot);
    void setMenuEntries(PlayerConfComponent *playerComp);
    void updateWriteComp(WriteComponent *writeComp);
    void updateStringWriteEntitiesInputMenu();
    inline void validDisplayMenu()
    {
        m_graphicEngine.validDisplayMenu();
    }
    inline void updateMusicVolume(uint32_t volume)
    {
        m_audioEngine.updateMusicVolume(volume);
        m_graphicEngine.updateMusicVolumeBar(volume);
    }
    inline void updateEffectsVolume(uint32_t volume)
    {
        m_audioEngine.updateEffectsVolume(volume);
        m_graphicEngine.updateEffectsVolumeBar(volume);
    }
    inline uint32_t getMusicVolume()const
    {
        return m_audioEngine.getMusicVolume();
    }
    inline uint32_t getEffectsVolume()const
    {
        return m_audioEngine.getEffectsVolume();
    }
    inline void setCurrentResolution(uint32_t resolution)
    {
        m_graphicEngine.setCurrentMenuResolution(resolution);
    }
    inline uint32_t getCurrentResolutionNum()const
    {
        return m_graphicEngine.getCurrentResolutionNum();
    }
    inline uint32_t getCurrentDisplayedResolutionNum()const
    {
        return m_graphicEngine.getCurrentDisplayedResolutionNum();
    }

    inline uint32_t getMaxResolutionNum()const
    {
        return m_graphicEngine.getMaxResolutionNum();
    }
    inline void decreaseDisplayQuality()
    {
        m_graphicEngine.decreaseMenuDisplayQuality();
    }
    inline void increaseDisplayQuality()
    {
        m_graphicEngine.increaseMenuDisplayQuality();
    }
    inline void toogleMenuEntryFullscreen()
    {
        m_graphicEngine.toogleMenuEntryFullscreen();
    }
private:
    void clearObjectToDelete();
    void savePlayerGear();
    void loadPlayerGear();
    void displayTransitionMenu();
    void loadColorEntities();
    void confSoundMenuEntities(uint32_t musicEntity, uint32_t effectEntity);
    void confUnifiedColorEntity(uint32_t entityNum, const tupleFloat_t &color, bool transparent);
    void loadBackgroundEntities(const GroundCeilingData &groundData, const GroundCeilingData &backgroundData, const LevelManager &levelManager);
    void confColorBackgroundComponents(uint32_t entity, const GroundCeilingData &groundData, bool ground);
    void confGroundSimpleTextBackgroundComponents(uint32_t entity, const GroundCeilingData &groundData, const std::vector<SpriteData> &vectSprite);
    void confCeilingSimpleTextBackgroundComponents(uint32_t entity, const GroundCeilingData &groundData, const std::vector<SpriteData> &vectSprite);
    void confTiledTextBackgroundComponents(uint32_t entity, const GroundCeilingData &backgroundData, const std::vector<SpriteData> &vectSprite);
    void confWriteEntitiesDisplayMenu();
    void confWriteEntitiesInputMenu();
    void confMenuCursorEntity();
    void confWriteEntities();
    void linkSystemsToGraphicEngine();
    void linkSystemsToPhysicalEngine();
    void linkSystemsToSoundEngine();
    void loadShotImpactSprite(const std::vector<SpriteData> &vectSpriteData, const PairImpactData_t &shootDisplayData, uint32_t impactEntity);
    void confPlayerVisibleShotsSprite(const std::vector<SpriteData> &vectSpriteData, const MapVisibleShotData_t &shootDisplayData,
                                      WeaponComponent *weaponComp);
    void loadPlayerEntity(const LevelManager &levelManager, uint32_t numWeaponEntity, uint32_t numDisplayTeleportEntity);
    void confPlayerEntity(const LevelManager &levelManager, uint32_t entityNum, const Level &level, uint32_t numWeaponEntity,
                          uint32_t numDisplayTeleportEntity);
    void confActionEntity();
    void loadWallEntities(const std::map<std::string, MoveableWallData> &wallData,
                          const std::vector<SpriteData> &vectSprite);
    void confBaseWallData(uint32_t wallEntity, const SpriteData &memSpriteData, const PairUI_t &coordLevel,
                          const std::vector<uint8_t> &numWallSprites,
                          const std::vector<SpriteData> &vectSprite, TriggerBehaviourType_e triggerType, bool moveable = false);
    void loadDoorEntities(const LevelManager &levelManager);
    void loadEnemiesEntities(const LevelManager &levelManager);
    void loadTriggerEntityData(const MoveableWallData &moveWallData, const std::vector<uint32_t> &vectPosition, const std::vector<SpriteData> &vectSprite, TriggerWallMoveType_e type);
    void confVisibleAmmo(uint32_t ammoEntity);
    void loadStaticElementEntities(const LevelManager &levelManager);
    void loadBarrelElementEntities(const LevelManager &levelManager);
    SoundElement loadSound(const std::string &file);
    uint32_t loadDisplayTeleportEntity(const LevelManager &levelManager);
    void loadStaticElementGroup(const std::vector<SpriteData> &vectSpriteData, const std::map<std::string, StaticLevelElementData> &staticData,
                                LevelStaticElementType_e elementType, const std::string &soundFile = "");
    void loadExitElement(const LevelManager &levelManager, const StaticLevelElementData &exit);
    void createPlayerAmmoEntities(PlayerConfComponent *playerConf, CollisionTag_e collTag);
    void confAmmoEntities(std::vector<uint32_t> &ammoEntities, CollisionTag_e collTag,
                          bool visibleShot, uint32_t damage, float shotVelocity = 0,
                          std::optional<float> damageRay = std::nullopt);
    void createPlayerVisibleShotEntity(WeaponComponent *weaponConf);
    void createPlayerImpactEntities(const std::vector<SpriteData> &vectSpriteData, WeaponComponent *weaponConf,
                                    const MapImpactData_t &mapImpactData);
    uint32_t confShotImpactEntity(const std::vector<SpriteData> &vectSpriteData, const PairImpactData_t &shootDisplayData);
    uint32_t createTriggerEntity(bool visible);
    uint32_t createColorEntity();
    uint32_t createTextureEntity();
    uint32_t createEnemyDropObject(const LevelManager &levelManager, const EnemyData &enemyData, uint32_t iterationNum);
    uint32_t createStaticElementEntity(LevelStaticElementType_e elementType, const StaticLevelElementData &staticElementData,
                                       const std::vector<SpriteData> &vectSpriteData, uint32_t iterationNum, const std::string &soundFile = "");
    uint32_t confObjectEntity(const StaticLevelElementData &objectData);
    uint32_t confTeleportEntity(const StaticLevelElementData &teleportData, uint32_t iterationNum, const std::string &soundFile);
    uint32_t createMeleeAttackEntity(bool sound = false);
    uint32_t createDamageZoneEntity(uint32_t damage, CollisionTag_e tag, float ray = 10.0f, const std::string soundFile = "");
    uint32_t loadWeaponsEntity(const LevelManager &levelManager);
    uint32_t createBackgroundEntity(bool color);
    uint32_t createWeaponEntity();
    uint32_t createWallEntity(bool multiSprite, bool moveable = false);
    uint32_t createDoorEntity();
    uint32_t createEnemyEntity();
    uint32_t createShotEntity();
    uint32_t createShotImpactEntity();
    uint32_t createWriteEntity();
    uint32_t createVisibleShotEntity();
    uint32_t createSimpleSpriteEntity();
    uint32_t createStaticEntity();
    uint32_t createTeleportEntity();
    uint32_t createBarrelEntity();
    uint32_t createObjectEntity();
    uint32_t createDisplayTeleportEntity();
    void confBaseComponent(uint32_t entityNum, const SpriteData &memSpriteData, const std::optional<PairUI_t> &coordLevel,
                           CollisionShape_e collisionShape, CollisionTag_e tag);
    void confStaticComponent(uint32_t entityNum, const PairFloat_t &elementSize, LevelStaticElementType_e elementType);
    void loadEnemySprites(const std::vector<SpriteData> &vectSprite, const EnemyData &enemiesData,
                          uint32_t numEntity, EnemyConfComponent *enemyComp, const MapVisibleShotData_t &visibleShot);
    void loadVisibleShotData(const std::vector<SpriteData> &vectSprite, const std::vector<uint32_t> &visibleAmmo,
                             const std::string &visibleShootID, const MapVisibleShotData_t &visibleShot);
    void memTimerPausedValue();
    void applyTimerPausedValue();
    void confPlayerVisibleShoot(std::vector<uint32_t> &playerVisibleShots,
                                const PairFloat_t &point, float degreeAngle);
    void confPlayerBullet(PlayerConfComponent *playerComp,
                          const PairFloat_t &point, float degreeAngle, uint32_t numBullet);
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
    uint32_t m_memInputCursorPos;
    float m_fpsValue = 1.0f / 60.0f;
    GraphicEngine m_graphicEngine;
    PhysicalEngine m_physicalEngine;
    AudioEngine m_audioEngine;
    ECSManager m_ecsManager;
    std::vector<std::pair<uint32_t, time_t>> m_vectMemPausedTimer;
    bool m_gamePaused = false, m_playerMem = false;
    SpriteData const *m_memCursorSpriteData = nullptr, *m_memVisibleShotA = nullptr;
    GeneralCollisionComponent *m_exitColl = nullptr;
    WriteComponent *m_writeConf = nullptr;
    PlayerConfComponent *m_playerConf = nullptr;
    WeaponComponent *m_weaponComp;
    MemPlayerConf m_memPlayerConf;
    std::set<PairUI_t> m_memWall;
    std::map<PairUI_t, uint32_t> m_memTriggerCreated, m_memWallPos;
};

void insertEnemySpriteFromType(const std::vector<SpriteData> &vectSprite, mapEnemySprite_t &mapSpriteAssociate,
                               std::vector<SpriteData const *> &vectSpriteData, const std::vector<uint8_t> &enemyMemArray,
                               EnemySpriteType_e type);
void confBullet(GeneralCollisionComponent *genColl, SegmentCollisionComponent *segmentColl, MoveableComponent *moveImpactComp,
                CollisionTag_e collTag, const PairFloat_t &point, float degreeAngle);
void setWeaponPlayer();
void confActionShape(MapCoordComponent *mapCompAction, GeneralCollisionComponent *genCompAction, const MapCoordComponent *attackerMapComp,
                     const MoveableComponent *attackerMoveComp);
