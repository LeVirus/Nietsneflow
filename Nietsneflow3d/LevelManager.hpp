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
    void loadFontData(const std::string &INIFileName);
    void loadLevel(const std::string &INIFileName, uint32_t levelNum);
    inline const PictureData &getPictureData()const {return m_pictureData;}
    inline const Level &getLevel()const {return m_level;}
    inline const FontData &getFontData()const {return m_fontData;}
    inline std::string getCursorSpriteName()const {return m_spriteCursorName;}
private:
    //texture and sprite loading
    void loadTexturePath(const INIReader &reader);
    void loadSpriteData(const INIReader &reader, const std::string &sectionName = "Sprite",
                        bool font = false);
    void loadGroundAndCeilingData(const INIReader &reader);

    //Level element datas loading
    void loadLevelData(const INIReader &reader);
    void loadPlayerData(const INIReader &reader);
    void loadGeneralStaticElements(const INIReader &reader,
                                  LevelStaticElementType_e elementType);
    void readStaticElement(const INIReader &reader, StaticLevelElementData &staticElement,
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
    void loadDoorData(const INIReader &reader);
    void loadEnemyData(const INIReader &reader);
    void loadUtilsData(const INIReader &reader);

    void loadEnemySprites(const INIReader &reader, const std::string &sectionName,
                          EnemySpriteElementType_e spriteTypeEnum, EnemyData &enemyData);
private:
    PictureData m_pictureData;
    FontData m_fontData;
    Level m_level;
    std::string m_spriteCursorName;
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
