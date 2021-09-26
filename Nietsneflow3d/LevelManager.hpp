#pragma once

#include <Level.hpp>
#include <PictureData.hpp>
#include <FontData.hpp>
#include <set>

class INIReader;
using SetStr_t = std::set<std::string>;

struct MemSpriteData
{
    uint8_t m_numSprite;
    PairFloat_t m_GLSize;
};

using MapVisibleShotData_t = std::map<std::string, std::vector<MemSpriteData>>;
using PairImpactData_t = std::pair<std::vector<MemSpriteData>, MemSpriteData>;
using MapImpactData_t = std::map<std::string, PairImpactData_t>;

struct WeaponINIData
{
    std::vector<MemSpriteData> m_spritesData;
    uint32_t m_maxAmmo, m_simultaneousShots, m_lastAnimNum, m_order, m_damage;
    AttackType_e m_attackType;
    AnimationMode_e m_animMode;
    std::string m_visibleShootID, m_impactID;
    float m_animationLatency, m_shotVelocity;
    bool m_startingPossess;
    std::optional<float> m_damageCircleRay;
    std::optional<uint32_t> m_startingAmmoCount;
};

struct WallData
{
    std::vector<uint8_t> m_sprites;
    std::set<PairUI_t> m_TileGamePosition;
};

struct AssociatedTriggerData
{
    PairUI_t m_pos;
    MemSpriteData m_displayData;
};

struct MoveableWallData
{
    std::vector<uint8_t> m_sprites;
    std::set<PairUI_t> m_TileGamePosition;
    std::vector<std::pair<Direction_e, uint32_t>> m_directionMove;
    TriggerWallMoveType_e m_triggerType;
    TriggerBehaviourType_e m_triggerBehaviourType;
    float m_velocity;
    std::optional<AssociatedTriggerData> m_associatedTriggerData;
    std::optional<PairUI_t> m_groundTriggerPos;
};

class LevelManager
{
public:
    LevelManager();
    void loadTextureData(const std::string &INIFileName);
    void loadStandardData(const std::string &INIFileName);
    void loadFontData(const std::string &INIFileName);
    void loadLevel(const std::string &INIFileName, uint32_t levelNum);
    void clearExistingPositionsElement();
    inline const PictureData &getPictureData()const {return m_pictureData;}
    inline const Level &getLevel()const {return m_level;}
    inline const FontData &getFontData()const {return m_fontData;}
    inline std::string getCursorSpriteName()const {return m_spriteCursorName;}
    inline const std::map<std::string, WallData> getWallData()const{return m_wallData;}
    inline const std::map<std::string, MoveableWallData> getMoveableWallData()const
    {
        return m_moveableWallData;
    }
    inline const std::map<std::string, StaticLevelElementData> &getGroundData()const
    {return m_groundElement;}
    inline const std::map<std::string, StaticLevelElementData> &getCeilingData()const
    {return m_ceilingElement;}
    inline const std::map<std::string, StaticLevelElementData> &getObjectData()const
    {return m_objectElement;}
    inline const std::map<std::string, StaticLevelElementData> &getTeleportData()const
    {
        return m_teleportElement;
    }
    inline const StaticLevelElementData &getExitElementData()const
    {
        return m_exitStaticElement;
    }
    inline const std::map<std::string, DoorData> &getDoorData()const
    {
        return m_doorData;
    }
    inline const std::map<std::string, EnemyData> &getEnemiesData()const
    {
        return m_enemyData;
    }
    inline const std::vector<SpriteData> &getPictureSpriteData()const
    {
        return m_pictureData.getSpriteData();
    }
    inline const std::vector<WeaponINIData> &getWeaponsData()const
    {
        return m_vectWeaponsINIData;
    }
    inline const MapImpactData_t &getImpactDisplayData()const
    {
        return m_impactINIData;
    }
    inline const MapVisibleShotData_t &getVisibleShootDisplayData()const
    {
        return m_visibleShootINIData;
    }
    inline const std::vector<MemSpriteData> &getVisibleTeleportData()const
    {
        return m_displayTeleportData;
    }
    inline const BarrelData &getBarrelData()const
    {
        return m_barrelElement;
    }
private:
    //texture and sprite loading
    void loadTexturePath(const INIReader &reader);
    void loadSpriteData(const INIReader &reader, const std::string &sectionName = "Sprite",
                        bool font = false);
    void loadBackgroundData(const INIReader &reader);
    void loadMusicData(const INIReader &reader);
    //Level element datas loading
    void loadLevelData(const INIReader &reader);
    void loadPositionPlayerData(const INIReader &reader);
    void loadGeneralStaticElements(const INIReader &reader,
                                   LevelStaticElementType_e elementType);
    void loadPositionStaticElements(const INIReader &reader);
    void loadBarrelElements(const INIReader &reader);
    void readStandardStaticElement(const INIReader &reader, StaticLevelElementData &staticElement,
                                   const std::string &sectionName,
                                   LevelStaticElementType_e elementType);
    void fillStandartPositionVect(const INIReader &reader, const std::string &sectionName,
                                  VectPairUI_t &vectPos);
    void fillTeleportPositions(const INIReader &reader, const std::string &sectionName);
    std::optional<PairUI_t> getPosition(const INIReader &reader, const std::string_view sectionName, const std::string_view propertyName);
    bool fillWallPositionVect(const INIReader &reader, const std::string &sectionName, const std::string &propertyName,
                              std::set<PairUI_t> &setPos);
    void removeWallPositionVect(const INIReader &reader, const std::string &sectionName,
                                std::set<PairUI_t> &vectPos);
    uint8_t getSpriteId(const INIReader &reader, const std::string &sectionName);
    void loadVisibleShotDisplayData(const INIReader &reader);
    void loadShotImpactDisplayData(const INIReader &reader);
    void loadWeaponsData(const INIReader &reader);
    void loadBarrelsData(const INIReader &reader);
    void loadExit(const INIReader &reader);
    void loadVisualTeleportData(const INIReader &reader);
    void loadTriggerElements(const INIReader &reader);
    void loadSpriteData(const INIReader &reader, const std::string &sectionName,
                        StaticLevelElementData &staticElement);
    void loadDisplayData(const INIReader &reader,
                         std::string_view sectionName, std::string_view subSectionName);
    void loadWeaponData(const INIReader &reader,
                        std::string_view sectionName, uint32_t numIt);
    void loadWallData(const INIReader &reader);
    void loadPositionWall(const INIReader &reader);
    void loadTriggerLevelData(const INIReader &reader, const std::string &sectionName);
    void loadDoorData(const INIReader &reader);
    void loadPositionDoorData(const INIReader &reader);
    void loadEnemyData(const INIReader &reader);
    void loadPositionEnemyData(const INIReader &reader);
    void loadUtilsData(const INIReader &reader);
    void loadEnemySprites(const INIReader &reader, const std::string &sectionName,
                          EnemySpriteElementType_e spriteTypeEnum, EnemyData &enemyData);
    void deleteWall(const PairUI_t &coord);
    void loadPositionExit(const INIReader &reader);
    std::vector<uint8_t> getVectSpriteNum(const INIReader &reader, const std::string_view section, const std::string_view param);
    std::vector<PairFloat_t> getVectSpriteGLSize(const INIReader &reader, const std::string_view section, const std::string_view weightParam,
                                                  const std::string_view heightParam);
private:
    PictureData m_pictureData;
    FontData m_fontData;
    Level m_level;
    std::string m_spriteCursorName;
    StaticLevelElementData m_exitStaticElement;
    std::map<std::string, WallData> m_wallData;
    std::map<std::string, MoveableWallData> m_moveableWallData;
    std::map<std::string, uint32_t> m_weaponINIAssociated, m_cardINIAssociated;
    std::map<std::string, StaticLevelElementData> m_groundElement, m_ceilingElement, m_objectElement, m_teleportElement;
    BarrelData m_barrelElement;
    std::map<std::string, DoorData> m_doorData;
    std::map<std::string, EnemyData> m_enemyData;
    std::map<std::string, MemSpriteData> m_triggerDisplayData;
    std::vector<MemSpriteData> m_displayTeleportData;
    //store the sprite number and the screen display size
    std::vector<WeaponINIData> m_vectWeaponsINIData;
    //first moving Shot sprite, all other destruct phase sprites
    MapVisibleShotData_t m_visibleShootINIData;
    MapImpactData_t m_impactINIData;
};

VectPairUI_t getPositionData(const INIReader &reader, const std::string & sectionName, const std::string &propertyName);
std::vector<uint32_t> convertStrToVectUI(const std::string &str);
std::optional<std::vector<uint32_t> > getBrutPositionData(const INIReader &reader, const std::string & sectionName, const std::string &propertyName);
std::vector<float> convertStrToVectFloat(const std::string &str);
std::vector<bool> convertStrToVectBool(const std::string &str);
std::vector<std::string> convertStrToVectStr(const std::string &str);
void fillPositionVerticalLine(const PairUI_t &origins, uint32_t size,
                              std::set<PairUI_t> &vectPos);
void fillPositionHorizontalLine(const PairUI_t &origins, uint32_t size,
                                std::set<PairUI_t> &vectPos);
void fillPositionRectangle(const PairUI_t &origins, const PairUI_t &size,
                           std::set<PairUI_t> &vectPos);
void fillPositionDiagLineUpLeft(const PairUI_t &origins, uint32_t size,
                                std::set<PairUI_t> &vectPos);
void fillPositionDiagLineDownLeft(const PairUI_t &origins, uint32_t size,
                                  std::set<PairUI_t> &vectPos);
void fillPositionDiagRectangle(const PairUI_t &origins, uint32_t size,
                               std::set<PairUI_t> &vectPos);
