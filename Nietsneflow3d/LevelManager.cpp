#include <LevelManager.hpp>
#include <inireader.h>
#include <cassert>
#include <sstream>
#include <iterator>

//===================================================================
LevelManager::LevelManager()
{

}

//===================================================================
void LevelManager::loadTexturePath(const INIReader &reader)
{
    std::string textures = reader.Get("PathToTexture", "textures", "");
    assert(!textures.empty() && "Textures path cannot be loaded.");
    std::istringstream iss(textures);
    vectStr_t results(std::istream_iterator<std::string>{iss},
                      std::istream_iterator<std::string>());
    m_pictureData.setTexturePath(results);
}

//===================================================================
void LevelManager::loadSpriteData(const INIReader &reader)
{
    const setStr &sections = reader.Sections();
    setStr::const_reverse_iterator revIt = sections.rbegin();
    while(revIt != sections.rend() && revIt->find("Sprite") != std::string::npos)
    {
        uint8_t textureNum = reader.GetInteger(*revIt, "texture", std::numeric_limits<char>::epsilon());
        assert(std::numeric_limits<char>::epsilon() != -1 && "Bad textureNumber");
        float texturePosX = reader.GetReal(*revIt, "texturePosX", 1.0f);
        float texturePosY = reader.GetReal(*revIt, "texturePosY", 1.0f);
        float textureWeight = reader.GetReal(*revIt, "textureWeight", 1.0f);
        float textureHeight = reader.GetReal(*revIt, "textureHeight", 1.0f);
        SpriteData spriteData{textureNum, {texturePosX, texturePosY},
                              {textureWeight, textureHeight}};
        m_pictureData.setSpriteData(spriteData, *revIt);
        ++revIt;
    }
}

//===================================================================
void LevelManager::loadGroundAndCeilingData(const INIReader &reader)
{
    std::array<GroundCeilingData, 2> arrayGAndCData;
    for(uint32_t i = 0; i < 2 ; ++i)
    {
        DisplayType_e displayType = reader.GetBoolean("Ground", "displayType", true) ?
                    DisplayType_e::COLOR : DisplayType_e::TEXTURE;
        if(displayType == DisplayType_e::TEXTURE)
        {
            std::string sprite = reader.Get("Ground", "sprite", "");
            uint8_t id = m_pictureData.getIdentifier(sprite);
            assert(id != std::numeric_limits<char>::epsilon() && "Cannot get texture identifier.");
            arrayGAndCData[i].m_spriteNum = id;
        }
        else
        {
            float colorR = reader.GetReal("Ground", "colorR", -1.0f);
            float colorG = reader.GetReal("Ground", "colorG", -1.0f);
            float colorB = reader.GetReal("Ground", "colorB", -1.0f);
            tupleFloat_t tupleFloat = std::make_tuple(colorR, colorG, colorB);
            arrayGAndCData[i].m_tupleColor = tupleFloat;
        }
        arrayGAndCData[i].m_apparence = displayType;
    }
    m_pictureData.setGroundAndCeilingData(arrayGAndCData);
}

//===================================================================
void LevelManager::loadLevelData(const INIReader &reader)
{
     std::pair<uint32_t, uint32_t> levelSize;
     levelSize.first = reader.GetInteger("Level", "weight", 0);
     levelSize.second = reader.GetInteger("Level", "height", 0);
     m_level.setLevelSize(levelSize);
}

//===================================================================
void LevelManager::loadPlayerData(const INIReader &reader)
{
    std::pair<uint32_t, uint32_t> playerOriginPosition;
    Direction_e playerOriginDirection;
    playerOriginPosition.first = reader.GetInteger("PlayerInit", "playerDepartureX", 0);
    playerOriginPosition.second = reader.GetInteger("PlayerInit", "playerDepartureY", 0);
    playerOriginDirection = (Direction_e)reader.GetInteger("PlayerInit", "PlayerOrientation", 0);
    m_level.setPlayerInitData(playerOriginPosition, playerOriginDirection);
}

//===================================================================
void LevelManager::loadGeneralStaticElement(const INIReader &reader,
                                            LevelStaticElementType_e elementType)
{
    std::vector<std::string> vectINISections;
    switch(elementType)
    {
        case LevelStaticElementType_e::GROUND:
        vectINISections = reader.getAllSectionNamesContaining("Ground");
        break;
        case LevelStaticElementType_e::CEILING:
        vectINISections = reader.getAllSectionNamesContaining("Ceiling");
        break;
        case LevelStaticElementType_e::OBJECT:
        vectINISections = reader.getAllSectionNamesContaining("Object");
        break;
    }

}

//===================================================================
void LevelManager::loadWallData(const INIReader &reader)
{

}

//===================================================================
void LevelManager::loadDoorData(const INIReader &reader)
{

}

//===================================================================
void LevelManager::loadEnemyData(const INIReader &reader)
{

}

//===================================================================
void LevelManager::loadTextureData(const std::string &INIFileName)
{
    INIReader reader(std::string("../Nietsneflow3d/Ressources/Level1/") + INIFileName);
    if (reader.ParseError() < 0)
    {
        assert("Error while reading INI file.");
    }
    loadTexturePath(reader);
    loadSpriteData(reader);
    loadGroundAndCeilingData(reader);
    m_pictureData.display();//debug
}

//===================================================================
void LevelManager::loadLevel(const std::string &INIFileName)
{
    INIReader reader(std::string("../Nietsneflow3d/Ressources/Level1/") + INIFileName);
    if (reader.ParseError() < 0)
    {
        assert("Error while reading INI file.");
    }
    loadLevelData(reader);
    loadPlayerData(reader);
}

