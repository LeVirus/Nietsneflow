#include <LevelManager.hpp>
#include <inireader.h>
#include <cassert>
#include <sstream>
#include <iostream>
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
    std::vector<std::string> sections = reader.getSectionNamesContaining("Sprite");
    for(uint32_t i = 0; i < sections.size(); ++i)
    {
        uint8_t textureNum = reader.GetInteger(sections[i], "texture",
                                               std::numeric_limits<char>::epsilon());
        assert(std::numeric_limits<char>::epsilon() != -1 && "Bad textureNumber");
        float texturePosX = reader.GetReal(sections[i], "texturePosX", 1.0f);
        float texturePosY = reader.GetReal(sections[i], "texturePosY", 1.0f);
        float textureWeight = reader.GetReal(sections[i], "textureWeight", 1.0f);
        float textureHeight = reader.GetReal(sections[i], "textureHeight", 1.0f);
        SpriteData spriteData{textureNum, {texturePosX, texturePosY},
                              {textureWeight, textureHeight}};
        m_pictureData.setSpriteData(spriteData, sections[i]);
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
            std::cout << " ddd"<<std::endl<<std::endl<<std::endl<<std::endl;
            uint8_t id = m_pictureData.getIdentifier(sprite);
            std::cerr << (uint16_t)id << " ddd"<<std::endl<<std::endl<<std::endl<<std::endl;
            assert(id != std::numeric_limits<uint8_t>::epsilon() && "Cannot get texture identifier.");
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
     pairUI_t levelSize;
     levelSize.first = reader.GetInteger("Level", "weight", 0);
     levelSize.second = reader.GetInteger("Level", "height", 0);
     m_level.setLevelSize(levelSize);
}

//===================================================================
void LevelManager::loadPlayerData(const INIReader &reader)
{
    pairUI_t playerOriginPosition;
    Direction_e playerOriginDirection;
    playerOriginPosition.first = reader.GetInteger("PlayerInit", "playerDepartureX", 0);
    playerOriginPosition.second = reader.GetInteger("PlayerInit", "playerDepartureY", 0);
    playerOriginDirection = (Direction_e)reader.GetInteger("PlayerInit", "PlayerOrientation", 0);
    m_level.setPlayerInitData(playerOriginPosition, playerOriginDirection);
}

//===================================================================
void LevelManager::loadGeneralStaticElements(const INIReader &reader,
                                             LevelStaticElementType_e elementType)
{
    std::vector<std::string> vectINISections;
    switch(elementType)
    {
        case LevelStaticElementType_e::GROUND:
        vectINISections = reader.getSectionNamesContaining("Ground");
        break;
        case LevelStaticElementType_e::CEILING:
        vectINISections = reader.getSectionNamesContaining("Ceiling");
        break;
        case LevelStaticElementType_e::OBJECT:
        vectINISections = reader.getSectionNamesContaining("Object");
        break;
    }
    std::vector<StaticLevelElementData> vectStaticElement;
    vectStaticElement.reserve(vectINISections.size());

    for(uint32_t i = 0; i < vectINISections.size(); ++i)
    {
        vectStaticElement.emplace_back(StaticLevelElementData());
        readStaticElement(reader, vectStaticElement.back(), vectINISections[i], elementType);
    }
    switch(elementType)
    {
        case LevelStaticElementType_e::GROUND:
        m_level.setGroundElement(vectStaticElement);
        break;
        case LevelStaticElementType_e::CEILING:
        m_level.setCeilingElement(vectStaticElement);
        break;
        case LevelStaticElementType_e::OBJECT:
        m_level.setObjectElement(vectStaticElement);
        break;
    }
}

//===================================================================
void LevelManager::readStaticElement(const INIReader &reader, StaticLevelElementData &staticElement,
                                     const std::string & sectionName, LevelStaticElementType_e elementType)
{
    staticElement.m_numSprite = getSpriteId(reader, sectionName);
    staticElement.m_inGameSpriteSize.first =
            reader.GetReal(sectionName, "SpriteWeightGame", 1.0f);
    staticElement.m_inGameSpriteSize.second =
            reader.GetReal(sectionName, "SpriteHeightGame", 1.0f);

    fillPositionVect(reader, sectionName, staticElement.m_TileGamePosition);
    if(elementType == LevelStaticElementType_e::GROUND)
    {
        staticElement.m_traversable = reader.GetBoolean(sectionName,
                                                        "traversable", true);
    }
}

//===================================================================
void LevelManager::fillPositionVect(const INIReader &reader,
                                    const std::string & sectionName,
                                    vectPairUI_t &vectPos)
{
    std::string gamePositions = reader.Get(sectionName, "GamePosition", "");
    assert(!gamePositions.empty() && "Error while getting positions.");
    std::vector<uint32_t> results = convertStrToVectUI(gamePositions);
    size_t finalSize = results.size() / 2;
    assert(!results.empty() && "Error inconsistent position datas.");
    assert(!(results.size() % 2) && "Error inconsistent position datas.");

    vectPos.reserve(finalSize);
    for(uint32_t j = 0; j < results.size(); ++j)
    {
        vectPos.emplace_back(pairUI_t{results[j], results[++j]});
    }
}

//===================================================================
uint8_t LevelManager::getSpriteId(const INIReader &reader,
                                  const std::string &sectionName)
{
    std::string sprite = reader.Get(sectionName, "Sprite", "");
    uint8_t id = m_pictureData.getIdentifier(sprite);
    assert(id != std::numeric_limits<uint8_t>::max() && "picture data does not exists.");
    return id;
}

//===================================================================
void LevelManager::loadWallData(const INIReader &reader)
{
    std::vector<WallData> vectWall;
    std::vector<std::string> vectINISections;
    vectINISections = reader.getSectionNamesContaining("Wall");
    vectWall.reserve(vectINISections.size());
    for(uint32_t i = 0; i < vectINISections.size(); ++i)
    {
        vectWall.emplace_back(WallData());
        vectWall.back().m_numSprite = getSpriteId(reader, vectINISections[i]);
        fillPositionVect(reader, vectINISections[i], vectWall.back().m_TileGamePosition);
    }
    m_level.setWallElement(vectWall);
}

//===================================================================
void LevelManager::loadDoorData(const INIReader &reader)
{
    std::vector<DoorData> vectDoor;
    std::vector<std::string> vectINISections;
    vectINISections = reader.getSectionNamesContaining("Door");
    vectDoor.reserve(vectINISections.size());
    for(uint32_t i = 0; i < vectINISections.size(); ++i)
    {
        vectDoor.emplace_back(DoorData());
        vectDoor.back().m_numSprite = getSpriteId(reader, vectINISections[i]);
        fillPositionVect(reader, vectINISections[i], vectDoor.back().m_TileGamePosition);
        vectDoor.back().m_vertical = reader.GetBoolean(vectINISections[i],
                                                       "Vertical", false);
    }
    m_level.setDoorElement(vectDoor);
}

//===================================================================
void LevelManager::loadEnemyData(const INIReader &reader)
{
    std::vector<EnemyData> vectEnemy;
    std::vector<std::string> vectINISections;
    vectINISections = reader.getSectionNamesContaining("Enemy");
    vectEnemy.reserve(vectINISections.size());
    for(uint32_t i = 0; i < vectINISections.size(); ++i)
    {
        vectEnemy.emplace_back(EnemyData());
        fillPositionVect(reader, vectINISections[i], vectEnemy.back().m_TileGamePosition);
        vectEnemy.back().m_traversable = reader.GetBoolean(vectINISections[i],
                                                       "traversable", false);
        vectEnemy.back().m_inGameSpriteSize.first =
                reader.GetReal(vectINISections[i], "SpriteWeightGame", 1.0f);
        vectEnemy.back().m_inGameSpriteSize.second =
                reader.GetReal(vectINISections[i], "SpriteHeightGame", 1.0f);
        loadEnemySprites(reader, vectINISections[i], EnemySpriteType_e::STATIC,
                         vectEnemy.back());
        loadEnemySprites(reader, vectINISections[i], EnemySpriteType_e::MOVE,
                         vectEnemy.back());
        loadEnemySprites(reader, vectINISections[i], EnemySpriteType_e::ATTACK,
                         vectEnemy.back());
        loadEnemySprites(reader, vectINISections[i], EnemySpriteType_e::DYING,
                         vectEnemy.back());
    }
    m_level.setEnemyElement(vectEnemy);
}

//===================================================================
void LevelManager::loadEnemySprites(const INIReader &reader, const std::string &sectionName,
                              EnemySpriteType_e spriteTypeEnum, EnemyData &enemyData)
{
    std::vector<uint8_t> *vectPtr;
    std::string spriteType;
    switch(spriteTypeEnum)
    {
    case EnemySpriteType_e::STATIC:
        spriteType = "StaticSprite";
        vectPtr = &enemyData.m_staticSprites;
        break;
    case EnemySpriteType_e::ATTACK:
        spriteType = "AttackSprite";
        vectPtr = &enemyData.m_attackSprites;
        break;
    case EnemySpriteType_e::MOVE:
        spriteType = "MoveSprite";
        vectPtr = &enemyData.m_moveSprites;
        break;
    case EnemySpriteType_e::DYING:
        spriteType = "DyingSprite";
        vectPtr = &enemyData.m_dyingSprites;
        break;
    }

    std::string sprites = reader.Get(sectionName, spriteType, "");
    assert(!sprites.empty() && "Enemy sprites cannot be loaded.");
    std::istringstream iss(sprites);
    vectStr_t results(std::istream_iterator<std::string>{iss},
                      std::istream_iterator<std::string>());
    vectPtr->reserve(results.size());
    for(uint32_t i = 0; i < results.size(); ++i)
    {
        vectPtr->emplace_back(m_pictureData.getIdentifier(results[i]));
    }
}

//===================================================================
std::vector<uint32_t> LevelManager::convertStrToVectUI(
        const std::string &str)
{
    std::istringstream iss(str);
    return std::vector<uint32_t>(std::istream_iterator<uint32_t>{iss},
                      std::istream_iterator<uint32_t>());
}

//===================================================================
void LevelManager::loadTextureData(const std::string &INIFileName, uint32_t levelNum)
{
    INIReader reader(LEVEL_RESSOURCES_DIR_STR + std::string("Level") +
                     std::to_string(levelNum) + std::string ("/") + INIFileName);
    if (reader.ParseError() < 0)
    {
        assert("Error while reading INI file.");
    }
    loadTexturePath(reader);
    loadSpriteData(reader);
    loadGroundAndCeilingData(reader);
    m_pictureData.setUpToDate();
    m_pictureData.display();//debug
}

//===================================================================
void LevelManager::loadLevel(const std::string &INIFileName, uint32_t levelNum)
{
    INIReader reader(LEVEL_RESSOURCES_DIR_STR + std::string("Level") +
                     std::to_string(levelNum) + std::string ("/") + INIFileName);
    if (reader.ParseError() < 0)
    {
        assert("Error while reading INI file.");
    }
    loadLevelData(reader);
    loadPlayerData(reader);
    loadGeneralStaticElements(reader, LevelStaticElementType_e::GROUND);
    loadGeneralStaticElements(reader, LevelStaticElementType_e::CEILING);
    loadGeneralStaticElements(reader, LevelStaticElementType_e::OBJECT);
    loadWallData(reader);
    loadDoorData(reader);
    loadEnemyData(reader);
    m_level.display();
}

