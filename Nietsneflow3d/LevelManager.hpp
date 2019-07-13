#ifndef LEVELMANAGER_H
#define LEVELMANAGER_H

#include <Level.hpp>
#include <PictureData.hpp>
#include <set>

class INIReader;

using setStr = std::set<std::string>;

class LevelManager
{
private:
    PictureData m_pictureData;
    Level m_level;
private:
    //texture and sprite loading
    void loadTexturePath(const INIReader &reader);
    void loadSpriteData(const INIReader &reader);
    void loadGroundAndCeilingData(const INIReader &reader);

    //Level element datas loading
    void loadLevelData(const INIReader &reader);
    void loadPlayerData(const INIReader &reader);
    void loadGeneralStaticElements(const INIReader &reader,
                                  LevelStaticElementType_e elementType);
    void readStaticElement(const INIReader &reader, StaticLevelElementData &staticElement, const std::__cxx11::string &sectionName,
                                  LevelStaticElementType_e elementType);
    void fillPositionVect(const INIReader &reader, const std::__cxx11::string &sectionName, vectPairUI &vectPos);
    uint8_t getSpriteId(const INIReader &reader, const std::string &sectionName);
    void loadWallData(const INIReader &reader);
    void loadDoorData(const INIReader &reader);
    void loadEnemyData(const INIReader &reader);

    void loadEnemySprites(const INIReader &reader, const std::string &sectionName,
                          EnemySpriteType_e spriteType, EnemyData &enemyData);
    std::vector<uint32_t> convertStrToVectUI(const std::string &str);
public:
    LevelManager();
    void loadTextureData(const std::string &INIFileName);
    void loadLevel(const std::string &INIFileName);
};

#endif // LEVELMANAGER_H
