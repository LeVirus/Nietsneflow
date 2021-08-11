#pragma once

#include <Level.hpp>
#include <PictureData.hpp>
#include <FontData.hpp>
#include <set>

class INIReader;
using SetStr_t = std::set<std::string>;

struct WeaponSpriteData
{
    uint8_t m_numSprite;
    pairFloat_t m_GLSize;
};

using mapVisibleShotData_t = std::map<std::string, std::vector<WeaponSpriteData>>;

struct WeaponINIData
{
    std::vector<WeaponSpriteData> m_spritesData;
    uint32_t m_maxAmmo, m_simultaneousShots, m_lastAnimNum;
    AttackType_e m_attackType;
    AnimationMode_e m_animMode;
    std::string m_visibleShootID;
    float m_animationLatency;
};

struct ShootDisplayData
{
    std::vector<WeaponSpriteData> m_impact;
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
    inline const std::map<std::string, StaticLevelElementData> &getGroundData()const
    {return m_groundElement;}
    inline const std::map<std::string, StaticLevelElementData> &getCeilingData()const
    {return m_ceilingElement;}
    inline const std::map<std::string, StaticLevelElementData> &getObjectData()const
    {return m_objectElement;}
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
    inline const std::vector<ShootDisplayData> &getShootDisplayData()const
    {
        return m_vectShootDisplayData;
    }
    inline const mapVisibleShotData_t &getVisibleShootDisplayData()const
    {
        return m_visibleShootINIData;
    }
private:
    //texture and sprite loading
    void loadTexturePath(const INIReader &reader);
    void loadSpriteData(const INIReader &reader, const std::string &sectionName = "Sprite",
                        bool font = false);
    void loadBackgroundData(const INIReader &reader);

    //Level element datas loading
    void loadLevelData(const INIReader &reader);
    void loadPositionPlayerData(const INIReader &reader);
    void loadGeneralStaticElements(const INIReader &reader,
                                   LevelStaticElementType_e elementType);
    void loadPositionStaticElements(const INIReader &reader);
    void readStandardStaticElement(const INIReader &reader, StaticLevelElementData &staticElement,
                                   const std::string &sectionName,
                                   LevelStaticElementType_e elementType);
    void fillStandartPositionVect(const INIReader &reader, const std::string &sectionName,
                                  vectPairUI_t &vectPos);
    bool fillWallPositionVect(const INIReader &reader, const std::string &sectionName, const std::string &propertyName,
                              std::set<pairUI_t> &vectPos);
    void removeWallPositionVect(const INIReader &reader, const std::string &sectionName,
                                std::set<pairUI_t> &vectPos);
    uint8_t getSpriteId(const INIReader &reader, const std::string &sectionName);
    void loadVisibleShootDisplayData(const INIReader &reader);
    void loadWeaponsDisplayData(const INIReader &reader);
    void loadExit(const INIReader &reader);
    void loadSpriteData(const INIReader &reader, const std::string &sectionName,
                        StaticLevelElementData &staticElement);
    void loadDisplayData(const INIReader &reader,
                         std::string_view sectionName, std::string_view subSectionName, uint32_t numIt);
    void loadWeaponData(const INIReader &reader,
                         std::string_view sectionName, uint32_t numIt);
    void loadWallData(const INIReader &reader);
    void loadPositionWall(const INIReader &reader);
    void loadDoorData(const INIReader &reader);
    void loadPositionDoorData(const INIReader &reader);
    void loadEnemyData(const INIReader &reader);
    void loadPositionEnemyData(const INIReader &reader);
    void loadUtilsData(const INIReader &reader);
    void loadEnemySprites(const INIReader &reader, const std::string &sectionName,
                          EnemySpriteElementType_e spriteTypeEnum, EnemyData &enemyData);
    void deleteWall(const pairUI_t &coord);
    void loadPositionExit(const INIReader &reader);
private:
    PictureData m_pictureData;
    FontData m_fontData;
    Level m_level;
    std::string m_spriteCursorName;
    StaticLevelElementData m_exitStaticElement;
    std::map<std::string, WallData> m_wallData;
    std::map<std::string, StaticLevelElementData> m_groundElement, m_ceilingElement, m_objectElement;
    std::map<std::string, DoorData> m_doorData;
    std::map<std::string, EnemyData> m_enemyData;
    //store the sprite number and the screen display size
    std::vector<WeaponINIData> m_vectWeaponsINIData;
    std::vector<ShootDisplayData> m_vectShootDisplayData;
    //first moving Shot sprite, all other destruct phase sprites
    mapVisibleShotData_t m_visibleShootINIData;
};

std::vector<uint32_t> convertStrToVectUI(const std::string &str);
std::optional<std::vector<uint32_t> > getBrutPositionData(const INIReader &reader, const std::string & sectionName, const std::string &propertyName);
std::vector<float> convertStrToVectFloat(const std::string &str);
std::vector<std::string> convertStrToVectStr(const std::string &str);
void fillPositionVerticalLine(const pairUI_t &origins, uint32_t size,
                              std::set<pairUI_t> &vectPos);
void fillPositionHorizontalLine(const pairUI_t &origins, uint32_t size,
                                std::set<pairUI_t> &vectPos);
void fillPositionRectangle(const pairUI_t &origins, const pairUI_t &size,
                           std::set<pairUI_t> &vectPos);
void fillPositionDiagLineUpLeft(const pairUI_t &origins, uint32_t size,
                                std::set<pairUI_t> &vectPos);
void fillPositionDiagLineDownLeft(const pairUI_t &origins, uint32_t size,
                                  std::set<pairUI_t> &vectPos);
void fillPositionDiagRectangle(const pairUI_t &origins, uint32_t size,
                               std::set<pairUI_t> &vectPos);
