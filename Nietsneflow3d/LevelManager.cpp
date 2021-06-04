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
void LevelManager::loadPositionPlayerData(const INIReader &reader)
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
    std::map<std::string, StaticLevelElementData> *memMap;
    std::vector<std::string> vectINISections;
    switch(elementType)
    {
        case LevelStaticElementType_e::GROUND:
        vectINISections = reader.getSectionNamesContaining("Ground");
        memMap = &m_groundElement;
        break;
        case LevelStaticElementType_e::CEILING:
        vectINISections = reader.getSectionNamesContaining("Ceiling");
        memMap = &m_ceilingElement;
        break;
        case LevelStaticElementType_e::OBJECT:
        vectINISections = reader.getSectionNamesContaining("Object");
        memMap = &m_objectElement;
        break;
    }
    for(uint32_t i = 0; i < vectINISections.size(); ++i)
    {
        memMap->insert({vectINISections[i], StaticLevelElementData()});
        readStandardStaticElement(reader, memMap->operator[](vectINISections[i]),
                vectINISections[i], elementType);
    }
}

//===================================================================
void LevelManager::loadExit(const INIReader &reader)
{
    std::vector<std::string> vectINISections;
    vectINISections = reader.getSectionNamesContaining("Exit");
    assert(!vectINISections.empty());
    loadSpriteData(reader, vectINISections[0], m_exitStaticElement);
}

//===================================================================
void LevelManager::loadPositionExit(const INIReader &reader)
{
    std::vector<std::string> vectINISections;
    vectINISections = reader.getSectionNamesContaining("Exit");
    assert(!vectINISections.empty());
    std::string gamePositions = reader.Get(vectINISections[0], "GamePosition", "");
    assert(!gamePositions.empty() && "Error while getting positions.");
    std::vector<uint32_t> results = convertStrToVectUI(gamePositions);
    for(uint32_t i = 0; i < results.size(); i += 2)
    {
        std::cerr << "Exit " << results[i] << " " << results[i + 1] << "\n";
        m_exitStaticElement.m_TileGamePosition.push_back({results[i], results[i + 1]});
        deleteWall(m_exitStaticElement.m_TileGamePosition.back());
    }
}

//===================================================================
void LevelManager::loadSpriteData(const INIReader &reader, const std::string &sectionName,
                                  StaticLevelElementData &staticElement)
{
    staticElement.m_numSprite = getSpriteId(reader, sectionName);
    staticElement.m_inGameSpriteSize.first =
            reader.GetReal(sectionName, "SpriteWeightGame", 1.0);
    staticElement.m_inGameSpriteSize.second =
            reader.GetReal(sectionName, "SpriteHeightGame", 1.0);
}

//===================================================================
void LevelManager::readStandardStaticElement(const INIReader &reader, StaticLevelElementData &staticElement,
                                             const std::string & sectionName, LevelStaticElementType_e elementType)
{
    loadSpriteData(reader, sectionName, staticElement);
    if(elementType == LevelStaticElementType_e::OBJECT)
    {
        staticElement.m_containing = reader.GetInteger(sectionName, "Containing", 1);
        uint32_t type = reader.GetInteger(sectionName, "Type", 1);
        assert(type < static_cast<uint32_t>(ObjectType_e::TOTAL));
        staticElement.m_type = static_cast<ObjectType_e>(type);
    }
    if(elementType != LevelStaticElementType_e::OBJECT)
    {
        staticElement.m_traversable = reader.GetBoolean(sectionName, "traversable", true);
    }
}

//===================================================================
std::optional<std::vector<uint32_t>> getBrutPositionData(const INIReader &reader,
                                                         const std::string &sectionName,
                                                         const std::string &propertyName)
{
    std::string gamePositions = reader.Get(sectionName, propertyName, "");
    if(gamePositions.empty())
    {
        return {};
    }
    return convertStrToVectUI(gamePositions);
}

//===================================================================
void LevelManager::fillStandartPositionVect(const INIReader &reader,
                                            const std::string &sectionName,
                                            vectPairUI_t &vectPos)
{
    std::optional<std::vector<uint32_t>> results = getBrutPositionData(reader, sectionName, "GamePosition");
    if(!results)
    {
        return;
    }
    assert(!(*results).empty() && "Error inconsistent position datas.");
    size_t finalSize = (*results).size() / 2;
    assert(!((*results).size() % 2) && "Error inconsistent position datas.");
    vectPos.reserve(finalSize);
    for(uint32_t j = 0; j < (*results).size(); j += 2)
    {
        vectPos.emplace_back(pairUI_t{(*results)[j], (*results)[j + 1]});
        deleteWall(vectPos.back());
    }
}

//===================================================================
void LevelManager::deleteWall(const pairUI_t &coord)
{
    std::set<pairUI_t>::iterator itt;
    std::map<std::string, WallData>::iterator it = m_wallData.begin();
    for(; it != m_wallData.end(); ++it)
    {
        itt = it->second.m_TileGamePosition.find(coord);
        if(itt != it->second.m_TileGamePosition.end())
        {
            it->second.m_TileGamePosition.erase(*itt);
        }
    }
}

//===================================================================
bool LevelManager::fillWallPositionVect(const INIReader &reader,
                                        const std::string &sectionName,
                                        const std::string &propertyName,
                                        std::set<pairUI_t> &vectPos)
{
    std::optional<std::vector<uint32_t>> results = getBrutPositionData(reader, sectionName, propertyName);

    if(!results || (*results).empty())
    {
        return false;
    }
    pairUI_t origins;
    uint32_t j = 0;
    while(j < (*results).size())
    {
        assert((*results)[j] < static_cast<uint32_t>(WallShapeINI_e::TOTAL));
        assert((*results).size() > (j + 2));
        origins = {(*results)[j + 1], (*results)[j + 2]};
        switch(static_cast<WallShapeINI_e>((*results)[j]))
        {
        case WallShapeINI_e::RECTANGLE:
            assert((*results).size() > (j + 4));
            fillPositionRectangle(origins, {(*results)[j + 3], (*results)[j + 4]}, vectPos);
            j += 5;
            break;
        case WallShapeINI_e::VERT_LINE:
            assert((*results).size() > (j + 3));
            fillPositionVerticalLine(origins, (*results)[j + 3], vectPos);
            j += 4;
            break;
        case WallShapeINI_e::HORIZ_LINE:
            assert((*results).size() > (j + 3));
            fillPositionHorizontalLine(origins, (*results)[j + 3], vectPos);
            j += 4;
            break;
        case WallShapeINI_e::POINT:
            vectPos.insert(origins);
            j += 3;
            break;
        case WallShapeINI_e::DIAG_RECT:
            assert((*results).size() > (j + 3));
            fillPositionDiagRectangle(origins, (*results)[j + 3], vectPos);
            j += 4;
            break;
        case WallShapeINI_e::DIAG_UP_LEFT:
            assert((*results).size() > (j + 3));
            fillPositionDiagLineUpLeft(origins, (*results)[j + 3], vectPos);
            j += 4;
            break;
        case WallShapeINI_e::DIAG_DOWN_LEFT:
            assert((*results).size() > (j + 3));
            fillPositionDiagLineDownLeft(origins, (*results)[j + 3], vectPos);
            j += 4;
            break;
        default:
            assert(false);
            break;
        }
    }
    return true;
}

//===================================================================
void LevelManager::removeWallPositionVect(const INIReader &reader,
                                          const std::string &sectionName,
                                          std::set<pairUI_t> &vectPos)
{
    std::set<pairUI_t> wallToRemove;
    if(!fillWallPositionVect(reader, sectionName, "RemovePosition", wallToRemove))
    {
        return;
    }
    std::set<pairUI_t>::iterator itt;
    for(std::set<pairUI_t>::const_iterator it = wallToRemove.begin(); it != wallToRemove.end(); ++it)
    {
        itt = vectPos.find(*it);
        if(itt != vectPos.end())
        {
            vectPos.erase(itt);
        }
    }
}


//===================================================================
void fillPositionVerticalLine(const pairUI_t &origins, uint32_t size,
                              std::set<pairUI_t> &vectPos)
{
    pairUI_t current = origins;
    for(uint32_t j = 0; j < size; ++j, ++current.second)
    {
        vectPos.insert(current);
    }
}

//===================================================================
void fillPositionHorizontalLine(const pairUI_t &origins, uint32_t size,
                                std::set<pairUI_t> &vectPos)
{
    pairUI_t current = origins;
    for(uint32_t j = 0; j < size; ++j, ++current.first)
    {
        vectPos.insert(current);
    }
}

//===================================================================
void fillPositionRectangle(const pairUI_t &origins, const pairUI_t &size,
                           std::set<pairUI_t> &vectPos)
{
    fillPositionHorizontalLine(origins, size.first, vectPos);
    fillPositionHorizontalLine({origins.first, origins.second + size.second - 1},
                               size.first, vectPos);
    fillPositionVerticalLine({origins.first, origins.second + 1},
                             size.second - 1, vectPos);
    fillPositionVerticalLine({origins.first + size.first - 1, origins.second + 1},
                             size.second - 1, vectPos);
}

//===================================================================
void fillPositionDiagLineUpLeft(const pairUI_t &origins, uint32_t size,
                                std::set<pairUI_t> &vectPos)
{
    pairUI_t current = origins;
    for(uint32_t j = 0; j < size; ++j, ++current.first, ++current.second)
    {
        vectPos.insert(current);
    }
}

//===================================================================
void fillPositionDiagLineDownLeft(const pairUI_t &origins, uint32_t size,
                                  std::set<pairUI_t> &vectPos)
{
    pairUI_t current = origins;
    for(uint32_t j = 0; j < size; ++j, ++current.first, --current.second)
    {
        vectPos.insert(current);
        if(current.second == 0)
        {
            break;
        }
    }
}

//===================================================================
void fillPositionDiagRectangle(const pairUI_t &origins, uint32_t size,
                               std::set<pairUI_t> &vectPos)
{
    if(size % 2 == 0)
    {
        ++size;
    }
    uint32_t halfSizeA = size / 2;
    uint32_t halfSizeB = halfSizeA + size % 2;
    fillPositionDiagLineUpLeft({origins.first, origins.second + halfSizeA}, halfSizeB,
                               vectPos);
    fillPositionDiagLineDownLeft({origins.first, origins.second + halfSizeA}, halfSizeB,
                                 vectPos);
    fillPositionDiagLineUpLeft({origins.first + halfSizeA, origins.second}, halfSizeB,
                               vectPos);
    fillPositionDiagLineDownLeft({origins.first + halfSizeA, origins.second + size - 1},
                                 halfSizeB, vectPos);
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
    loadWeaponsData(reader, WeaponsType_e::AXE, vectINISections[0], vectWeaponsDisplayData);
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
    case WeaponsType_e::AXE:
        weaponSpriteSection = "WeaponsAxeSprite";
        spriteWeight = "SpriteAxeWeightGame";
        spriteHeight = "SpriteAxeHeightGame";
        break;
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
    std::vector<std::string> vectINISections;
    vectINISections = reader.getSectionNamesContaining("Wall");
    for(uint32_t i = 0; i < vectINISections.size(); ++i)
    {
        m_wallData.insert({vectINISections[i], WallData()});
        std::string sprites = reader.Get(vectINISections[i], "Sprite", "");
        assert(!sprites.empty() && "Wall sprites cannot be loaded.");
        std::istringstream iss(sprites);
        vectStr_t results(std::istream_iterator<std::string>{iss},
                          std::istream_iterator<std::string>());
        m_wallData[vectINISections[i]].m_sprites.reserve(results.size());
        //load sprites
        for(uint32_t j = 0; j < results.size(); ++j)
        {
            m_wallData[vectINISections[i]].m_sprites.emplace_back(*m_pictureData.getIdentifier(results[j]));
        }
    }
}

//===================================================================
void LevelManager::loadPositionWall(const INIReader &reader)
{
    std::vector<std::string> vectINISections = reader.getSectionNamesContaining("Wall");
    std::map<std::string, WallData>::iterator it;
    for(uint32_t i = 0; i < vectINISections.size(); ++i)
    {
        it = m_wallData.find(vectINISections[i]);
        assert(it != m_wallData.end());
        if(!fillWallPositionVect(reader, vectINISections[i], "GamePosition", it->second.m_TileGamePosition))
        {
            assert(false);
        }
        removeWallPositionVect(reader, vectINISections[i], it->second.m_TileGamePosition);
    }
}

//===================================================================
void LevelManager::loadDoorData(const INIReader &reader)
{
    std::vector<std::string> vectINISections;
    vectINISections = reader.getSectionNamesContaining("Door");
    for(uint32_t i = 0; i < vectINISections.size(); ++i)
    {
        m_doorData.insert({vectINISections[i], DoorData()});
        m_doorData[vectINISections[i]].m_numSprite = getSpriteId(reader, vectINISections[i]);
        m_doorData[vectINISections[i]].m_vertical = reader.GetBoolean(vectINISections[i], "Vertical", false);
    }
}

//===================================================================
void LevelManager::loadPositionDoorData(const INIReader &reader)
{
    std::vector<std::string> vectINISections;
    vectINISections = reader.getSectionNamesContaining("Door");
    std::map<std::string, DoorData>::iterator it;
    for(uint32_t i = 0; i < vectINISections.size(); ++i)
    {
        it = m_doorData.find(vectINISections[i]);
        assert(it != m_doorData.end());
        fillStandartPositionVect(reader, vectINISections[i], it->second.m_TileGamePosition);
    }
}

//===================================================================
void LevelManager::loadEnemyData(const INIReader &reader)
{
    std::vector<std::string> vectINISections;
    vectINISections = reader.getSectionNamesContaining("Enemy");
    for(uint32_t i = 0; i < vectINISections.size(); ++i)
    {
        m_enemyData.insert({vectINISections[i], EnemyData()});
        m_enemyData[vectINISections[i]].m_traversable = reader.GetBoolean(vectINISections[i],
                                                       "traversable", false);
        m_enemyData[vectINISections[i]].m_inGameSpriteSize.first =
                reader.GetReal(vectINISections[i], "SpriteWeightGame", 1.0);
        m_enemyData[vectINISections[i]].m_inGameSpriteSize.second =
                reader.GetReal(vectINISections[i], "SpriteHeightGame", 1.0);
        loadEnemySprites(reader, vectINISections[i], EnemySpriteElementType_e::STATIC_FRONT,
                         m_enemyData[vectINISections[i]]);
        loadEnemySprites(reader, vectINISections[i], EnemySpriteElementType_e::STATIC_FRONT_LEFT,
                         m_enemyData[vectINISections[i]]);
        loadEnemySprites(reader, vectINISections[i], EnemySpriteElementType_e::STATIC_FRONT_RIGHT,
                         m_enemyData[vectINISections[i]]);
        loadEnemySprites(reader, vectINISections[i], EnemySpriteElementType_e::STATIC_BACK,
                         m_enemyData[vectINISections[i]]);
        loadEnemySprites(reader, vectINISections[i], EnemySpriteElementType_e::STATIC_BACK_LEFT,
                         m_enemyData[vectINISections[i]]);
        loadEnemySprites(reader, vectINISections[i], EnemySpriteElementType_e::STATIC_BACK_RIGHT,
                         m_enemyData[vectINISections[i]]);
        loadEnemySprites(reader, vectINISections[i], EnemySpriteElementType_e::STATIC_LEFT,
                         m_enemyData[vectINISections[i]]);
        loadEnemySprites(reader, vectINISections[i], EnemySpriteElementType_e::STATIC_RIGHT,
                         m_enemyData[vectINISections[i]]);

        loadEnemySprites(reader, vectINISections[i], EnemySpriteElementType_e::MOVE,
                         m_enemyData[vectINISections[i]]);
        loadEnemySprites(reader, vectINISections[i], EnemySpriteElementType_e::ATTACK,
                         m_enemyData[vectINISections[i]]);
        loadEnemySprites(reader, vectINISections[i], EnemySpriteElementType_e::DYING,
                         m_enemyData[vectINISections[i]]);
        loadEnemySprites(reader, vectINISections[i], EnemySpriteElementType_e::VISIBLE_SHOOT,
                         m_enemyData[vectINISections[i]]);
        loadEnemySprites(reader, vectINISections[i], EnemySpriteElementType_e::VISIBLE_SHOOT_DESTRUCT,
                         m_enemyData[vectINISections[i]]);
    }
}

//===================================================================
void LevelManager::loadPositionEnemyData(const INIReader &reader)
{
    std::vector<std::string> vectINISections;
    vectINISections = reader.getSectionNamesContaining("Enemy");
    std::map<std::string, EnemyData>::iterator it;
    for(uint32_t i = 0; i < vectINISections.size(); ++i)
    {
        it = m_enemyData.find(vectINISections[i]);
        assert(it != m_enemyData.end());
        fillStandartPositionVect(reader, vectINISections[i], it->second.m_TileGamePosition);
    }
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
void LevelManager::loadStandardData(const std::string &INIFileName)
{
    INIReader reader(LEVEL_RESSOURCES_DIR_STR + INIFileName);
    if (reader.ParseError() < 0)
    {
        assert("Error while reading INI file.");
    }
    loadLevelData(reader);
    loadWallData(reader);
    loadGeneralStaticElements(reader, LevelStaticElementType_e::GROUND);
    loadGeneralStaticElements(reader, LevelStaticElementType_e::CEILING);
    loadGeneralStaticElements(reader, LevelStaticElementType_e::OBJECT);
    loadExit(reader);
    loadDoorData(reader);
    loadEnemyData(reader);
    loadUtilsData(reader);
    loadWeaponsDisplayData(reader);
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
void LevelManager::loadPositionStaticElements(const INIReader &reader)
{
    std::map<std::string, StaticLevelElementData>::iterator it = m_groundElement.begin();
    for(; it != m_groundElement.end(); ++it)
    {
        fillStandartPositionVect(reader, it->first, it->second.m_TileGamePosition);
    }
    it = m_ceilingElement.begin();
    for(; it != m_ceilingElement.end(); ++it)
    {
        fillStandartPositionVect(reader, it->first, it->second.m_TileGamePosition);
    }
    it = m_objectElement.begin();
    for(; it != m_objectElement.end(); ++it)
    {
        fillStandartPositionVect(reader, it->first, it->second.m_TileGamePosition);
    }
}

//===================================================================
void LevelManager::loadLevel(const std::string &INIFileName, uint32_t levelNum)
{
    INIReader reader(LEVEL_RESSOURCES_DIR_STR + std::string("Level") +
                     std::to_string(levelNum) + std::string ("/") + INIFileName);
    if(reader.ParseError() < 0)
    {
        assert("Error while reading INI file.");
    }
    loadPositionPlayerData(reader);
    loadPositionWall(reader);
    loadPositionStaticElements(reader);
    loadPositionExit(reader);
    loadPositionDoorData(reader);
    loadPositionEnemyData(reader);
}
