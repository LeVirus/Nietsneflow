#include <LevelManager.hpp>
#include <MainEngine.hpp>
#include <inireader.h>
#include <cassert>
#include <sstream>
#include <iostream>
#include <iterator>
#include <filesystem>

//===================================================================
LevelManager::LevelManager()
{
    std::ifstream inStream(LEVEL_RESSOURCES_DIR_STR + "Saves/CustomSettings.ini");
    m_ini.parse(inStream);
    inStream.close();
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
    std::vector<std::string> sections = reader.getSectionNamesContaining("GroundBackground");
    GroundCeilingData groundData, ceilingData;
    uint32_t colorIndex = static_cast<uint32_t>(DisplayType_e::COLOR),
            simpleTextureIndex = static_cast<uint32_t>(DisplayType_e::SIMPLE_TEXTURE),
            tiledTextureIndex = static_cast<uint32_t>(DisplayType_e::TEXTURED_TILE);
    groundData.m_apparence.reset();
    ceilingData.m_apparence.reset();
    for(uint32_t i = 0; i < sections.size() ; ++i)
    {
        if(sections[i] == "ColorGroundBackground")
        {
            std::vector<float> colorR, colorG, colorB;
            colorR = convertStrToVectFloat(reader.Get(sections[i], "colorR", ""));
            colorG = convertStrToVectFloat(reader.Get(sections[i], "colorG", ""));
            colorB = convertStrToVectFloat(reader.Get(sections[i], "colorB", ""));
            for(uint32_t j = 0; j < 4 ; ++j)
            {
                groundData.m_color[j] = tupleFloat_t{colorR[j], colorG[j], colorB[j]};
            }
            groundData.m_apparence[colorIndex] = true;
        }
        else if(sections[i] == "SimpleTextureGroundBackground")
        {
            std::optional<uint8_t> picNum = m_pictureData.getIdentifier(reader.Get(sections[i], "sprite", ""));
            assert(picNum);
            groundData.m_spriteSimpleTextNum = *picNum;
            groundData.m_apparence[simpleTextureIndex] = true;
        }
        else if(sections[i] == "TiledTextureGroundBackground")
        {
            std::optional<uint8_t> picNum = m_pictureData.getIdentifier(reader.Get(sections[i], "sprite", ""));
            assert(picNum);
            groundData.m_spriteTiledTextNum = *picNum;
            groundData.m_apparence[tiledTextureIndex] = true;
        }
    }
    sections = reader.getSectionNamesContaining("CeilingBackground");
    for(uint32_t i = 0; i < sections.size() ; ++i)
    {
        if(sections[i] == "ColorCeilingBackground")
        {
            std::vector<float> colorR, colorG, colorB;
            colorR = convertStrToVectFloat(reader.Get(sections[i], "colorR", ""));
            colorG = convertStrToVectFloat(reader.Get(sections[i], "colorG", ""));
            colorB = convertStrToVectFloat(reader.Get(sections[i], "colorB", ""));
            for(uint32_t j = 0; j < 4 ; ++j)
            {
                ceilingData.m_color[j] = tupleFloat_t{colorR[j], colorG[j], colorB[j]};
            }
            ceilingData.m_apparence[colorIndex] = true;
        }
        else if(sections[i] == "SimpleTextureCeilingBackground")
        {
            std::optional<uint8_t> picNum = m_pictureData.getIdentifier(reader.Get(sections[i], "sprite", ""));
            assert(picNum);
            ceilingData.m_spriteSimpleTextNum = *picNum;
            ceilingData.m_apparence[simpleTextureIndex] = true;
        }
        else if(sections[i] == "TiledTextureCeilingBackground")
        {
            std::optional<uint8_t> picNum = m_pictureData.getIdentifier(reader.Get(sections[i], "sprite", ""));
            assert(picNum);
            ceilingData.m_spriteTiledTextNum = *picNum;
            ceilingData.m_apparence[tiledTextureIndex] = true;
        }
    }
    m_pictureData.setBackgroundData(groundData, ceilingData);
}

//===================================================================
void LevelManager::loadMusicData(const INIReader &reader)
{
    m_level.setMusicFile(reader.Get("Level", "music", ""));
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
void LevelManager::loadGeneralStaticElements(const INIReader &reader, LevelStaticElementType_e elementType)
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
    case LevelStaticElementType_e::TELEPORT:
        vectINISections = reader.getSectionNamesContaining("Teleport");
        memMap = &m_teleportElement;
        break;
    case LevelStaticElementType_e::IMPACT:
        break;
    }
    for(uint32_t i = 0; i < vectINISections.size(); ++i)
    {
        memMap->insert({vectINISections[i], StaticLevelElementData()});
        readStandardStaticElement(reader, memMap->operator[](vectINISections[i]), vectINISections[i], elementType);
    }
}

//===================================================================
void LevelManager::loadBarrelsData(const INIReader &reader)
{
    std::vector<std::string> vectINISections;
    vectINISections = reader.getSectionNamesContaining("Barrel");
    assert(!vectINISections.empty());
    //Static
    m_barrelElement.m_staticSprite = getVectSpriteNum(reader, vectINISections[0], "StaticSprite");
    m_barrelElement.m_inGameStaticSpriteSize.first = reader.GetReal(vectINISections[0], "StaticSpriteWeightGame", 0.0f);
    m_barrelElement.m_inGameStaticSpriteSize.second = reader.GetReal(vectINISections[0], "StaticSpriteHeightGame", 0.0f);
    //Explosion
    m_barrelElement.m_explosionSprite = getVectSpriteNum(reader, vectINISections[0], "ExplosionSprite");
    m_barrelElement.m_vectinGameExplosionSpriteSize = getVectSpriteGLSize(reader, vectINISections[0],
            "ExplosionSpriteWeightGame", "ExplosionSpriteHeightGame");
    m_barrelElement.m_explosionSoundFile = reader.Get(vectINISections[0], "ExplosionSound", "");
    assert(m_barrelElement.m_vectinGameExplosionSpriteSize.size() == m_barrelElement.m_explosionSprite.size());
}

//===================================================================
std::vector<uint8_t> LevelManager::getVectSpriteNum(const INIReader &reader, const std::string_view section, const std::string_view param)
{
    std::string str = reader.Get(section.data(), param.data(), "");
    assert(!str.empty());
    vectStr_t vectSprite = convertStrToVectStr(str);
    std::vector<uint8_t> retVect;
    retVect.reserve(vectSprite.size());
    for(uint32_t i = 0; i < vectSprite.size(); ++i)
    {
        std::optional<uint8_t> id = m_pictureData.getIdentifier(vectSprite[i]);
        assert(id);
        retVect.emplace_back(*id);
    }
    return retVect;
}

//===================================================================
std::vector<PairFloat_t> LevelManager::getVectSpriteGLSize(const INIReader &reader, const std::string_view section,
                                                           const std::string_view weightParam, const std::string_view heightParam)
{
    std::vector<PairFloat_t> retVect;
    std::string str = reader.Get(section.data(), weightParam.data(), "");
    assert(!str.empty());
    std::vector<float> vectWeight = convertStrToVectFloat(str);

    str = reader.Get(section.data(), heightParam.data(), "");
    assert(!str.empty());
    std::vector<float> vectHeight = convertStrToVectFloat(str);
    assert(vectHeight.size() == vectWeight.size());
    retVect.reserve(vectHeight.size());
    for(uint32_t i = 0; i < vectHeight.size(); ++i)
    {
        retVect.emplace_back(PairFloat_t{vectWeight[i], vectHeight[i]});
    }
    return retVect;
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
void LevelManager::loadVisualTeleportData(const INIReader &reader)
{
    std::vector<std::string> vectINISections;
    vectINISections = reader.getSectionNamesContaining("TelepAnim");
    assert(!vectINISections.empty());
    std::string str = reader.Get(vectINISections[0], "Sprite", "");
    assert(!str.empty());
    vectStr_t vectSprite = convertStrToVectStr(str);
    str = reader.Get(vectINISections[0], "SpriteWeightGame", "");
    assert(!str.empty());
    std::vector<float> vectWeight = convertStrToVectFloat(str);
    str = reader.Get(vectINISections[0], "SpriteHeightGame", "");
    assert(!str.empty());
    std::vector<float> vectHeight = convertStrToVectFloat(str);
    m_displayTeleportData.resize(vectSprite.size());
    for(uint32_t i = 0; i < vectSprite.size(); ++i)
    {
        m_displayTeleportData[i].m_numSprite = *m_pictureData.getIdentifier(vectSprite[i]);
        m_displayTeleportData[i].m_GLSize = {vectWeight[i], vectHeight[i]};
    }
}

//===================================================================
void LevelManager::loadTriggerElements(const INIReader &reader)
{
    std::vector<std::string> vectINISections;
    vectINISections = reader.getSectionNamesContaining("Trigger");
    if(vectINISections.empty())
    {
        return;
    }
    std::string spriteID;
    for(uint32_t i = 0; i < vectINISections.size(); ++i)
    {
        spriteID = reader.Get(vectINISections[i], "Sprite", "");
        assert(!spriteID.empty());
        m_triggerDisplayData.insert({vectINISections[i], MemSpriteData()});
        m_triggerDisplayData[vectINISections[i]].m_numSprite = *m_pictureData.getIdentifier(spriteID);
        m_triggerDisplayData[vectINISections[i]].m_GLSize.first =
                reader.GetReal(vectINISections[i], "SpriteWeightGame", 1.0);
        m_triggerDisplayData[vectINISections[i]].m_GLSize.second =
                reader.GetReal(vectINISections[i], "SpriteHeightGame", 1.0);
    }
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
                                             const std::string &sectionName, LevelStaticElementType_e elementType)
{
    loadSpriteData(reader, sectionName, staticElement);
    if(elementType == LevelStaticElementType_e::OBJECT)
    {
        uint32_t type = reader.GetInteger(sectionName, "Type", 4);
        assert(type < static_cast<uint32_t>(ObjectType_e::TOTAL));
        staticElement.m_type = static_cast<ObjectType_e>(type);
        if(staticElement.m_type == ObjectType_e::AMMO_WEAPON || staticElement.m_type == ObjectType_e::WEAPON ||
                staticElement.m_type == ObjectType_e::HEAL)
        {
            staticElement.m_containing = reader.GetInteger(sectionName, "Containing", 1);
        }
        if(staticElement.m_type == ObjectType_e::AMMO_WEAPON || staticElement.m_type == ObjectType_e::WEAPON)
        {
            std::map<std::string, uint32_t>::const_iterator it = m_weaponINIAssociated.find(reader.Get(sectionName, "WeaponID", ""));
            assert(it != m_weaponINIAssociated.end());
            staticElement.m_weaponID = it->second;
        }
        else if(staticElement.m_type == ObjectType_e::CARD)
        {
            staticElement.m_cardID = reader.GetInteger(sectionName, "CardID", 4);
            m_cardINIAssociated.insert({sectionName, *staticElement.m_cardID});
        }
    }
    if(elementType != LevelStaticElementType_e::OBJECT)
    {
        staticElement.m_traversable = reader.GetBoolean(sectionName, "traversable", true);
    }
}

//===================================================================
std::optional<std::vector<uint32_t>> getBrutPositionData(const INIReader &reader, const std::string &sectionName,
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
void LevelManager::fillStandartPositionVect(const INIReader &reader, const std::string &sectionName, VectPairUI_t &vectPos)
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
        vectPos.emplace_back(PairUI_t{(*results)[j], (*results)[j + 1]});
        deleteWall(vectPos.back());
    }
}

//===================================================================
void LevelManager::fillTeleportPositions(const INIReader &reader, const std::string &sectionName)
{
    std::optional<std::vector<uint32_t>> resultsPosA = getBrutPositionData(reader, sectionName, "PosA"),
            resultsPosB = getBrutPositionData(reader, sectionName, "PosB");
    if(!resultsPosA)
    {
        return;
    }
    assert(resultsPosB);
    assert((*resultsPosA).size() % 2 == 0);
    assert((*resultsPosB).size() % 2 == 0);
    assert((*resultsPosB).size() == (*resultsPosA).size());
    uint32_t vectSize = (*resultsPosA).size() / 2;
    m_teleportElement[sectionName].m_teleportData = TeleportData();
    m_teleportElement[sectionName].m_teleportData->m_biDirection =
            convertStrToVectBool(reader.Get(sectionName, "BiDirection", ""));
    assert(m_teleportElement[sectionName].m_teleportData->m_biDirection.size() == vectSize);
    VectPairUI_t &vect = m_teleportElement[sectionName].m_teleportData->m_targetTeleport;
    vect.reserve(vectSize);
    m_teleportElement[sectionName].m_TileGamePosition.reserve(vectSize);
    for(uint32_t j = 0; j < (*resultsPosA).size(); j += 2)
    {
        //Origin
        m_teleportElement[sectionName].m_TileGamePosition.emplace_back(PairUI_t{(*resultsPosA)[j], (*resultsPosA)[j + 1]});
        //Target
        vect.emplace_back(PairUI_t{(*resultsPosB)[j], (*resultsPosB)[j + 1]});
        if(m_teleportElement[sectionName].m_teleportData->m_biDirection[j / 2])
        {
            //Origin
            m_teleportElement[sectionName].m_TileGamePosition.emplace_back(PairUI_t{(*resultsPosB)[j], (*resultsPosB)[j + 1]});
            //Target
            vect.emplace_back(PairUI_t{(*resultsPosA)[j], (*resultsPosA)[j + 1]});
        }
    }
}

//===================================================================
std::optional<PairUI_t> LevelManager::getPosition(const INIReader &reader,
                                                  const std::string_view sectionName, const std::string_view propertyName)
{
    std::optional<std::vector<uint32_t>> results = getBrutPositionData(reader, sectionName.data(), propertyName.data());
    if(!results || results->size() != 2)
    {
        return {};
    }
    return PairUI_t{(*results)[0], (*results)[1]};
}

//===================================================================
void LevelManager::deleteWall(const PairUI_t &coord)
{
    std::set<PairUI_t>::iterator itt;
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
                                        std::set<PairUI_t> &vectPos)
{
    std::optional<std::vector<uint32_t>> results = getBrutPositionData(reader, sectionName, propertyName);
    if(!results || (*results).empty())
    {
        return false;
    }
    PairUI_t origins;
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
void fillPositionVerticalLine(const PairUI_t &origins, uint32_t size,
                              std::set<PairUI_t> &vectPos)
{
    PairUI_t current = origins;
    for(uint32_t j = 0; j < size; ++j, ++current.second)
    {
        vectPos.insert(current);
    }
}

//===================================================================
void fillPositionHorizontalLine(const PairUI_t &origins, uint32_t size,
                                std::set<PairUI_t> &vectPos)
{
    PairUI_t current = origins;
    for(uint32_t j = 0; j < size; ++j, ++current.first)
    {
        vectPos.insert(current);
    }
}

//===================================================================
void fillPositionRectangle(const PairUI_t &origins, const PairUI_t &size,
                           std::set<PairUI_t> &vectPos)
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
void fillPositionDiagLineUpLeft(const PairUI_t &origins, uint32_t size,
                                std::set<PairUI_t> &vectPos)
{
    PairUI_t current = origins;
    for(uint32_t j = 0; j < size; ++j, ++current.first, ++current.second)
    {
        vectPos.insert(current);
    }
}

//===================================================================
void fillPositionDiagLineDownLeft(const PairUI_t &origins, uint32_t size,
                                  std::set<PairUI_t> &vectPos)
{
    PairUI_t current = origins;
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
void fillPositionDiagRectangle(const PairUI_t &origins, uint32_t size,
                               std::set<PairUI_t> &vectPos)
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
void LevelManager::loadVisibleShotDisplayData(const INIReader &reader)
{
    std::vector<std::string> vectINISections = reader.getSectionNamesContaining("VisibleShoot");
    if(vectINISections.empty())
    {
        return;
    }
    std::string sprites, weight, height, impactSound;
    vectStr_t vectSprites;
    std::vector<float> vectWeight, vectHeight;
    for(uint32_t i = 0; i < vectINISections.size(); ++i)
    {
        sprites = reader.Get(vectINISections[i], "Sprites", "");
        weight = reader.Get(vectINISections[i], "SpriteWeightGame", "");
        height = reader.Get(vectINISections[i], "SpriteHeightGame", "");
        vectWeight = convertStrToVectFloat(weight);
        vectHeight = convertStrToVectFloat(height);
        vectSprites = convertStrToVectStr(sprites);
        assert(vectWeight.size() == vectHeight.size());
        assert(vectSprites.size() == vectHeight.size());
        impactSound = reader.Get(vectINISections[i], "ImpactSound", "");
        assert(!impactSound.empty());
        m_visibleShootINIData.insert({vectINISections[i],
                                      {impactSound, std::vector<MemSpriteData>()}});
        for(uint32_t j = 0; j < vectSprites.size(); ++j)
        {
            m_visibleShootINIData[vectINISections[i]].second.emplace_back(
                        MemSpriteData{*m_pictureData.getIdentifier(vectSprites[j]), {vectWeight[j], vectHeight[j]}});
        }
    }
}

//===================================================================
void LevelManager::loadShotImpactDisplayData(const INIReader &reader)
{
    std::vector<std::string> vectINISections = reader.getSectionNamesContaining("Impact");
    if(vectINISections.empty())
    {
        return;
    }
    std::string spritesStd, spritesImpact;
    float weight, height;
    vectStr_t vectSprites;
    for(uint32_t i = 0; i < vectINISections.size(); ++i)
    {
        spritesStd = reader.Get(vectINISections[i], "SpritesStd", "");
        spritesImpact = reader.Get(vectINISections[i], "SpriteTouched", "");
        assert(!spritesStd.empty());
        weight = reader.GetReal(vectINISections[i], "SpriteWeightGame", 1.0f);
        height = reader.GetReal(vectINISections[i], "SpriteHeightGame", 1.0f);
        vectSprites = convertStrToVectStr(spritesStd);
        m_impactINIData.insert({vectINISections[i], PairImpactData_t()});
        for(uint32_t j = 0; j < vectSprites.size(); ++j)
        {
            m_impactINIData[vectINISections[i]].first.emplace_back(
                        MemSpriteData{*m_pictureData.getIdentifier(vectSprites[j]), {weight, height}});
        }
        m_impactINIData[vectINISections[i]].second =
                    MemSpriteData{*m_pictureData.getIdentifier(spritesImpact), {weight, height}};
    }
}

//===================================================================
void LevelManager::loadWeaponsData(const INIReader &reader)
{
    std::vector<std::string> vectINISections;
    vectINISections = reader.getSectionNamesContaining("Weapon");
    assert(!vectINISections.empty());
    m_vectWeaponsINIData.resize(vectINISections.size());
    for(uint32_t i = 0; i < vectINISections.size(); ++i)
    {
        loadWeaponData(reader, vectINISections[i], i);
        loadDisplayData(reader, vectINISections[i], "ShotImpact");
    }
}

//===================================================================
void LevelManager::loadGeneralSoundData(const INIReader &reader)
{
    m_pickObjectSound = reader.Get("SoundUtils", "PickUpObjectSound", "");
    assert(!m_pickObjectSound.empty());
    m_doorOpeningSound = reader.Get("SoundUtils", "DoorOpeningSound", "");
    assert(!m_doorOpeningSound.empty());
    m_hitSound = reader.Get("SoundUtils", "HitSound", "");
    assert(!m_hitSound.empty());
    m_teleportSound = reader.Get("SoundUtils", "TeleportSound", "");
    assert(!m_teleportSound.empty());
}

//===================================================================
void LevelManager::loadDisplayData(const INIReader &reader, std::string_view sectionName,
                                   std::string_view subSectionName)
{
    std::string sprites = reader.Get(sectionName.data(), subSectionName.data(), "");
    if(sprites.empty())
    {
        return;
    }
    vectStr_t vectsprite = convertStrToVectStr(sprites);
    std::vector<MemSpriteData> *vect = nullptr;
    assert(vect);
    (*vect).reserve(vectsprite.size());
    for(uint32_t i = 0; i < vectsprite.size(); ++i)
    {
        (*vect).emplace_back(MemSpriteData{(*(m_pictureData.getIdentifier(vectsprite[i]))), {0.5f, 0.5f}});
    }
}

//===================================================================
void LevelManager::loadWeaponData(const INIReader &reader, std::string_view sectionName, uint32_t numIt)
{
    std::string sprites, str;
    m_vectWeaponsINIData[numIt].m_maxAmmo = reader.GetInteger(sectionName.data(), "MaxAmmo", 1);
    assert(m_vectWeaponsINIData[numIt].m_maxAmmo != 1);
    m_vectWeaponsINIData[numIt].m_simultaneousShots = reader.GetInteger(sectionName.data(), "NumberOfShots", 1);
    m_vectWeaponsINIData[numIt].m_animMode = static_cast<AnimationMode_e>(reader.GetInteger(sectionName.data(), "AnimationType", 0));
    m_vectWeaponsINIData[numIt].m_order = reader.GetInteger(sectionName.data(), "Order", 100000);
    m_vectWeaponsINIData[numIt].m_lastAnimNum = reader.GetInteger(sectionName.data(), "LastAnimNum", 1);
    m_vectWeaponsINIData[numIt].m_simultaneousShots = reader.GetInteger(sectionName.data(), "NumberOfShots", 1);
    m_vectWeaponsINIData[numIt].m_startingPossess = reader.GetBoolean(sectionName.data(), "Possess", false);
    m_vectWeaponsINIData[numIt].m_startingAmmoCount = reader.GetInteger(sectionName.data(), "BaseAmmo", 0);
    m_vectWeaponsINIData[numIt].m_damage = reader.GetInteger(sectionName.data(), "Damage", 1);
    m_vectWeaponsINIData[numIt].m_impactID = reader.Get(sectionName.data(), "ShotImpactID", "");
    m_vectWeaponsINIData[numIt].m_shotVelocity = reader.GetReal(sectionName.data(), "ShotVelocity", 1.0f);
    m_vectWeaponsINIData[numIt].m_attackType = static_cast<AttackType_e>(
                reader.GetInteger(sectionName.data(), "AttackType", 1));
    m_vectWeaponsINIData[numIt].m_shotSound = reader.Get(sectionName.data(), "SoundEffect", "");
    m_vectWeaponsINIData[numIt].m_reloadSound = reader.Get(sectionName.data(), "ReloadSound", "");
    if(m_vectWeaponsINIData[numIt].m_attackType == AttackType_e::VISIBLE_SHOTS)
    {
        m_vectWeaponsINIData[numIt].m_visibleShootID = reader.Get(sectionName.data(), "ShootSpritesID", "");
        assert(!m_vectWeaponsINIData[numIt].m_visibleShootID.empty());
    }
    m_weaponINIAssociated.insert({sectionName.data(), m_vectWeaponsINIData[numIt].m_order});
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
    m_vectWeaponsINIData[numIt].m_animationLatency = reader.GetReal(sectionName.data(), "AnimationLatency", -1.0f);
    assert(m_vectWeaponsINIData[numIt].m_animationLatency > 0.00f);
    m_vectWeaponsINIData[numIt].m_damageCircleRay = reader.GetReal(sectionName.data(), "DamageZoneRay", -1.0f);
    if(m_vectWeaponsINIData[numIt].m_damageCircleRay < 0.0f)
    {
        m_vectWeaponsINIData[numIt].m_damageCircleRay = std::nullopt;
    }
    for(uint32_t i = 0; i < vectSprites.size(); ++i)
    {
        m_vectWeaponsINIData[numIt].m_spritesData.emplace_back(
                    MemSpriteData{*(m_pictureData.getIdentifier(vectSprites[i])),
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
    std::vector<std::string> vectINISections = reader.getSectionNamesContaining("WallShape");
    std::map<std::string, WallData>::iterator it;
    std::string direction, moveNumber;
    for(uint32_t i = 0; i < vectINISections.size(); ++i)
    {
        direction = reader.Get(vectINISections[i], "Direction", "");
        it = m_wallData.find(reader.Get(vectINISections[i], "WallDisplayID", ""));
        assert(it != m_wallData.end());
        //Moveable wall
        m_mainWallData.insert({vectINISections[i], MoveableWallData()});
        m_mainWallData[vectINISections[i]].m_sprites = it->second.m_sprites;
        fillWallPositionVect(reader, vectINISections[i], "GamePosition",
                             m_mainWallData[vectINISections[i]].m_TileGamePosition);
        fillWallPositionVect(reader, vectINISections[i], "RemovePosition",
                             m_mainWallData[vectINISections[i]].m_removeGamePosition);
        if(direction.empty())
        {
            continue;
        }
        moveNumber = reader.Get(vectINISections[i], "NumberOfMove", "");
        assert(!direction.empty());
        assert(!moveNumber.empty());
        std::vector<uint32_t> vectDir = convertStrToVectUI(direction);
        std::vector<uint32_t> vectMov = convertStrToVectUI(moveNumber);
        assert(vectDir.size() == vectMov.size());
        m_mainWallData[vectINISections[i]].m_directionMove.reserve(vectDir.size());
        for(uint32_t j = 0; j < vectDir.size(); ++j)
        {
            m_mainWallData[vectINISections[i]].m_directionMove.emplace_back(
                        std::pair<Direction_e, uint32_t>{static_cast<Direction_e>(vectDir[j]), vectMov[j]});
        }
        m_mainWallData[vectINISections[i]].m_velocity = reader.GetReal(vectINISections[i], "Velocity", 1.0f);
        m_mainWallData[vectINISections[i]].m_triggerType =
                static_cast<TriggerWallMoveType_e>(reader.GetInteger(vectINISections[i], "TriggerType", 0));
        m_mainWallData[vectINISections[i]].m_triggerBehaviourType =
                static_cast<TriggerBehaviourType_e>(reader.GetInteger(vectINISections[i], "TriggerBehaviourType", 0));
        if(m_mainWallData[vectINISections[i]].m_triggerType == TriggerWallMoveType_e::BUTTON)
        {
            loadTriggerLevelData(reader, vectINISections[i]);
        }
        else if(m_mainWallData[vectINISections[i]].m_triggerType == TriggerWallMoveType_e::GROUND)
        {
            m_mainWallData[vectINISections[i]].m_groundTriggerPos = PairUI_t();
            m_mainWallData[vectINISections[i]].m_groundTriggerPos =
                    *getPosition(reader,  vectINISections[i], "TriggerGamePosition");
        }
    }
}

//===================================================================
void LevelManager::loadTriggerLevelData(const INIReader &reader, const std::string &sectionName)
{
    std::string str;
    std::map<std::string, MemSpriteData>::iterator it;
    m_mainWallData[sectionName].m_associatedTriggerData = AssociatedTriggerData();
    str = reader.Get(sectionName, "TriggerDisplayID", "");
    it = m_triggerDisplayData.find(str);
    assert(it != m_triggerDisplayData.end());
    m_mainWallData[sectionName].m_associatedTriggerData->m_displayData = it->second;
    m_mainWallData[sectionName].m_associatedTriggerData->m_pos =
            *getPosition(reader,  sectionName, "TriggerGamePosition");
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
        std::string cardStr = reader.Get(vectINISections[i], "CardID", "");
        if(!cardStr.empty())
        {
            std::map<std::string, uint32_t>::const_iterator it = m_cardINIAssociated.find(cardStr);
            assert(it != m_cardINIAssociated.end());
            m_doorData[vectINISections[i]].m_cardID = {it->second, it->first};
        }
        else
        {
            m_doorData[vectINISections[i]].m_cardID = std::nullopt;
        }
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
    std::string str;
    for(uint32_t i = 0; i < vectINISections.size(); ++i)
    {
        m_enemyData.insert({vectINISections[i], EnemyData()});
        m_enemyData[vectINISections[i]].m_traversable = reader.GetBoolean(vectINISections[i], "traversable", false);
        m_enemyData[vectINISections[i]].m_inGameSpriteSize.first = reader.GetReal(vectINISections[i], "SpriteWeightGame", 1.0);
        m_enemyData[vectINISections[i]].m_inGameSpriteSize.second = reader.GetReal(vectINISections[i], "SpriteHeightGame", 1.0);
        m_enemyData[vectINISections[i]].m_visibleShootID = reader.Get(vectINISections[i], "ShootSpritesID", "");
        m_enemyData[vectINISections[i]].m_impactID = reader.Get(vectINISections[i], "ShotImpactID", "");
        m_enemyData[vectINISections[i]].m_attackPower = reader.GetInteger(vectINISections[i], "Damage", 1);
        m_enemyData[vectINISections[i]].m_life = reader.GetInteger(vectINISections[i], "Life", 1);
        m_enemyData[vectINISections[i]].m_velocity = reader.GetReal(vectINISections[i], "Velocity", 1.0f);
        m_enemyData[vectINISections[i]].m_shotVelocity = reader.GetReal(vectINISections[i], "ShotVelocity", 1.0f);
        m_enemyData[vectINISections[i]].m_dropedObjectID = reader.Get(vectINISections[i], "DropedObjectID", "");
        m_enemyData[vectINISections[i]].m_meleeDamage = reader.GetInteger(vectINISections[i], "MeleeDamage", 0);
        m_enemyData[vectINISections[i]].m_normalBehaviourSoundFile = reader.Get(vectINISections[i], "NormalBehaviourSound", "");
        m_enemyData[vectINISections[i]].m_detectBehaviourSoundFile = reader.Get(vectINISections[i], "DetectBehaviourSound", "");
        m_enemyData[vectINISections[i]].m_attackSoundFile = reader.Get(vectINISections[i], "AttackSound", "");
        loadEnemySprites(reader, vectINISections[i], EnemySpriteElementType_e::STATIC_FRONT, m_enemyData[vectINISections[i]]);
        loadEnemySprites(reader, vectINISections[i], EnemySpriteElementType_e::STATIC_FRONT_LEFT, m_enemyData[vectINISections[i]]);
        loadEnemySprites(reader, vectINISections[i], EnemySpriteElementType_e::STATIC_FRONT_RIGHT, m_enemyData[vectINISections[i]]);
        loadEnemySprites(reader, vectINISections[i], EnemySpriteElementType_e::STATIC_BACK, m_enemyData[vectINISections[i]]);
        loadEnemySprites(reader, vectINISections[i], EnemySpriteElementType_e::STATIC_BACK_LEFT, m_enemyData[vectINISections[i]]);
        loadEnemySprites(reader, vectINISections[i], EnemySpriteElementType_e::STATIC_BACK_RIGHT, m_enemyData[vectINISections[i]]);
        loadEnemySprites(reader, vectINISections[i], EnemySpriteElementType_e::STATIC_LEFT, m_enemyData[vectINISections[i]]);
        loadEnemySprites(reader, vectINISections[i], EnemySpriteElementType_e::STATIC_RIGHT, m_enemyData[vectINISections[i]]);
        loadEnemySprites(reader, vectINISections[i], EnemySpriteElementType_e::ATTACK, m_enemyData[vectINISections[i]]);
        loadEnemySprites(reader, vectINISections[i], EnemySpriteElementType_e::DYING, m_enemyData[vectINISections[i]]);
        loadEnemySprites(reader, vectINISections[i], EnemySpriteElementType_e::TOUCHED, m_enemyData[vectINISections[i]]);

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
    vectINISections = reader.getSectionNamesContaining("GraphicUtils");
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
    }
    assert(vectPtr);
    std::string sprites = reader.Get(sectionName, spriteType, "");
    assert((!sprites.empty()) && "Enemy sprites cannot be loaded.");
    std::istringstream iss(sprites);
    vectStr_t results(std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>());
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
std::vector<bool> convertStrToVectBool(const std::string &str)
{
    std::istringstream iss(str);
    return std::vector<bool>(std::istream_iterator<bool>{iss},
                              std::istream_iterator<bool>());
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
    loadWeaponsData(reader);
    loadGeneralSoundData(reader);
    loadGeneralStaticElements(reader, LevelStaticElementType_e::GROUND);
    loadGeneralStaticElements(reader, LevelStaticElementType_e::CEILING);
    loadGeneralStaticElements(reader, LevelStaticElementType_e::OBJECT);
    loadGeneralStaticElements(reader, LevelStaticElementType_e::TELEPORT);
    loadVisualTeleportData(reader);
    loadBarrelsData(reader);
    loadExit(reader);
    loadTriggerElements(reader);
    loadDoorData(reader);
    loadVisibleShotDisplayData(reader);
    loadShotImpactDisplayData(reader);
    loadEnemyData(reader);
    loadUtilsData(reader);
    m_weaponINIAssociated.clear();
    m_cardINIAssociated.clear();
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
    //OOOOK only one teleport element
    for(it = m_teleportElement.begin(); it != m_teleportElement.end(); ++it)
    {
        fillTeleportPositions(reader, it->first);
    }
}

//===================================================================
void LevelManager::loadBarrelElements(const INIReader &reader)
{
    //OOOOK refact pos
    m_barrelElement.m_TileGamePosition = getPositionData(reader, "Barrel", "GamePosition");
}

//===================================================================
VectPairUI_t getPositionData(const INIReader &reader, const std::string &sectionName, const std::string &propertyName)
{
    VectPairUI_t vectRet;
    std::optional<std::vector<uint32_t>> resultsPos = getBrutPositionData(reader, sectionName.data(), propertyName.data());
    if(!resultsPos)
    {
        return {};
    }
    assert((*resultsPos).size() % 2 == 0);
    vectRet.reserve((*resultsPos).size() / 2);
    for(uint32_t i = 0; i < (*resultsPos).size(); ++i)
    {
        vectRet.emplace_back(PairUI_t{(*resultsPos)[i], (*resultsPos)[i + 1]});
    }
    return vectRet;
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
    m_mainWallData.clear();
    loadLevelData(reader);
    loadPositionPlayerData(reader);
    loadPositionWall(reader);
    loadPositionStaticElements(reader);
    loadBarrelElements(reader);
    loadPositionExit(reader);
    loadPositionDoorData(reader);
    loadPositionEnemyData(reader);
    loadBackgroundData(reader);
    loadMusicData(reader);
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
    for(std::map<std::string, StaticLevelElementData>::iterator it = m_teleportElement.begin(); it != m_teleportElement.end(); ++it)
    {
        it->second.m_TileGamePosition.clear();
    }
    m_barrelElement.m_TileGamePosition.clear();
}

//===================================================================
bool LevelManager::loadSettingsData()
{
    INIReader reader(LEVEL_RESSOURCES_DIR_STR + "Saves/CustomSettings.ini");
    if(reader.ParseError() < 0)
    {
        return false;
    }
    //AUDIO
    m_settingsData.m_musicVolume = reader.GetInteger("Audio", "musicVolume", 100);
    m_settingsData.m_effectsVolume = reader.GetInteger("Audio", "effectsVolume", 100);
    //DISPLAY
    m_settingsData.m_fullscreen = reader.GetBoolean("Display", "fullscreen", false);
    m_settingsData.m_resolutionWidth = reader.GetInteger("Display", "resolutionWidth", 0);
    m_settingsData.m_resolutionHeight = reader.GetInteger("Display", "resolutionHeight", 0);
    //INPUT
    std::map<std::string, uint32_t>::const_iterator kbIt;
    std::map<std::string, GamepadInputState>::const_iterator gpIt;
    ControlKey_e currentKey;
    for(uint32_t i = 0; i < m_settingsData.m_arrayKeyboard.size(); ++i)
    {
        currentKey = static_cast<ControlKey_e>(i);
        //KEYBOARD
        kbIt = m_inputKeyboardKeyString.find(reader.Get("Keyboard", m_inputIDString[i], ""));
        if(kbIt != m_inputKeyboardKeyString.end())
        {
            m_settingsData.m_arrayKeyboard[i] = kbIt->second;
        }
        else
        {
            m_settingsData.m_arrayKeyboard[i] = MAP_KEYBOARD_DEFAULT_KEY.at(currentKey);
        }
        //GAMEPAD
        gpIt = m_inputGamepadKeyString.find(reader.Get("Gamepad", m_inputIDString[i], ""));
        if(gpIt != m_inputGamepadKeyString.end())
        {
            m_settingsData.m_arrayGamepad[i] = gpIt->second;
        }
        //default
        else
        {
            m_settingsData.m_arrayGamepad[i] = MAP_GAMEPAD_DEFAULT_KEY.at(currentKey);
        }
    }
    return true;
}

//===================================================================
void LevelManager::saveAudioSettings(uint32_t musicVolume, uint32_t effectVolume)
{
    m_outputStream.open(LEVEL_RESSOURCES_DIR_STR + "Saves/CustomSettings.ini");
    m_ini.setValue("Audio", "musicVolume", std::to_string(musicVolume));
    m_ini.setValue("Audio", "effectsVolume", std::to_string(effectVolume));
    m_ini.generate(m_outputStream);
    m_outputStream.close();
}

//===================================================================
void LevelManager::saveDisplaySettings(const pairI_t &resolution, bool fullscreen)
{
    m_outputStream.open(LEVEL_RESSOURCES_DIR_STR + "Saves/CustomSettings.ini");
    m_ini.setValue("Display", "resolutionWidth", std::to_string(resolution.first));
    m_ini.setValue("Display", "resolutionHeight", std::to_string(resolution.second));
    m_ini.setValue("Display", "fullscreen", fullscreen ? "true" : "false");
    m_ini.generate(m_outputStream);
    m_outputStream.close();
}

//===================================================================
void LevelManager::saveInputSettings(const std::map<ControlKey_e, GamepadInputState> &gamepadArray,
                                     const std::map<ControlKey_e, uint32_t> &keyboardArray)
{
    m_outputStream.open(LEVEL_RESSOURCES_DIR_STR + "Saves/CustomSettings.ini");
    uint32_t currentIndex;
    std::string valStr;
    for(std::map<ControlKey_e, GamepadInputState>::const_iterator it = gamepadArray.begin(); it != gamepadArray.end(); ++it)
    {
        currentIndex = static_cast<uint32_t>(it->first);
        if(it->second.m_standardButton)
        {
            valStr = m_inputGamepadSimpleButtons.at(it->second.m_keyID);
        }
        else
        {
            valStr = m_inputGamepadAxis.at(it->second.m_keyID);
            if(*it->second.m_axisPos)
            {
                valStr += '+';
            }
            else
            {
                valStr += '-';
            }
        }
        m_ini.setValue("Gamepad", m_inputIDString[currentIndex], valStr);
    }
    for(std::map<ControlKey_e, uint32_t>::const_iterator it = keyboardArray.begin(); it != keyboardArray.end(); ++it)
    {
        currentIndex = static_cast<uint32_t>(it->first);
        m_ini.setValue("Keyboard", m_inputIDString[currentIndex] , INPUT_KEYBOARD_KEY_STRING.at(it->second));
    }
    m_ini.generate(m_outputStream);
    m_outputStream.close();
}

//===================================================================
void LevelManager::saveGameProgress(const MemPlayerConf &playerConf, uint32_t levelNum, uint32_t numSaveFile)
{
    m_outputStream.open(LEVEL_RESSOURCES_DIR_STR + "Saves/save" + std::to_string(numSaveFile) + ".ini");
    m_ini.clear();
    m_ini.setValue("Level", "levelNum", std::to_string(levelNum));
    m_ini.setValue("Player", "life", std::to_string(playerConf.m_life));
    m_ini.setValue("Player", "previousWeapon", std::to_string(playerConf.m_previousWeapon));
    m_ini.setValue("Player", "currentWeapon", std::to_string(playerConf.m_currentWeapon));
    std::string weaponPosses, weaponAmmoCount;
    assert(playerConf.m_ammunationsCount.size() == playerConf.m_weapons.size());
    for(uint32_t i = 0; i < playerConf.m_ammunationsCount.size(); ++i)
    {
        weaponAmmoCount += std::to_string(playerConf.m_ammunationsCount[i]) + " ";
        weaponPosses += std::to_string(playerConf.m_weapons[i]) + " ";
    }
    m_ini.setValue("Player", "weaponPossess", weaponPosses);
    m_ini.setValue("Player", "weaponAmmoCount", weaponAmmoCount);
    m_ini.generate(m_outputStream);
    m_outputStream.close();
}

//===================================================================
std::optional<std::pair<uint32_t, MemPlayerConf>> LevelManager::loadSavedGame(uint32_t saveNum)
{
    INIReader reader(LEVEL_RESSOURCES_DIR_STR + "Saves/save" + std::to_string(saveNum) + ".ini");
    if(saveNum == 0 || reader.ParseError() < 0)
    {
        return {};
    }
    MemPlayerConf playerConf;
    std::string str;
    uint32_t levelNum = reader.GetInteger("Level", "levelNum", 0);
    if(levelNum == 0)
    {
        return {};
    }
    playerConf.m_life = reader.GetInteger("Player", "life", 100);
    playerConf.m_currentWeapon = reader.GetInteger("Player", "currentWeapon", 100);
    playerConf.m_previousWeapon = reader.GetInteger("Player", "previousWeapon", 100);
    playerConf.m_ammunationsCount = convertStrToVectUI(reader.Get("Player", "weaponAmmoCount", ""));
    playerConf.m_weapons = convertStrToVectBool(reader.Get("Player", "weaponPossess", ""));
    if(playerConf.m_ammunationsCount.size() != playerConf.m_weapons.size())
    {
        return {};
    }
    return std::pair<uint32_t, MemPlayerConf>{levelNum, playerConf};
}

//===================================================================
bool LevelManager::checkSavedGameExists(uint32_t saveNum)const
{
    return std::filesystem::exists(LEVEL_RESSOURCES_DIR_STR + "Saves/save" + std::to_string(saveNum) + ".ini");
}
