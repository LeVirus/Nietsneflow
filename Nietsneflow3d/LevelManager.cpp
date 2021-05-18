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
void LevelManager::loadSpriteData(const INIReader &reader, const std::string &sectionName,
                                  bool font)
{
    std::vector<std::string> sections = reader.getSectionNamesContaining(sectionName);
    for(uint32_t i = 0; i < sections.size(); ++i)
    {
        uint32_t textureNum = reader.GetInteger(sections[i], "texture",
                                               std::numeric_limits<uint8_t>::max());
        assert(textureNum != std::numeric_limits<uint8_t>::max() && "Bad textureNumber");
        double texturePosX = reader.GetReal(sections[i], "texturePosX", 10.0);
        double texturePosY = reader.GetReal(sections[i], "texturePosY", 10.0);
        double textureWeight = reader.GetReal(sections[i], "textureWeight", 10.0);
        double textureHeight = reader.GetReal(sections[i], "textureHeight", 10.0);
        SpriteData spriteData
        {
            textureNum,
            {
                {
                    {texturePosX, texturePosY},
                    {texturePosX + textureWeight, texturePosY},
                    {texturePosX + textureWeight, texturePosY + textureHeight},
                    {texturePosX, texturePosY + textureHeight}
                }
            }
        };
        if(font)
        {
            m_fontData.addCharSpriteData(spriteData, sections[i]);
        }
        else
        {
            m_pictureData.setSpriteData(spriteData, sections[i]);
        }
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
            std::optional<uint8_t> id = m_pictureData.getIdentifier(reader.Get("Ground", "sprite", ""));
            assert(id && "Cannot get tegetIdentifierier.");
            arrayGAndCData[i].m_spriteNum = *id;
        }
        else
        {
            arrayGAndCData[i].m_tupleColor = {reader.GetReal("Ground", "colorR", -1.0),
                                              reader.GetReal("Ground", "colorG", -1.0),
                                              reader.GetReal("Ground", "colorB", -1.0)};
        }
        arrayGAndCData[i].m_apparence = displayType;
    }
    m_pictureData.setGroundAndCeilingData(arrayGAndCData);
}

//===================================================================
void LevelManager::loadLevelData(const INIReader &reader)
{
     m_level.setLevelSize({reader.GetInteger("Level", "weight", 10),
                           reader.GetInteger("Level", "height", 10)});
}

//===================================================================
void LevelManager::loadPlayerData(const INIReader &reader)
{
    m_level.setPlayerInitData({reader.GetInteger("PlayerInit", "playerDepartureX", 0),
                               reader.GetInteger("PlayerInit", "playerDepartureY", 0)},
                              static_cast<Direction_e>(reader.GetInteger("PlayerInit",
                                                                         "PlayerOrientation", 0)));
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
            reader.GetReal(sectionName, "SpriteWeightGame", 1.0);
    staticElement.m_inGameSpriteSize.second =
            reader.GetReal(sectionName, "SpriteHeightGame", 1.0);
    if(elementType == LevelStaticElementType_e::OBJECT)
    {
        staticElement.m_containing = reader.GetInteger(sectionName, "Containing", 1);
        uint32_t type = reader.GetInteger(sectionName, "Type", 1);
        assert(type < static_cast<uint32_t>(ObjectType_e::TOTAL));
        staticElement.m_type = static_cast<ObjectType_e>(type);
    }
    fillPositionVect(reader, sectionName, staticElement.m_TileGamePosition);
    if(elementType != LevelStaticElementType_e::OBJECT)
    {
        staticElement.m_traversable = reader.GetBoolean(sectionName, "traversable", true);
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
    for(uint32_t j = 0; j < results.size(); j += 2)
    {
        vectPos.emplace_back(pairUI_t{results[j], results[j + 1]});
    }
}

//===================================================================
uint8_t LevelManager::getSpriteId(const INIReader &reader,
                                  const std::string &sectionName)
{
    std::optional<uint8_t> id = m_pictureData.getIdentifier(reader.Get(sectionName, "Sprite", ""));
    assert(id && "picture data does not exists.");
    return *id;
}

//===================================================================
void LevelManager::loadWeaponsDisplayData(const INIReader &reader)
{
    std::vector<pairUIPairFloat_t> vectWeaponsDisplayData;
    std::vector<uint8_t> vectVisibleShotsData;
    std::vector<std::string> vectINISections;
    vectINISections = reader.getSectionNamesContaining("Weapons");
    assert(!vectINISections.empty());
    loadWeaponsData(reader, WeaponsType_e::GUN, vectINISections[0], vectWeaponsDisplayData);
    loadWeaponsData(reader, WeaponsType_e::SHOTGUN, vectINISections[0], vectWeaponsDisplayData);
    loadDisplayData(reader, vectINISections[0], "VisibleShot", vectVisibleShotsData);
    loadDisplayData(reader, vectINISections[0], "VisibleShotDestruct", vectVisibleShotsData);
    m_level.setWeaponsElement(vectWeaponsDisplayData, vectVisibleShotsData);
}

//===================================================================
void LevelManager::loadDisplayData(const INIReader &reader,
                                   std::string_view sectionName,
                                   std::string_view elementName,
                                   std::vector<uint8_t> &vectVisibleShotsData)
{
    std::string sprites = reader.Get(sectionName.data(), elementName.data(), "");
    std::istringstream iss(sprites);
    vectStr_t results(std::istream_iterator<std::string>{iss},
                      std::istream_iterator<std::string>());
    vectVisibleShotsData.reserve(vectVisibleShotsData.size() + results.size());
    for(uint32_t i = 0; i < results.size(); ++i)
    {
        vectVisibleShotsData.emplace_back(*(m_pictureData.getIdentifier(results[i])));
    }
}

//===================================================================
void LevelManager::loadWeaponsData(const INIReader &reader, WeaponsType_e weapon,
                                   std::string_view sectionName,
                                   std::vector<pairUIPairFloat_t> &vectWeaponsDisplayData)
{
    std::string weaponSpriteSection, spriteWeight, spriteHeight;
    switch (weapon)
    {
    case WeaponsType_e::GUN:
        weaponSpriteSection = "WeaponsGunSprite";
        spriteWeight = "SpriteGunWeightGame";
        spriteHeight = "SpriteGunHeightGame";
        break;
    case WeaponsType_e::SHOTGUN:
        weaponSpriteSection = "WeaponsShotgunSprite";
        spriteWeight = "SpriteGunWeightGame";
        spriteHeight = "SpriteGunHeightGame";
        break;
    case WeaponsType_e::TOTAL:
        assert(false);
        break;
    }
    std::string sprites = reader.Get(sectionName.data(), weaponSpriteSection, "");
    assert(!sprites.empty() && "Wall sprites cannot be loaded.");
    std::istringstream iss(sprites);
    vectStr_t results(std::istream_iterator<std::string>{iss},
                      std::istream_iterator<std::string>());
    std::string resultWeight = reader.Get(sectionName.data(), spriteWeight, ""),
            resultHeight = reader.Get(sectionName.data(), spriteHeight, "");

    std::vector<float> vectWeight = convertStrToVectFloat(resultWeight),
            vectHeight = convertStrToVectFloat(resultHeight);
    assert(vectHeight.size() == vectWeight.size());
    assert(results.size() == vectWeight.size());
    vectWeaponsDisplayData.reserve(results.size());
    for(uint32_t i = 0; i < results.size(); ++i)
    {
        vectWeaponsDisplayData.emplace_back(pairUIPairFloat_t{
                                                *(m_pictureData.getIdentifier(results[i])),
                                                {vectWeight[i], vectHeight[i]}
                                            });
    }
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
        std::string sprites = reader.Get(vectINISections[i], "Sprite", "");
        assert(!sprites.empty() && "Wall sprites cannot be loaded.");
        std::istringstream iss(sprites);
        vectStr_t results(std::istream_iterator<std::string>{iss},
                          std::istream_iterator<std::string>());
        vectWall.back().m_sprites.reserve(results.size());
        for(uint32_t i = 0; i < results.size(); ++i)
        {
            vectWall.back().m_sprites.emplace_back(*m_pictureData.getIdentifier(results[i]));
        }
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
        vectDoor.back().m_vertical = reader.GetBoolean(vectINISections[i], "Vertical", false);
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
                reader.GetReal(vectINISections[i], "SpriteWeightGame", 1.0);
        vectEnemy.back().m_inGameSpriteSize.second =
                reader.GetReal(vectINISections[i], "SpriteHeightGame", 1.0);
        loadEnemySprites(reader, vectINISections[i], EnemySpriteElementType_e::STATIC_FRONT,
                         vectEnemy.back());
        loadEnemySprites(reader, vectINISections[i], EnemySpriteElementType_e::STATIC_FRONT_LEFT,
                         vectEnemy.back());
        loadEnemySprites(reader, vectINISections[i], EnemySpriteElementType_e::STATIC_FRONT_RIGHT,
                         vectEnemy.back());
        loadEnemySprites(reader, vectINISections[i], EnemySpriteElementType_e::STATIC_BACK,
                         vectEnemy.back());
        loadEnemySprites(reader, vectINISections[i], EnemySpriteElementType_e::STATIC_BACK_LEFT,
                         vectEnemy.back());
        loadEnemySprites(reader, vectINISections[i], EnemySpriteElementType_e::STATIC_BACK_RIGHT,
                         vectEnemy.back());
        loadEnemySprites(reader, vectINISections[i], EnemySpriteElementType_e::STATIC_LEFT,
                         vectEnemy.back());
        loadEnemySprites(reader, vectINISections[i], EnemySpriteElementType_e::STATIC_RIGHT,
                         vectEnemy.back());

        loadEnemySprites(reader, vectINISections[i], EnemySpriteElementType_e::MOVE,
                         vectEnemy.back());
        loadEnemySprites(reader, vectINISections[i], EnemySpriteElementType_e::ATTACK,
                         vectEnemy.back());
        loadEnemySprites(reader, vectINISections[i], EnemySpriteElementType_e::DYING,
                         vectEnemy.back());
        loadEnemySprites(reader, vectINISections[i], EnemySpriteElementType_e::VISIBLE_SHOOT,
                         vectEnemy.back());
        loadEnemySprites(reader, vectINISections[i], EnemySpriteElementType_e::VISIBLE_SHOOT_DESTRUCT,
                         vectEnemy.back());
    }
    m_level.setEnemyElement(vectEnemy);
}

//===================================================================
void LevelManager::loadUtilsData(const INIReader &reader)
{
    std::vector<std::string> vectINISections;
    vectINISections = reader.getSectionNamesContaining("Utils");
    assert(!vectINISections.empty());
    m_spriteCursorName = reader.Get(vectINISections[0], "CursorSprite", "");
}

//===================================================================
void LevelManager::loadEnemySprites(const INIReader &reader, const std::string &sectionName,
                                    EnemySpriteElementType_e spriteTypeEnum, EnemyData &enemyData)
{
    std::vector<uint8_t> *vectPtr = nullptr;
    std::string spriteType;
    switch(spriteTypeEnum)
    {
    case EnemySpriteElementType_e::STATIC_FRONT:
        spriteType = "StaticSpriteFront";
        vectPtr = &enemyData.m_staticFrontSprites;
        break;
    case EnemySpriteElementType_e::STATIC_FRONT_LEFT:
        spriteType = "StaticSpriteFrontLeft";
        vectPtr = &enemyData.m_staticFrontLeftSprites;
        break;
    case EnemySpriteElementType_e::STATIC_FRONT_RIGHT:
        spriteType = "StaticSpriteFrontRight";
        vectPtr = &enemyData.m_staticFrontRightSprites;
        break;
    case EnemySpriteElementType_e::STATIC_BACK:
        spriteType = "StaticSpriteBack";
        vectPtr = &enemyData.m_staticBackSprites;
        break;
    case EnemySpriteElementType_e::STATIC_BACK_LEFT:
        spriteType = "StaticSpriteBackLeft";
        vectPtr = &enemyData.m_staticBackLeftSprites;
        break;
    case EnemySpriteElementType_e::STATIC_BACK_RIGHT:
        spriteType = "StaticSpriteBackRight";
        vectPtr = &enemyData.m_staticBackRightSprites;
        break;
    case EnemySpriteElementType_e::STATIC_LEFT:
        spriteType = "StaticSpriteLeft";
        vectPtr = &enemyData.m_staticLeftSprites;
        break;
    case EnemySpriteElementType_e::STATIC_RIGHT:
        spriteType = "StaticSpriteRight";
        vectPtr = &enemyData.m_staticRightSprites;
        break;
    case EnemySpriteElementType_e::ATTACK:
        spriteType = "AttackSprite";
        vectPtr = &enemyData.m_attackSprites;
        break;
    case EnemySpriteElementType_e::MOVE:
        spriteType = "MoveSprite";
        vectPtr = &enemyData.m_moveSprites;
        break;
    case EnemySpriteElementType_e::DYING:
        spriteType = "DyingSprite";
        vectPtr = &enemyData.m_dyingSprites;
        break;
    case EnemySpriteElementType_e::VISIBLE_SHOOT:
        spriteType = "VisibleShot";
        vectPtr = &enemyData.m_visibleShotSprites;
        break;
    case EnemySpriteElementType_e::VISIBLE_SHOOT_DESTRUCT:
        spriteType = "VisibleShotDestruct";
        vectPtr = &enemyData.m_visibleShotDestructSprites;
        break;

    }
    assert(vectPtr);
    std::string sprites = reader.Get(sectionName, spriteType, "");
    assert(!sprites.empty() && "Enemy sprites cannot be loaded.");
    std::istringstream iss(sprites);
    vectStr_t results(std::istream_iterator<std::string>{iss},
                      std::istream_iterator<std::string>());
    vectPtr->reserve(results.size());
    for(uint32_t i = 0; i < results.size(); ++i)
    {
        vectPtr->emplace_back(*m_pictureData.getIdentifier(results[i]));
    }
}

//======================================================
std::vector<uint32_t> convertStrToVectUI(const std::string &str)
{
    std::istringstream iss(str);
    return std::vector<uint32_t>(std::istream_iterator<uint32_t>{iss},
                      std::istream_iterator<uint32_t>());
}

//======================================================
std::vector<float> convertStrToVectFloat(const std::string &str)
{
    std::istringstream iss(str);
    return std::vector<float>(std::istream_iterator<float>{iss},
                      std::istream_iterator<float>());
}

//===================================================================
void LevelManager::loadTextureData(const std::string &INIFileName)
{
    INIReader reader(LEVEL_RESSOURCES_DIR_STR + INIFileName);
    if (reader.ParseError() < 0)
    {
        assert("Error while reading INI file.");
    }
    loadTexturePath(reader);
    loadSpriteData(reader);
    loadGroundAndCeilingData(reader);
    m_pictureData.setUpToDate();
}

//===================================================================
void LevelManager::loadFontData(const std::string &INIFileName)
{
    INIReader reader(LEVEL_RESSOURCES_DIR_STR + INIFileName);
    if (reader.ParseError() < 0)
    {
        assert("Error while reading INI file.");
    }
    loadSpriteData(reader, "", true);
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
    loadWeaponsDisplayData(reader);
    loadWallData(reader);
    loadDoorData(reader);
    loadEnemyData(reader);
    loadUtilsData(reader);
}

