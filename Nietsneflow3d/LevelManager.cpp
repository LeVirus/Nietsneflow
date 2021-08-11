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
void LevelManager::loadBackgroundData(const INIReader &reader)
{
    std::array<GroundCeilingData, 2> arrayGAndCData;
    //first ground
    std::string current = "Ground";
    for(uint32_t i = 0; i < 2 ; ++i)
    {
        arrayGAndCData[i].m_apparence =
                static_cast<DisplayType_e>(reader.GetInteger(current, "displayType", 0));
        if(arrayGAndCData[i].m_apparence == DisplayType_e::COLOR)
        {
            std::vector<float> colorR, colorG, colorB;

            colorR = convertStrToVectFloat(reader.Get(current, "colorR", ""));
            colorG = convertStrToVectFloat(reader.Get(current, "colorG", ""));
            colorB = convertStrToVectFloat(reader.Get(current, "colorB", ""));
            for(uint32_t j = 0; j < 4 ; ++j)
            {
                arrayGAndCData[i].m_color[j] = tupleFloat_t{colorR[j], colorG[j], colorB[j]};
            }
        }
        else
        {
            std::optional<uint8_t> id = m_pictureData.getIdentifier(
                        reader.Get(current, "sprite", ""));
            assert(id && "Cannot get tegetIdentifierier.");
            arrayGAndCData[i].m_spriteNum = *id;
        }
        //second ceiling
        current = "Ceiling";
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
    case LevelStaticElementType_e::IMPACT:
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
uint8_t LevelManager::getSpriteId(const INIReader &reader, const std::string &sectionName)
{
    std::optional<uint8_t> id = m_pictureData.getIdentifier(reader.Get(sectionName, "Sprite", ""));
    assert(id && "picture data does not exists.");
    return *id;
}

//===================================================================
void LevelManager::loadVisibleShootDisplayData(const INIReader &reader)
{
    std::vector<std::string> vectINISections = reader.getSectionNamesContaining("VisibleShoot");;
    if(vectINISections.empty())
    {
        return;
    }
    std::string sprites, weight, height;
    std::vector<float> vectWeight, vectHeight;
    vectStr_t vectSprites;
    for(uint32_t i = 0; i < vectINISections.size(); ++i)
    {
        sprites = reader.Get(vectINISections[i], "ShotSprites", "");
        assert(!sprites.empty());
        weight = reader.Get(vectINISections[i], "SpriteWeightGame", "");
        assert(!weight.empty());
        height = reader.Get(vectINISections[i], "SpriteHeightGame", "");
        assert(!height.empty());
        vectSprites = convertStrToVectStr(sprites);
        vectWeight = convertStrToVectFloat(weight);
        vectHeight = convertStrToVectFloat(height);
        assert(vectWeight.size() == vectHeight.size());
        assert(vectSprites.size() == vectHeight.size());
        m_visibleShootINIData.insert({vectINISections[i], std::vector<WeaponSpriteData>()});
        for(uint32_t j = 0; j < vectSprites.size(); ++j)
        {
            m_visibleShootINIData[vectINISections[i]].emplace_back(
                        WeaponSpriteData{*m_pictureData.getIdentifier(vectSprites[j]), {vectWeight[j], vectHeight[j]}});
        }
    }
}

//===================================================================
void LevelManager::loadWeaponsDisplayData(const INIReader &reader)
{
    std::vector<std::string> vectINISections;
    vectINISections = reader.getSectionNamesContaining("Weapon");
    assert(!vectINISections.empty());
    m_vectWeaponsINIData.resize(vectINISections.size());
    for(uint32_t i = 0; i < vectINISections.size(); ++i)
    {
        m_vectShootDisplayData.emplace_back(ShootDisplayData());
        loadWeaponData(reader, vectINISections[i], i);
        loadDisplayData(reader, vectINISections[i], "ShotImpact", i);
    }
}

//===================================================================
void LevelManager::loadDisplayData(const INIReader &reader, std::string_view sectionName,
                                   std::string_view subSectionName, uint32_t numIt)
{
    std::string sprites = reader.Get(sectionName.data(), subSectionName.data(), "");
    if(sprites.empty())
    {
        return;
    }
    vectStr_t vectsprite = convertStrToVectStr(sprites);
    std::vector<WeaponSpriteData> *vect = nullptr;
    pairFloat_t GLSize{0.5f, 0.5f};
    if(subSectionName == "ShotImpact")
    {
        GLSize = {0.1f, 0.1f};
        m_vectShootDisplayData[numIt].m_impact.reserve(vectsprite.size());
        std::optional<uint8_t> spriteNum;
        for(uint32_t i = 0; i < vectsprite.size(); ++i)
        {
            spriteNum = m_pictureData.getIdentifier(vectsprite[i]);
            assert(spriteNum);
            m_vectShootDisplayData[numIt].m_impact.
                    emplace_back(WeaponSpriteData{*spriteNum, GLSize});
        }
        return;
    }
    assert(vect);
    (*vect).reserve(vectsprite.size());
    for(uint32_t i = 0; i < vectsprite.size(); ++i)
    {
        (*vect).emplace_back(WeaponSpriteData{(*(m_pictureData.getIdentifier(vectsprite[i]))), GLSize});
    }
}

//===================================================================
void LevelManager::loadWeaponData(const INIReader &reader, std::string_view sectionName, uint32_t numIt)
{
    std::string sprites, latency;
    m_vectWeaponsINIData[numIt].m_maxAmmo =
            std::stoul(reader.Get(sectionName.data(), "MaxAmmo", "1"));
    assert(m_vectWeaponsINIData[numIt].m_maxAmmo != 1);
    m_vectWeaponsINIData[numIt].m_simultaneousShots =
            std::stoul(reader.Get(sectionName.data(), "NumberOfShots", "1"));
    m_vectWeaponsINIData[numIt].m_animMode = static_cast<AnimationMode_e>(std::stoul(reader.Get(sectionName.data(), "AnimationType", "0")));
    m_vectWeaponsINIData[numIt].m_lastAnimNum = std::stoul(reader.Get(sectionName.data(), "LastAnimNum", "1"));
    m_vectWeaponsINIData[numIt].m_simultaneousShots = std::stoul(reader.Get(sectionName.data(), "NumberOfShots", "1"));
    m_vectWeaponsINIData[numIt].m_attackType = static_cast<AttackType_e>(
                std::stoul(reader.Get(sectionName.data(), "AttackType", "1")));
    if(m_vectWeaponsINIData[numIt].m_attackType == AttackType_e::VISIBLE_SHOTS)
    {
        m_vectWeaponsINIData[numIt].m_visibleShootID = reader.Get(sectionName.data(), "ShootSpritesID", "");
        assert(!m_vectWeaponsINIData[numIt].m_visibleShootID.empty());
    }
    sprites = reader.Get(sectionName.data(), "StaticSprite", "");
    sprites += " ";
    sprites += reader.Get(sectionName.data(), "AttackSprite", "");
    assert(!sprites.empty() && "Wall sprites cannot be loaded.");
    vectStr_t vectSprites = convertStrToVectStr(sprites);
    std::string resultWeight = reader.Get(sectionName.data(), "SpriteWeightGame", ""),
            resultHeight = reader.Get(sectionName.data(), "SpriteHeightGame", "");
    std::vector<float> vectWeight = convertStrToVectFloat(resultWeight),
            vectHeight = convertStrToVectFloat(resultHeight);
    assert(vectHeight.size() == vectWeight.size());
    assert(vectSprites.size() == vectWeight.size());
    m_vectWeaponsINIData[numIt].m_spritesData.reserve(vectSprites.size());
    latency = reader.Get(sectionName.data(), "AnimationLatency", "");
    assert(!latency.empty());
    m_vectWeaponsINIData[numIt].m_animationLatency = std::stof(latency);
    for(uint32_t i = 0; i < vectSprites.size(); ++i)
    {
        m_vectWeaponsINIData[numIt].m_spritesData.emplace_back(
                    WeaponSpriteData{*(m_pictureData.getIdentifier(vectSprites[i])),
                                      {vectWeight[i], vectHeight[i]}});
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
        vectStr_t results = convertStrToVectStr(sprites);
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

        loadEnemySprites(reader, vectINISections[i], EnemySpriteElementType_e::ATTACK,
                         m_enemyData[vectINISections[i]]);
        loadEnemySprites(reader, vectINISections[i], EnemySpriteElementType_e::DYING,
                         m_enemyData[vectINISections[i]]);
        loadEnemySprites(reader, vectINISections[i], EnemySpriteElementType_e::TOUCHED,
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
    case EnemySpriteElementType_e::DYING:
        spriteType = "DyingSprite";
        vectPtr = &enemyData.m_dyingSprites;
        break;
    case EnemySpriteElementType_e::TOUCHED:
        spriteType = "Touched";
        vectPtr = &enemyData.m_touched;
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
    assert((spriteTypeEnum != EnemySpriteElementType_e::VISIBLE_SHOOT ||
            spriteTypeEnum != EnemySpriteElementType_e::VISIBLE_SHOOT_DESTRUCT ||
            !sprites.empty()) && "Enemy sprites cannot be loaded.");
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

//======================================================
std::vector<std::string> convertStrToVectStr(const std::string &str)
{
    std::istringstream iss(str);
    return std::vector<std::string>(std::istream_iterator<std::string>{iss},
                      std::istream_iterator<std::string>());
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
    loadWallData(reader);
    loadGeneralStaticElements(reader, LevelStaticElementType_e::GROUND);
    loadGeneralStaticElements(reader, LevelStaticElementType_e::CEILING);
    loadGeneralStaticElements(reader, LevelStaticElementType_e::OBJECT);
    loadExit(reader);
    loadDoorData(reader);
    loadVisibleShootDisplayData(reader);
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
    for(it = m_ceilingElement.begin(); it != m_ceilingElement.end(); ++it)
    {
        fillStandartPositionVect(reader, it->first, it->second.m_TileGamePosition);
    }
    for(it = m_objectElement.begin(); it != m_objectElement.end(); ++it)
    {
        fillStandartPositionVect(reader, it->first, it->second.m_TileGamePosition);
    }
}

//===================================================================
void LevelManager::loadLevel(const std::string &INIFileName, uint32_t levelNum)
{
    INIReader reader(LEVEL_RESSOURCES_DIR_STR + std::string("Level") +
                     std::to_string(levelNum) + std::string ("/") + INIFileName);
    if(reader.ParseError() != 0)
    {
        assert("Error while reading INI file.");
    }
    loadLevelData(reader);
    loadPositionPlayerData(reader);
    loadPositionWall(reader);
    loadPositionStaticElements(reader);
    loadPositionExit(reader);
    loadPositionDoorData(reader);
    loadPositionEnemyData(reader);
    loadBackgroundData(reader);
}

//===================================================================
void LevelManager::clearExistingPositionsElement()
{
    m_exitStaticElement.m_TileGamePosition.clear();
    for(std::map<std::string, WallData>::iterator it = m_wallData.begin(); it != m_wallData.end(); ++it)
    {
        it->second.m_TileGamePosition.clear();
    }
    for(std::map<std::string, StaticLevelElementData>::iterator it = m_groundElement.begin(); it != m_groundElement.end(); ++it)
    {
        it->second.m_TileGamePosition.clear();
    }
    for(std::map<std::string, StaticLevelElementData>::iterator it = m_ceilingElement.begin(); it != m_ceilingElement.end(); ++it)
    {
        it->second.m_TileGamePosition.clear();
    }
    for(std::map<std::string, StaticLevelElementData>::iterator it = m_objectElement.begin(); it != m_objectElement.end(); ++it)
    {
        it->second.m_TileGamePosition.clear();
    }
    for(std::map<std::string, DoorData>::iterator it = m_doorData.begin(); it != m_doorData.end(); ++it)
    {
        it->second.m_TileGamePosition.clear();
    }
    for(std::map<std::string, EnemyData>::iterator it = m_enemyData.begin(); it != m_enemyData.end(); ++it)
    {
        it->second.m_TileGamePosition.clear();
    }
}
