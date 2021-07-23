#pragma once

#include <Level.hpp>
#include <PictureData.hpp>
#include <FontData.hpp>
#include <set>

class INIReader;

using setStr = std::set<std::string>;

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
    void loadWeaponsDisplayData(const INIReader &reader);
    void loadExit(const INIReader &reader);
    void loadSpriteData(const INIReader &reader, const std::string &sectionName,
                        StaticLevelElementData &staticElement);
    void loadDisplayData(const INIReader &reader,
                         std::string_view sectionName,
                         std::string_view elementName,
                         std::vector<uint8_t> &vectVisibleShotsData);
    void loadWeaponsData(const INIReader &reader, WeaponsType_e weapon,
                         std::string_view sectionName,
                         std::vector<pairUIPairFloat_t> &vectWeaponsDisplayData);
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
};

std::vector<uint32_t> convertStrToVectUI(const std::string &str);
std::optional<std::vector<uint32_t> > getBrutPositionData(const INIReader &reader, const std::string & sectionName, const std::string &propertyName);
std::vector<float> convertStrToVectFloat(const std::string &str);
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
