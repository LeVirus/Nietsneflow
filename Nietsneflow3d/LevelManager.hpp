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
    void fillPositionVect(const INIReader &reader, const std::string &sectionName,
                          vectPairUI_t &vectPos);
    uint8_t getSpriteId(const INIReader &reader, const std::string &sectionName);
    void loadWeaponsDisplayData(const INIReader &reader);
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
std::vector<float> convertStrToVectFloat(const std::string &str);
