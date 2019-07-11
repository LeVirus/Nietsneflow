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
    Level m_level;
    PictureData m_pictureData;
private:
    void loadTexturePath(const INIReader &reader);
    void loadSpriteData(const INIReader &reader);
    void loadGroundAndCeilingData(const INIReader &reader);
public:
    LevelManager();
    void loadTextureData(const std::string &INIFileName);
    void loadLevel(const std::string &INIFileName);
};

#endif // LEVELMANAGER_H
