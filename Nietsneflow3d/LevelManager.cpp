#include <LevelManager.hpp>
#include <MainEngine.hpp>
#include <cassert>
#include <sstream>
#include <set>
#include <iostream>
#include <iterator>
#include <filesystem>
#include <ctime>

namespace fs = std::filesystem;

//===================================================================
LevelManager::LevelManager()
{
    std::ifstream inStream(LEVEL_RESSOURCES_DIR_STR + "Saves/CustomSettings.ini");
    m_ini.parse(inStream);
    inStream.close();
}

//===================================================================
void LevelManager::loadTexturePath()
{
    std::optional<std::string> val = m_ini.getValue("PathToTexture", "textures");
    assert(val);
    std::string textures = *val;
    assert(!textures.empty() && "Textures path cannot be loaded.");
    std::istringstream iss(textures);
    vectStr_t results(std::istream_iterator<std::string>{iss},
                      std::istream_iterator<std::string>());
    m_pictureData.setTexturePath(results);
}

//===================================================================
void LevelManager::loadSpriteData(const std::string &sectionName, bool font, std::optional<Font_e> type)
{
    std::vector<std::string> sections = m_ini.getSectionNamesContaining(sectionName);
    std::optional<std::string> val;
    for(uint32_t i = 0; i < sections.size(); ++i)
    {
        val = m_ini.getValue(sections[i], "texture");
        assert(val);
        uint32_t textureNum = std::stoi(*val);
        assert(textureNum != std::numeric_limits<uint16_t>::max() && "Bad textureNumber");
        val = m_ini.getValue(sections[i], "texturePosX");
        assert(val);
        double texturePosX = std::stof(*val);
        val = m_ini.getValue(sections[i], "texturePosY");
        assert(val);
        double texturePosY = std::stof(*val);
        val = m_ini.getValue(sections[i], "textureWeight");
        assert(val);
        double textureWeight = std::stof(*val);
        val = m_ini.getValue(sections[i], "textureHeight");
        assert(val);
        double textureHeight = std::stof(*val);
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
            assert(type);
            m_fontData.addCharSpriteData(spriteData, sections[i], *type);
        }
        else
        {
            m_pictureData.setSpriteData(spriteData, sections[i]);
        }
    }
}

//===================================================================
bool LevelManager::loadBackgroundData()
{
    std::optional<std::string> valA, valB, valC;
    std::vector<std::string> sections = m_ini.getSectionNamesContaining("GroundBackground");
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
            valA = m_ini.getValue(sections[i], "colorR");
            valB = m_ini.getValue(sections[i], "colorG");
            valC = m_ini.getValue(sections[i], "colorB");
            if(!valA || !valB || !valC)
            {
                std::cout << "Error while loading color background" << std::endl;
                return false;
            }
            colorR = convertStrToVectFloat(*valA);
            colorG = convertStrToVectFloat(*valB);
            colorB = convertStrToVectFloat(*valC);
            for(uint32_t j = 0; j < 4 ; ++j)
            {
                groundData.m_color[j] = tupleFloat_t{colorR[j], colorG[j], colorB[j]};
            }
            groundData.m_apparence[colorIndex] = true;
        }
        else if(sections[i] == "SimpleTextureGroundBackground")
        {
            valA = m_ini.getValue(sections[i], "sprite");
            if(!valA)
            {
                std::cout << "Error while loading background picture" << std::endl;
                return false;
            }
            std::optional<uint16_t> picNum = m_pictureData.getIdentifier(*valA);
            if(!picNum)
            {
                std::cout << "Error while loading background picture" << std::endl;
                return false;
            }
            groundData.m_spriteSimpleTextNum = *picNum;
            groundData.m_apparence[simpleTextureIndex] = true;
        }
        else if(sections[i] == "TiledTextureGroundBackground")
        {
            valA = m_ini.getValue(sections[i], "sprite");
            if(!valA)
            {
                std::cout << "Error while loading background picture" << std::endl;
                return false;
            }
            std::optional<uint16_t> picNum = m_pictureData.getIdentifier(*valA);
            if(!picNum)
            {
                std::cout << "Error while loading background picture" << std::endl;
                return false;
            }
            groundData.m_spriteTiledTextNum = *picNum;
            groundData.m_apparence[tiledTextureIndex] = true;
        }
    }
    sections = m_ini.getSectionNamesContaining("CeilingBackground");
    for(uint32_t i = 0; i < sections.size() ; ++i)
    {
        if(sections[i] == "ColorCeilingBackground")
        {
            std::vector<float> colorR, colorG, colorB;
            valA = m_ini.getValue(sections[i], "colorR");
            valB = m_ini.getValue(sections[i], "colorG");
            valC = m_ini.getValue(sections[i], "colorB");
            if(!valA || !valB || !valC)
            {
                std::cout << "Error while loading color background" << std::endl;
                return false;
            }
            colorR = convertStrToVectFloat(*valA);
            colorG = convertStrToVectFloat(*valB);
            colorB = convertStrToVectFloat(*valC);
            for(uint32_t j = 0; j < 4 ; ++j)
            {
                ceilingData.m_color[j] = tupleFloat_t{colorR[j], colorG[j], colorB[j]};
            }
            ceilingData.m_apparence[colorIndex] = true;
        }
        else if(sections[i] == "SimpleTextureCeilingBackground")
        {
            valA = m_ini.getValue(sections[i], "sprite");
            if(!valA)
            {
                std::cout << "Error while loading background picture" << std::endl;
                return false;
            }
            std::optional<uint16_t> picNum = m_pictureData.getIdentifier(*valA);
            if(!picNum)
            {
                std::cout << "Error while loading background picture" << std::endl;
                return false;
            }
            ceilingData.m_spriteSimpleTextNum = *picNum;
            ceilingData.m_apparence[simpleTextureIndex] = true;
        }
        else if(sections[i] == "TiledTextureCeilingBackground")
        {
            valA = m_ini.getValue(sections[i], "sprite");
            if(!valA)
            {
                std::cout << "Error while loading background picture" << std::endl;
                return false;
            }
            std::optional<uint16_t> picNum = m_pictureData.getIdentifier(*valA);
            if(!picNum)
            {
                std::cout << "Error while loading background picture" << std::endl;
                return false;
            }
            ceilingData.m_spriteTiledTextNum = *picNum;
            ceilingData.m_apparence[tiledTextureIndex] = true;
        }
    }
    m_pictureData.setBackgroundData(groundData, ceilingData);
    return true;
}

//===================================================================
void LevelManager::loadMusicData()
{
    std::optional<std::string> val = m_ini.getValue("Level", "music");
    if(val)
    {
        m_level.setMusicFile(*val);
    }
}

//===================================================================
bool LevelManager::loadLevelData()
{
    std::optional<std::string> valWeight = m_ini.getValue("Level", "weight");
    std::optional<std::string> valHeight = m_ini.getValue("Level", "height");
    if(!valWeight || !valHeight)
    {
        return false;
    }
    m_level.setLevelSize({std::stoi(*valWeight), std::stoi(*valHeight)});
    return true;
}

//===================================================================
bool LevelManager::loadPositionPlayerData(uint32_t levelNum, bool customLevel)
{
    std::optional<std::string> playerDepartureX = m_ini.getValue("PlayerInit", "playerDepartureX");
    std::optional<std::string> playerDepartureY = m_ini.getValue("PlayerInit", "playerDepartureY");
    std::optional<std::string> PlayerOrientation = m_ini.getValue("PlayerInit", "PlayerOrientation");
    std::optional<std::string> levelNumIni = m_ini.getValue("PlayerInit", "levelNum");
    if(!playerDepartureX || !playerDepartureY || !PlayerOrientation || (!customLevel && !levelNum))
    {
        return false;
    }
    if(!customLevel && std::stoul(*levelNumIni) != levelNum)
    {
        std::cout << "Bad level number\n";
        return false;
    }
    m_level.setPlayerInitData({std::stoi(*playerDepartureX), std::stoi(*playerDepartureY)},
                              static_cast<Direction_e>(std::stoi(*PlayerOrientation)));
    return true;
}

//===================================================================
void LevelManager::loadGeneralStaticElements(LevelStaticElementType_e elementType)
{
    std::map<std::string, StaticLevelElementData> *memMap;
    std::vector<std::string> vectINISections;
    switch(elementType)
    {
    case LevelStaticElementType_e::GROUND:
        vectINISections = m_ini.getSectionNamesContaining("Ground");
        memMap = &m_groundElement;
        break;
    case LevelStaticElementType_e::CEILING:
        vectINISections = m_ini.getSectionNamesContaining("Ceiling");
        memMap = &m_ceilingElement;
        break;
    case LevelStaticElementType_e::OBJECT:
        vectINISections = m_ini.getSectionNamesContaining("Object");
        memMap = &m_objectElement;
        break;
    case LevelStaticElementType_e::TELEPORT:
        vectINISections = m_ini.getSectionNamesContaining("Teleport");
        memMap = &m_teleportElement;
        break;
    case LevelStaticElementType_e::IMPACT:
        break;
    }
    for(uint32_t i = 0; i < vectINISections.size(); ++i)
    {
        memMap->insert({vectINISections[i], StaticLevelElementData()});
        readStandardStaticElement(memMap->operator[](vectINISections[i]), vectINISections[i], elementType);
    }
}

//===================================================================
void LevelManager::loadBarrelsData()
{
    std::vector<std::string> vectINISections;
    vectINISections = m_ini.getSectionNamesContaining("Barrel");
    assert(!vectINISections.empty());
    //Static
    std::optional<std::string> val;
    m_barrelElement.m_staticSprite = getVectSpriteNum(vectINISections[0], "StaticSprite");
    val = m_ini.getValue(vectINISections[0], "StaticSpriteWeightGame");
    assert(val);
    m_barrelElement.m_inGameStaticSpriteSize.first = std::stof(*val);
    val = m_ini.getValue(vectINISections[0], "StaticSpriteHeightGame");
    assert(val);
    m_barrelElement.m_inGameStaticSpriteSize.second = std::stof(*val);
    //Explosion
    m_barrelElement.m_explosionSprite = getVectSpriteNum(vectINISections[0], "ExplosionSprite");
    m_barrelElement.m_vectinGameExplosionSpriteSize = getVectSpriteGLSize(vectINISections[0],
            "ExplosionSpriteWeightGame", "ExplosionSpriteHeightGame");
    val = m_ini.getValue(vectINISections[0], "ExplosionSound");
    assert(val);
    m_barrelElement.m_explosionSoundFile = *val;
    assert(m_barrelElement.m_vectinGameExplosionSpriteSize.size() == m_barrelElement.m_explosionSprite.size());
}

//===================================================================
void LevelManager::loadLogData()
{
    std::vector<std::string> vectINISections;
    vectINISections = m_ini.getSectionNamesContaining("MessageLog");
    assert(!vectINISections.empty());
    std::optional<std::string> val;
    std::string sprite;
    PairFloat_t size;
    for(uint32_t i = 0; i < vectINISections.size(); ++i)
    {
        val = m_ini.getValue(vectINISections[i], "Sprite");
        assert(val);
        std::optional<uint16_t> spritenum = *m_pictureData.getIdentifier(*val);
        val = m_ini.getValue(vectINISections[i], "SpriteWeightGame");
        assert(val);
        size.first = std::stof(*val);
        val = m_ini.getValue(vectINISections[i], "SpriteHeightGame");
        assert(val);
        size.second = std::stof(*val);
        assert(spritenum);
        m_logStdData.insert({vectINISections[i], {*spritenum, size}});
    }
}

//===================================================================
std::vector<uint16_t> LevelManager::getVectSpriteNum(const std::string_view section, const std::string_view param)
{
    std::optional<std::string> val = m_ini.getValue(section.data(), param.data());
    assert(val);
    std::string str = *val;
    assert(!str.empty());
    vectStr_t vectSprite = convertStrToVectStr(str);
    std::vector<uint16_t> retVect;
    retVect.reserve(vectSprite.size());
    for(uint32_t i = 0; i < vectSprite.size(); ++i)
    {
        std::optional<uint16_t> id = m_pictureData.getIdentifier(vectSprite[i]);
        assert(id);
        retVect.emplace_back(*id);
    }
    return retVect;
}

//===================================================================
std::vector<PairFloat_t> LevelManager::getVectSpriteGLSize(const std::string_view section,
                                                           const std::string_view weightParam, const std::string_view heightParam)
{
    std::vector<PairFloat_t> retVect;
    std::optional<std::string> val = m_ini.getValue(section.data(), weightParam.data());
    assert(val);
    std::string str = *val;
    assert(!str.empty());
    std::vector<float> vectWeight = convertStrToVectFloat(str);
    val = m_ini.getValue(section.data(), heightParam.data());
    assert(val);
    str = *val;
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
std::optional<std::vector<uint32_t>> LevelManager::getBrutPositionData(const std::string &sectionName,
                                                                       const std::string &propertyName)
{
    std::optional<std::string> pos = m_ini.getValue(sectionName, propertyName);
    if(!pos || (*pos).empty())
    {
        return {};
    }
    return convertStrToVectUI(*pos);
}

//===================================================================
VectPairUI_t LevelManager::getPositionData(const std::string &sectionName, const std::string &propertyName)
{
    VectPairUI_t vectRet;
    std::optional<std::vector<uint32_t>> resultsPos = getBrutPositionData(sectionName.data(), propertyName.data());
    if(!resultsPos)
    {
        return {};
    }
    if((*resultsPos).size() % 2 == 1)
    {
        std::cout << "Warning : inconsistent position datas in " << sectionName << std::endl;
        return {};
    }
    vectRet.reserve((*resultsPos).size() / 2);
    for(uint32_t i = 0; i < (*resultsPos).size(); i += 2)
    {
        vectRet.emplace_back(PairUI_t{(*resultsPos)[i], (*resultsPos)[i + 1]});
    }
    return vectRet;
}

//===================================================================
void LevelManager::loadExit()
{
    std::vector<std::string> vectINISections;
    vectINISections = m_ini.getSectionNamesContaining("Exit");
    assert(!vectINISections.empty());
    loadSpriteData(vectINISections[0], m_exitStaticElement);
}

//===================================================================
void LevelManager::loadVisualTeleportData()
{
    std::vector<std::string> vectINISections;
    vectINISections = m_ini.getSectionNamesContaining("TelepAnim");
    assert(!vectINISections.empty());
    std::optional<std::string> val = m_ini.getValue(vectINISections[0], "Sprite");
    assert(val);
    std::string str = *val;
    assert(!str.empty());
    vectStr_t vectSprite = convertStrToVectStr(str);
    val = m_ini.getValue(vectINISections[0], "SpriteWeightGame");
    assert(val);
    str = *val;
    assert(!str.empty());
    std::vector<float> vectWeight = convertStrToVectFloat(str);
    val = m_ini.getValue(vectINISections[0], "SpriteHeightGame");
    assert(val);
    str = *val;
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
void LevelManager::loadTriggerElements()
{
    std::vector<std::string> vectINISections;
    vectINISections = m_ini.getSectionNamesContaining("Trigger");
    if(vectINISections.empty())
    {
        return;
    }
    std::string spriteID;
    std::optional<std::string> val;
    for(uint32_t i = 0; i < vectINISections.size(); ++i)
    {
        val = m_ini.getValue(vectINISections[i], "Sprite");
        assert(val);
        spriteID = *val;
        assert(!spriteID.empty());
        m_triggerDisplayData.insert({vectINISections[i], MemSpriteData()});
        m_triggerDisplayData[vectINISections[i]].m_numSprite = *m_pictureData.getIdentifier(spriteID);
        val = m_ini.getValue(vectINISections[i], "SpriteWeightGame");
        assert(val);
        m_triggerDisplayData[vectINISections[i]].m_GLSize.first = std::stof(*val);
        val = m_ini.getValue(vectINISections[i], "SpriteHeightGame");
        assert(val);
        m_triggerDisplayData[vectINISections[i]].m_GLSize.second = std::stof(*val);
    }
}

//===================================================================
bool LevelManager::loadPositionExit()
{
    std::vector<std::string> vectINISections;
    vectINISections = m_ini.getSectionNamesContaining("Exit");
    if(vectINISections.empty())
    {
        return false;
    }
    std::optional<std::string> gamePositions = m_ini.getValue(vectINISections[0], "GamePosition");
    if(!gamePositions || (*gamePositions).empty())
    {
        return false;
    }
    std::vector<uint32_t> results = convertStrToVectUI(*gamePositions);
    for(uint32_t i = 0; i < results.size(); i += 2)
    {
        m_exitStaticElement.m_TileGamePosition.push_back({results[i], results[i + 1]});
        deleteWall(m_exitStaticElement.m_TileGamePosition.back());
    }
    return true;
}

//===================================================================
void LevelManager::loadSpriteData(const std::string &sectionName,
                                  StaticLevelElementData &staticElement)
{
    staticElement.m_numSprite = getSpriteId(sectionName);
    std::optional<std::string> val;
    val = m_ini.getValue(sectionName, "SpriteWeightGame");
    assert(val);
    staticElement.m_inGameSpriteSize.first = std::stof(*val);
    val = m_ini.getValue(sectionName, "SpriteHeightGame");
    assert(val);
    staticElement.m_inGameSpriteSize.second = std::stof(*val);
}

//===================================================================
void LevelManager::readStandardStaticElement(StaticLevelElementData &staticElement,
                                             const std::string &sectionName, LevelStaticElementType_e elementType)
{
    loadSpriteData(sectionName, staticElement);
    std::optional<std::string> val;
    if(elementType == LevelStaticElementType_e::OBJECT)
    {
        val = m_ini.getValue(sectionName, "Type");
        assert(val);
        uint32_t type = std::stof(*val);
        assert(type < static_cast<uint32_t>(ObjectType_e::TOTAL));
        staticElement.m_type = static_cast<ObjectType_e>(type);
        if(staticElement.m_type == ObjectType_e::AMMO_WEAPON || staticElement.m_type == ObjectType_e::WEAPON ||
                staticElement.m_type == ObjectType_e::HEAL)
        {
            val = m_ini.getValue(sectionName, "Containing");
            assert(val);
            staticElement.m_containing = std::stoi(*val);
        }
        if(staticElement.m_type == ObjectType_e::AMMO_WEAPON || staticElement.m_type == ObjectType_e::WEAPON)
        {
            val = m_ini.getValue(sectionName, "WeaponID");
            assert(val);
            std::map<std::string, uint32_t>::const_iterator it = m_weaponINIAssociated.find(*val);
            assert(it != m_weaponINIAssociated.end());
            staticElement.m_weaponID = it->second;
        }
        else if(staticElement.m_type == ObjectType_e::CARD)
        {
            val = m_ini.getValue(sectionName, "CardID");
            assert(val);
            staticElement.m_cardID = std::stoi(*val);
            val = m_ini.getValue(sectionName, "CardName");
            assert(val);
            staticElement.m_cardName = *val;
            m_cardINIAssociated.insert({sectionName, *staticElement.m_cardID});
        }
    }
    if(elementType != LevelStaticElementType_e::OBJECT)
    {
        val = m_ini.getValue(sectionName, "traversable");
        assert(val);
        //oOOOOK BOOL
        std::optional<bool> res = toBool(*val);
        assert(res);
        staticElement.m_traversable = *res;
    }
}


//===================================================================
bool LevelManager::fillStandartPositionVect(const std::string &sectionName, VectPairUI_t &vectPos)
{
    std::optional<std::vector<uint32_t>> results = getBrutPositionData(sectionName, "GamePosition");
    if(!results)
    {
        return true;
    }
    if((*results).empty() || (*results).size() % 2 == 1)
    {
        std::cout << "Warning : inconsistent position datas in " << sectionName << std::endl;
        return false;
    }
    size_t finalSize = (*results).size() / 2;
    vectPos.reserve(finalSize);
    for(uint32_t j = 0; j < (*results).size(); j += 2)
    {
        vectPos.emplace_back(PairUI_t{(*results)[j], (*results)[j + 1]});
        deleteWall(vectPos.back());
    }
    return true;
}

//===================================================================
bool LevelManager::fillTeleportPositions(const std::string &sectionName)
{
    std::optional<std::vector<uint32_t>> resultsPosA = getBrutPositionData(sectionName, "PosA"),
            resultsPosB = getBrutPositionData(sectionName, "PosB");
    if(!resultsPosA || !resultsPosB || (*resultsPosA).size() % 2 == 1 || (*resultsPosB).size() % 2 == 1 ||
            (*resultsPosB).size() != (*resultsPosA).size())
    {
        std::cout << "Warning : inconsistent position datas in " << sectionName << std::endl;
        return false;
    }
    uint32_t vectSize = (*resultsPosA).size() / 2;
    m_teleportElement[sectionName].m_teleportData = TeleportData();
    std::optional<std::string> val = m_ini.getValue(sectionName, "BiDirection");
    if(!val)
    {
        std::cout << "Warning : bidirection datas in " << sectionName << std::endl;
        return false;
    }
    m_teleportElement[sectionName].m_teleportData->m_biDirection = convertStrToVectBool(*val);
    if(m_teleportElement[sectionName].m_teleportData->m_biDirection.size() != vectSize)
    {
        std::cout << "Warning : bidirection datas in " << sectionName << std::endl;
        return false;
    }
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
    return true;
}

//===================================================================
std::map<std::string, StaticLevelElementData>::iterator LevelManager::removeStaticElement(const std::string_view sectionName,
                                                                                          LevelStaticElementType_e elementType)
{
    std::map<std::string, StaticLevelElementData> *container;
    switch(elementType)
    {
    case LevelStaticElementType_e::GROUND:
        container = &m_groundElement;
        break;
    case LevelStaticElementType_e::CEILING:
        container = &m_ceilingElement;
        break;
    case LevelStaticElementType_e::OBJECT:
        container = &m_objectElement;
        break;
    case LevelStaticElementType_e::TELEPORT:
        container = &m_teleportElement;
        break;
    case LevelStaticElementType_e::IMPACT:
        assert(false);
        break;
    }
    std::map<std::string, StaticLevelElementData>::iterator it = container->find(sectionName.data());
    assert(it != container->end());
    return container->erase(it);
}

//===================================================================
std::optional<PairUI_t> LevelManager::getPosition(const std::string_view sectionName,
                                                  const std::string_view propertyName)
{
    std::optional<std::vector<uint32_t>> results = getBrutPositionData(sectionName.data(), propertyName.data());
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
bool LevelManager::fillWallPositionVect(const std::string &sectionName,
                                        const std::string &propertyName,
                                        std::set<PairUI_t> &vectPos)
{
    std::optional<std::vector<uint32_t>> results = getBrutPositionData(sectionName, propertyName);
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
uint16_t LevelManager::getSpriteId(const std::string &sectionName)
{
    std::optional<std::string> val = m_ini.getValue(sectionName, "Sprite");
    assert(val);
    std::optional<uint16_t> id = m_pictureData.getIdentifier(*val);
    assert(id && "picture data does not exists.");
    return *id;
}

//===================================================================
void LevelManager::loadVisibleShotDisplayData()
{
    std::vector<std::string> vectINISections = m_ini.getSectionNamesContaining("VisibleShoot");
    if(vectINISections.empty())
    {
        return;
    }
    std::string sprites, weight, height, impactSound;
    vectStr_t vectSprites;
    std::vector<float> vectWeight, vectHeight;
    std::optional<std::string> val;
    for(uint32_t i = 0; i < vectINISections.size(); ++i)
    {
        val = m_ini.getValue(vectINISections[i], "Sprites");
        assert(val);
        sprites = *val;
        val = m_ini.getValue(vectINISections[i], "SpriteWeightGame");
        assert(val);
        weight = *val;
        val = m_ini.getValue(vectINISections[i], "SpriteHeightGame");
        assert(val);
        height = *val;
        vectWeight = convertStrToVectFloat(weight);
        vectHeight = convertStrToVectFloat(height);
        vectSprites = convertStrToVectStr(sprites);
        assert(vectWeight.size() == vectHeight.size());
        assert(vectSprites.size() == vectHeight.size());
        val = m_ini.getValue(vectINISections[i], "ImpactSound");
        assert(val);
        impactSound = *val;
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
void LevelManager::loadShotImpactDisplayData()
{
    std::vector<std::string> vectINISections = m_ini.getSectionNamesContaining("Impact");
    if(vectINISections.empty())
    {
        return;
    }
    std::string spritesStd, spritesImpact;
    float weight, height;
    vectStr_t vectSprites;
    std::optional<std::string> val;
    for(uint32_t i = 0; i < vectINISections.size(); ++i)
    {
        val = m_ini.getValue(vectINISections[i], "SpritesStd");
        assert(val);
        spritesStd = *val;
        val = m_ini.getValue(vectINISections[i], "SpriteTouched");
        assert(val);
        spritesImpact = *val;
        assert(!spritesStd.empty());
        val = m_ini.getValue(vectINISections[i], "SpriteWeightGame");
        assert(val);
        weight = std::stof(*val);
        val = m_ini.getValue(vectINISections[i], "SpriteHeightGame");
        assert(val);
        height = std::stof(*val);
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
void LevelManager::loadWeaponsData()
{
    std::vector<std::string> vectINISections;
    vectINISections = m_ini.getSectionNamesContaining("Weapon");
    assert(!vectINISections.empty());
    m_vectWeaponsINIData.resize(vectINISections.size());
    for(uint32_t i = 0; i < vectINISections.size(); ++i)
    {
        loadWeaponData(vectINISections[i], i);
        loadDisplayData(vectINISections[i], "ShotImpact");
    }
}

//===================================================================
void LevelManager::loadGeneralSoundData()
{
    std::optional<std::string> val = m_ini.getValue("SoundUtils", "PickUpObjectSound");
    assert(val);
    m_pickObjectSound = *val;
    assert(!m_pickObjectSound.empty());
    val = m_ini.getValue("SoundUtils", "DoorOpeningSound");
    assert(val);
    m_doorOpeningSound = *val;
    assert(!m_doorOpeningSound.empty());
    val = m_ini.getValue("SoundUtils", "HitSound");
    assert(val);
    m_hitSound = *val;
    assert(!m_hitSound.empty());
    val = m_ini.getValue("SoundUtils", "TeleportSound");
    assert(val);
    m_teleportSound = *val;
    assert(!m_teleportSound.empty());
    val = m_ini.getValue("SoundUtils", "PlayerDeathSound");
    assert(val);
    m_playerDeathSound = *val;
    assert(!m_playerDeathSound.empty());
    val = m_ini.getValue("SoundUtils", "TriggerSound");
    assert(val);
    m_triggerSound = *val;
    assert(!m_triggerSound.empty());
}

//===================================================================
void LevelManager::loadDisplayData(std::string_view sectionName,
                                   std::string_view subSectionName)
{
    std::optional<std::string> val = m_ini.getValue(sectionName.data(), subSectionName.data());
    if(!val || (*val).empty())
    {
        return;
    }
    std::string sprites = *val;
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
void LevelManager::loadWeaponData(std::string_view sectionName, uint32_t numIt)
{
    std::string resultWeight, resultHeight;
    std::string sprites, str;
    std::optional<std::string> val = m_ini.getValue(sectionName.data(), "MaxAmmo");
    assert(val);
    m_vectWeaponsINIData[numIt].m_maxAmmo = std::stoi(*val);
    val = m_ini.getValue(sectionName.data(), "NumberOfShots");
    if(val)
    {
        m_vectWeaponsINIData[numIt].m_simultaneousShots = std::stoi(*val);
    }
    else
    {
        m_vectWeaponsINIData[numIt].m_simultaneousShots = 1;
    }
    val = m_ini.getValue(sectionName.data(), "AnimationType");
    assert(val);
    m_vectWeaponsINIData[numIt].m_animMode = static_cast<AnimationMode_e>(std::stoi(*val));
    val = m_ini.getValue(sectionName.data(), "Order");
    assert(val);
    m_vectWeaponsINIData[numIt].m_order = std::stoi(*val);
    val = m_ini.getValue(sectionName.data(), "LastAnimNum");
    if(val)
    {
        m_vectWeaponsINIData[numIt].m_lastAnimNum = std::stoi(*val);
    }
    val = m_ini.getValue(sectionName.data(), "NumberOfShots");
    if(val)
    {
        m_vectWeaponsINIData[numIt].m_simultaneousShots = std::stoi(*val);
    }
    val = m_ini.getValue(sectionName.data(), "Possess");
    if(val)
    {
        std::optional<bool> res = toBool(*val);
        assert(res);
        m_vectWeaponsINIData[numIt].m_startingPossess = *res;
    }
    else
    {
        m_vectWeaponsINIData[numIt].m_startingPossess = false;
    }
    val = m_ini.getValue(sectionName.data(), "BaseAmmo");
    if(val)
    {
        m_vectWeaponsINIData[numIt].m_startingAmmoCount = std::stoi(*val);
    }
    val = m_ini.getValue(sectionName.data(), "Damage");
    assert(val);
    m_vectWeaponsINIData[numIt].m_damage = std::stoi(*val);
    val = m_ini.getValue(sectionName.data(), "WeaponName");
    assert(val);
    m_vectWeaponsINIData[numIt].m_weaponName = *val;
    val = m_ini.getValue(sectionName.data(), "ShotImpactID");
    if(val)
    {
        m_vectWeaponsINIData[numIt].m_impactID = *val;
    }
    val = m_ini.getValue(sectionName.data(), "ShotVelocity");
    if(val)
    {
        m_vectWeaponsINIData[numIt].m_shotVelocity = std::stof(*val);
    }
    val = m_ini.getValue(sectionName.data(), "AttackType");
    assert(val);
    m_vectWeaponsINIData[numIt].m_attackType = static_cast<AttackType_e>(std::stoi(*val));
    val = m_ini.getValue(sectionName.data(), "SoundEffect");
    if(val)
    {
        m_vectWeaponsINIData[numIt].m_shotSound = *val;
    }
    val = m_ini.getValue(sectionName.data(), "ReloadSound");
    if(val)
    {
        m_vectWeaponsINIData[numIt].m_reloadSound = *val;
    }
    if(m_vectWeaponsINIData[numIt].m_attackType == AttackType_e::VISIBLE_SHOTS)
    {
        val = m_ini.getValue(sectionName.data(), "ShootSpritesID");
        assert(val);
        m_vectWeaponsINIData[numIt].m_visibleShootID = *val;
        assert(!m_vectWeaponsINIData[numIt].m_visibleShootID.empty());
    }
    m_weaponINIAssociated.insert({sectionName.data(), m_vectWeaponsINIData[numIt].m_order});
    val = m_ini.getValue(sectionName.data(), "StaticSprite");
    assert(val);
    sprites = *val;
    sprites += " ";
    val = m_ini.getValue(sectionName.data(), "AttackSprite");
    assert(val);
    sprites += *val;
    assert(!sprites.empty() && "Wall sprites cannot be loaded.");
    vectStr_t vectSprites = convertStrToVectStr(sprites);
    val = m_ini.getValue(sectionName.data(), "SpriteWeightGame");
    assert(val);
    resultWeight = *val;
    val = m_ini.getValue(sectionName.data(), "SpriteHeightGame");
    assert(val);
    resultHeight = *val;
    std::vector<float> vectWeight = convertStrToVectFloat(resultWeight),
            vectHeight = convertStrToVectFloat(resultHeight);
    assert(vectHeight.size() == vectWeight.size());
    assert(vectSprites.size() == vectWeight.size());
    m_vectWeaponsINIData[numIt].m_spritesData.reserve(vectSprites.size());
    val = m_ini.getValue(sectionName.data(), "AnimationLatency");
    assert(val);
    m_vectWeaponsINIData[numIt].m_animationLatency = std::stof(*val);
    assert(m_vectWeaponsINIData[numIt].m_animationLatency > 0.00f);
    val = m_ini.getValue(sectionName.data(), "DamageZoneRay");
    if(val)
    {
        m_vectWeaponsINIData[numIt].m_damageCircleRay = std::stof(*val);
    }
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
void LevelManager::loadWallData()
{
    std::vector<std::string> vectINISections = m_ini.getSectionNamesContaining("Wall");
    std::optional<std::string> datas;
    std::vector<float> time;
    for(uint32_t i = 0; i < vectINISections.size(); ++i)
    {
        m_wallData.insert({vectINISections[i], WallData()});
        datas = m_ini.getValue(vectINISections[i], "Sprite");
        assert(!datas || (!(*datas).empty() && "Wall sprites cannot be loaded."));
        vectStr_t results = convertStrToVectStr(*datas);
        m_wallData[vectINISections[i]].m_sprites.reserve(results.size());
        //load sprites
        for(uint32_t j = 0; j < results.size(); ++j)
        {
            std::optional<uint16_t> res = m_pictureData.getIdentifier(results[j]);
            assert(res);
            m_wallData[vectINISections[i]].m_sprites.emplace_back(*res);
        }
        //Time data
        datas = m_ini.getValue(vectINISections[i], "Time");
        if(datas)
        {
            time = convertStrToVectFloat(*datas);
            assert(results.size() == time.size());
            m_wallData[vectINISections[i]].m_cyclesTime.reserve(time.size());
            for(uint32_t j = 0; j < time.size(); ++j)
            {
                m_wallData[vectINISections[i]].m_cyclesTime.emplace_back(time[j] / FPS_VALUE);
            }
        }
    }
}

//===================================================================
void LevelManager::loadPositionWall()
{
    std::vector<std::string> vectINISections = m_ini.getSectionNamesContaining("WallShape");
    std::map<std::string, WallData>::iterator it;
    std::optional<std::string> direction, moveNumber, velocity, triggerType, triggerBehaviourType, wallDisplayID;
    for(uint32_t i = 0; i < vectINISections.size(); ++i)
    {
        wallDisplayID = m_ini.getValue(vectINISections[i], "WallDisplayID");
        if(!wallDisplayID)
        {
            std::cout << "WARNING errors in " << vectINISections[i] << " datas skip\n";
            continue;
        }
        it = m_wallData.find(*wallDisplayID);
        if(it == m_wallData.end())
        {
            std::cout << "WARNING errors in " << vectINISections[i] << " datas skip\n";
            continue;
        }
        //Moveable wall
        m_mainWallData.insert({vectINISections[i], MoveableWallData()});
        m_mainWallData[vectINISections[i]].m_sprites = it->second.m_sprites;
        m_mainWallData[vectINISections[i]].m_cyclesTime = it->second.m_cyclesTime;
        fillWallPositionVect(vectINISections[i], "GamePosition",
                             m_mainWallData[vectINISections[i]].m_TileGamePosition);
        fillWallPositionVect(vectINISections[i], "RemovePosition",
                             m_mainWallData[vectINISections[i]].m_removeGamePosition);
        direction = m_ini.getValue(vectINISections[i], "Direction");
        moveNumber = m_ini.getValue(vectINISections[i], "NumberOfMove");
        if(!direction || !moveNumber)
        {
            continue;
        }
        std::vector<uint32_t> vectDir = convertStrToVectUI(*direction);
        std::vector<uint32_t> vectMov = convertStrToVectUI(*moveNumber);
        velocity = m_ini.getValue(vectINISections[i], "Velocity");
        triggerBehaviourType = m_ini.getValue(vectINISections[i], "TriggerBehaviourType");
        if(!triggerBehaviourType || !velocity || vectDir.size() != vectMov.size())
        {
            std::cout << "WARNING errors in " << vectINISections[i] << " moveable wall datas skip\n";
            continue;
        }
        m_mainWallData[vectINISections[i]].m_directionMove.reserve(vectDir.size());
        for(uint32_t j = 0; j < vectDir.size(); ++j)
        {
            m_mainWallData[vectINISections[i]].m_directionMove.emplace_back(
                        std::pair<Direction_e, uint32_t>{static_cast<Direction_e>(vectDir[j]), vectMov[j]});
        }
        m_mainWallData[vectINISections[i]].m_velocity = std::stof(*velocity);
        triggerType = m_ini.getValue(vectINISections[i], "TriggerType");
        if(triggerType)
        {
            m_mainWallData[vectINISections[i]].m_triggerType =
                    static_cast<TriggerWallMoveType_e>(std::stoi(*triggerType));
        }
        m_mainWallData[vectINISections[i]].m_triggerBehaviourType =
                static_cast<TriggerBehaviourType_e>(std::stoi(*triggerBehaviourType));
        if(m_mainWallData[vectINISections[i]].m_triggerType == TriggerWallMoveType_e::BUTTON)
        {
            loadTriggerLevelData(vectINISections[i]);
        }
        else if(m_mainWallData[vectINISections[i]].m_triggerType == TriggerWallMoveType_e::GROUND)
        {
            m_mainWallData[vectINISections[i]].m_groundTriggerPos =
                    *getPosition(vectINISections[i], "TriggerGamePosition");
        }
    }
}

//===================================================================
void LevelManager::loadTriggerLevelData(const std::string &sectionName)
{
    std::optional<std::string> str;
    std::map<std::string, MemSpriteData>::iterator it;
    m_mainWallData[sectionName].m_associatedTriggerData = AssociatedTriggerData();
    str = m_ini.getValue(sectionName, "TriggerDisplayID");
    assert(str);
    it = m_triggerDisplayData.find(*str);
    assert(it != m_triggerDisplayData.end());
    m_mainWallData[sectionName].m_associatedTriggerData->m_displayData = it->second;
    m_mainWallData[sectionName].m_associatedTriggerData->m_pos =
            *getPosition(sectionName, "TriggerGamePosition");
}

//===================================================================
void LevelManager::loadDoorData()
{
    std::vector<std::string> vectINISections;
    vectINISections = m_ini.getSectionNamesContaining("Door");
    std::optional<std::string> val;
    for(uint32_t i = 0; i < vectINISections.size(); ++i)
    {
        m_doorData.insert({vectINISections[i], DoorData()});
        m_doorData[vectINISections[i]].m_numSprite = getSpriteId(vectINISections[i]);
        val = m_ini.getValue(vectINISections[i], "Vertical");
        assert(val);
        std::optional<bool> res = toBool(*val);
        assert(res);
        m_doorData[vectINISections[i]].m_vertical = *res;
        val = m_ini.getValue(vectINISections[i], "CardID");
        if(val || (*val).empty())
        {
            std::map<std::string, uint32_t>::const_iterator it = m_cardINIAssociated.find(*val);
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
void LevelManager::loadPositionDoorData()
{
    std::vector<std::string> vectINISections;
    vectINISections = m_ini.getSectionNamesContaining("Door");
    std::map<std::string, DoorData>::iterator it;
    for(uint32_t i = 0; i < vectINISections.size(); ++i)
    {
        it = m_doorData.find(vectINISections[i]);
        if(it == m_doorData.end())
        {
            std::cout << "Warning : door data id cannot be found" << std::endl;
            continue;
        }
        fillStandartPositionVect(vectINISections[i], it->second.m_TileGamePosition);
    }
}

//===================================================================
void LevelManager::loadEnemyData()
{
    std::vector<std::string> vectINISections;
    vectINISections = m_ini.getSectionNamesContaining("Enemy");
    std::string str;
    std::optional<std::string> val;
    std::optional<bool> resBool;
    for(uint32_t i = 0; i < vectINISections.size(); ++i)
    {
        m_enemyData.insert({vectINISections[i], EnemyData()});
        val = m_ini.getValue(vectINISections[i], "traversable");
        assert(val);
        resBool = toBool(*val);
        assert(resBool);
        m_enemyData[vectINISections[i]].m_traversable = *resBool;
        val = m_ini.getValue(vectINISections[i], "SpriteWeightGame");
        assert(val);
        m_enemyData[vectINISections[i]].m_inGameSpriteSize.first = std::stof(*val);
        val = m_ini.getValue(vectINISections[i], "SpriteHeightGame");
        assert(val);
        m_enemyData[vectINISections[i]].m_inGameSpriteSize.second = std::stof(*val);
        val = m_ini.getValue(vectINISections[i], "ShootSpritesID");
        if(val)
        {
            m_enemyData[vectINISections[i]].m_visibleShootID = *val;
        }
        val = m_ini.getValue(vectINISections[i], "ShotImpactID");
        if(val)
        {
            m_enemyData[vectINISections[i]].m_impactID = *val;
        }
        val = m_ini.getValue(vectINISections[i], "Damage");
        assert(val);
        m_enemyData[vectINISections[i]].m_attackPower = std::stoi(*val);
        val = m_ini.getValue(vectINISections[i], "Life");
        assert(val);
        m_enemyData[vectINISections[i]].m_life = std::stoi(*val);
        val = m_ini.getValue(vectINISections[i], "Velocity");
        assert(val);
        m_enemyData[vectINISections[i]].m_velocity = std::stof(*val);
        val = m_ini.getValue(vectINISections[i], "ShotVelocity");
        if(val)
        {
            m_enemyData[vectINISections[i]].m_shotVelocity = std::stof(*val);
        }
        val = m_ini.getValue(vectINISections[i], "DropedObjectID");
        if(val)
        {
            m_enemyData[vectINISections[i]].m_dropedObjectID = *val;
        }
        val = m_ini.getValue(vectINISections[i], "MeleeDamage");
        if(val)
        {
            m_enemyData[vectINISections[i]].m_meleeDamage = std::stoi(*val);
        }
        val = m_ini.getValue(vectINISections[i], "NormalBehaviourSound");
        assert(val);
        m_enemyData[vectINISections[i]].m_normalBehaviourSoundFile = *val;
        val = m_ini.getValue(vectINISections[i], "DetectBehaviourSound");
        assert(val);
        m_enemyData[vectINISections[i]].m_detectBehaviourSoundFile = *val;
        val = m_ini.getValue(vectINISections[i], "AttackSound");
        assert(val);
        m_enemyData[vectINISections[i]].m_attackSoundFile = *val;

        val = m_ini.getValue(vectINISections[i], "FrozenOnAttack");
        assert(val);
        std::optional<bool> resBool = toBool(*val);
        assert(resBool);
        m_enemyData[vectINISections[i]].m_frozenOnAttack = *resBool;
        val = m_ini.getValue(vectINISections[i], "MeleeOnly");
        if(val)
        {
            std::optional<bool> resBool = toBool(*val);
            assert(resBool);
            m_enemyData[vectINISections[i]].m_meleeOnly = *resBool;
        }
        else
        {
            m_enemyData[vectINISections[i]].m_meleeOnly = false;
        }
        loadEnemySprites(vectINISections[i], EnemySpriteElementType_e::STATIC_FRONT, m_enemyData[vectINISections[i]]);
        loadEnemySprites(vectINISections[i], EnemySpriteElementType_e::STATIC_FRONT_LEFT, m_enemyData[vectINISections[i]]);
        loadEnemySprites(vectINISections[i], EnemySpriteElementType_e::STATIC_FRONT_RIGHT, m_enemyData[vectINISections[i]]);
        loadEnemySprites(vectINISections[i], EnemySpriteElementType_e::STATIC_BACK, m_enemyData[vectINISections[i]]);
        loadEnemySprites(vectINISections[i], EnemySpriteElementType_e::STATIC_BACK_LEFT, m_enemyData[vectINISections[i]]);
        loadEnemySprites(vectINISections[i], EnemySpriteElementType_e::STATIC_BACK_RIGHT, m_enemyData[vectINISections[i]]);
        loadEnemySprites(vectINISections[i], EnemySpriteElementType_e::STATIC_LEFT, m_enemyData[vectINISections[i]]);
        loadEnemySprites(vectINISections[i], EnemySpriteElementType_e::STATIC_RIGHT, m_enemyData[vectINISections[i]]);
        loadEnemySprites(vectINISections[i], EnemySpriteElementType_e::ATTACK, m_enemyData[vectINISections[i]]);
        loadEnemySprites(vectINISections[i], EnemySpriteElementType_e::DYING, m_enemyData[vectINISections[i]]);
        loadEnemySprites(vectINISections[i], EnemySpriteElementType_e::TOUCHED, m_enemyData[vectINISections[i]]);

    }
}

//===================================================================
bool LevelManager::loadPositionEnemyData()
{
    std::vector<std::string> vectINISections;
    vectINISections = m_ini.getSectionNamesContaining("Enemy");
    std::map<std::string, EnemyData>::iterator it;
    bool exit = false;
    for(uint32_t i = 0; i < vectINISections.size(); ++i)
    {
        it = m_enemyData.find(vectINISections[i]);
        if(it == m_enemyData.end())
        {
            std::cout << "Warning : enemy data id cannot be found" << std::endl;
            continue;
        }
        fillStandartPositionVect(vectINISections[i], it->second.m_TileGamePosition);
        std::optional<std::vector<uint32_t>> results = getBrutPositionData(vectINISections[i], "EndLevelEnemyPos");
        if(results)
        {
            assert(results->size() == 2);
            it->second.m_endLevelPos = {(*results)[0], (*results)[1]};
            exit = true;
            it->second.m_TileGamePosition.push_back({(*results)[0], (*results)[1]});
        }
    }
    return exit;
}

//===================================================================
void LevelManager::loadPositionCheckpointsData()
{
    std::vector<std::string> vectINISections;
    VectPairUI_t vectPos;
    m_checkpointsPos.clear();
    vectINISections = m_ini.getSectionNamesContaining("Checkpoints");
    if(vectINISections.empty())
    {
        return;
    }
    fillStandartPositionVect(vectINISections[0], vectPos);
    std::optional<std::string> dir = m_ini.getValue("Checkpoints", "Direction");
    std::vector<uint32_t> vectDir = convertStrToVectUI(*dir);
    if(vectDir.size() != vectPos.size())
    {
        std::cout << "Warning : inconsistent checkpoints position datas" << std::endl;
        return;
    }
    m_checkpointsPos.reserve(vectDir.size());
    for(uint32_t j = 0; j < vectDir.size(); ++j)
    {
        m_checkpointsPos.emplace_back(std::pair<PairUI_t, Direction_e>{vectPos[j], static_cast<Direction_e>(vectDir[j])});
    }
}

//===================================================================
void LevelManager::loadPositionSecretsData()
{
    std::vector<std::string> vectINISections;
    m_secretsPos.clear();
    vectINISections = m_ini.getSectionNamesContaining("Secrets");
    if(!vectINISections.empty())
    {
        fillStandartPositionVect(vectINISections[0], m_secretsPos);
    }
}

//===================================================================
void LevelManager::loadPositionLogsData()
{
    std::vector<std::string> vectINISections;
    m_logsLevelData.clear();
    vectINISections = m_ini.getSectionNamesContaining("MessageLog");
    m_logsLevelData.reserve(vectINISections.size());
    std::optional<std::string> val;
    std::string id, message;
    PairUI_t pos;
    std::vector<uint32_t> vectPos;
    for(uint32_t j = 0; j < vectINISections.size(); ++j)
    {
        val = m_ini.getValue(vectINISections[j], "DisplayID");
        if(!val)
        {
            std::cout << "Warning : loading logs display id. Skip..." << std::endl;
            continue;
        }
        id = *val;
        val = m_ini.getValue(vectINISections[j], "GamePosition");
        if(!val)
        {
            std::cout << "Warning : loading logs position datas. Skip..." << std::endl;
            continue;
        }
        vectPos = convertStrToVectUI(*val);
        if(vectPos.size() != 2)
        {
            std::cout << "Warning : loading logs position datas. Skip..." << std::endl;
            continue;
        }
        pos = {vectPos[0], vectPos[1]};
        val = m_ini.getValue(vectINISections[j], "Message");
        if(!val)
        {
            std::cout << "Error loading logs message datas. Skip..." << std::endl;
            continue;
        }
        message = *val;
        m_logsLevelData.emplace_back(LogLevelData{id, message, pos});
    }
}

//===================================================================
void LevelManager::loadPrologueAndEpilogue()
{
    std::optional<std::string> val = m_ini.getValue("LevelMessage", "prologue");
    if(val)
    {
        m_prologue = *val;
    }
    val = m_ini.getValue("LevelMessage", "epilogue");
    if(val)
    {
        m_epilogue = *val;
    }
    val = m_ini.getValue("LevelMessage", "epilogueMusic");
    if(val)
    {
        m_epilogueMusic = *val;
    }
}

//===================================================================
void LevelManager::loadUtilsData()
{
    m_weaponsPreviewData.clear();
    std::optional<std::string> val = m_ini.getValue("GraphicUtils", "PannelSprite");
    assert(val);
    m_spritePannelName = *val;
    val = m_ini.getValue("GraphicUtils", "LifeIconSprite");
    assert(val);
    m_spriteLifeName= *val;
    val = m_ini.getValue("GraphicUtils", "AmmoIconSprite");
    assert(val);
    m_spriteAmmoName = *val;
    val = m_ini.getValue("GraphicUtils", "GenericMenuSprite");
    assert(val);
    m_spriteGenericMenuName = *val;
    val = m_ini.getValue("GraphicUtils", "TitleMenuSprite");
    assert(val);
    m_spriteTitleMenuName = *val;
    val = m_ini.getValue("GraphicUtils", "LeftMenuSprite");
    assert(val);
    m_spriteLeftMenuName = *val;
    val = m_ini.getValue("GraphicUtils", "RightLeftMenuSprite");
    assert(val);
    m_spriteRightLeftMenuName = *val;
    //Load visual weapons (changing)
    PairStrPairFloat_t pair;
    m_weaponsPreviewData.reserve(6);
    //FIST
    val = m_ini.getValue("GraphicUtils", "FistIcon");
    assert(val);
    m_spriteFistName = *val;
    pair.first = *val;
    val = m_ini.getValue("GraphicUtils", "SpriteWeightFist");
    assert(val);
    pair.second.first = std::stof(*val) / 2;
    val = m_ini.getValue("GraphicUtils", "SpriteHeightFist");
    assert(val);
    pair.second.second = std::stof(*val) / 2;
    m_weaponsPreviewData.emplace_back(pair);
    //GUN
    val = m_ini.getValue("GraphicUtils", "GunIcon");
    assert(val);
    m_spriteGunName = *val;
    pair.first = *val;
    val = m_ini.getValue("GraphicUtils", "SpriteWeightGun");
    assert(val);
    pair.second.first = std::stof(*val) / 2;
    val = m_ini.getValue("GraphicUtils", "SpriteHeightGun");
    assert(val);
    pair.second.second = std::stof(*val) / 2;
    m_weaponsPreviewData.emplace_back(pair);

    val = m_ini.getValue("GraphicUtils", "ShotgunID");
    assert(val);
    m_weaponsPreviewData.emplace_back(loadPreviewWeaponData(*val));
    m_spriteShotgunName = m_weaponsPreviewData.back().first;
    val = m_ini.getValue("GraphicUtils", "PlasmaRifleID");
    assert(val);
    m_weaponsPreviewData.emplace_back(loadPreviewWeaponData(*val));
    m_spritePlasmaRifleName = m_weaponsPreviewData.back().first;
    val = m_ini.getValue("GraphicUtils", "MachineGunID");
    assert(val);
    m_weaponsPreviewData.emplace_back(loadPreviewWeaponData(*val));
    m_spriteMachineGunName = m_weaponsPreviewData.back().first;
    val = m_ini.getValue("GraphicUtils", "BazookaID");
    assert(val);
    m_weaponsPreviewData.emplace_back(loadPreviewWeaponData(*val));
    m_spriteBazookaName = m_weaponsPreviewData.back().first;
}

//===================================================================
PairStrPairFloat_t LevelManager::loadPreviewWeaponData(const std::string &iniObject)
{
    std::optional<std::string> sprite = m_ini.getValue(iniObject, "Sprite");
    std::optional<std::string> spriteWeight = m_ini.getValue(iniObject, "SpriteWeightGame");
    std::optional<std::string> spriteHeight = m_ini.getValue(iniObject, "SpriteHeightGame");
    assert(sprite);
    assert(spriteWeight);
    assert(spriteHeight);
    return {*sprite, {std::stof(*spriteWeight) / 2, std::stof(*spriteHeight) / 2}};
}

//===================================================================
void LevelManager::loadEnemySprites(const std::string &sectionName,
                                    EnemySpriteElementType_e spriteTypeEnum, EnemyData &enemyData)
{
    std::vector<uint16_t> *vectPtr = nullptr;
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
    std::optional<std::string> val = m_ini.getValue(sectionName, spriteType);
    assert(val);
    std::string sprites = *val;
    assert((!sprites.empty()) && "Enemy sprites cannot be loaded.");
    std::istringstream iss(sprites);
    vectStr_t results(std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>());
    vectPtr->reserve(results.size());
    std::optional<uint16_t> optIdentifier;
    for(uint32_t i = 0; i < results.size(); ++i)
    {
        optIdentifier = m_pictureData.getIdentifier(results[i]);
        assert(optIdentifier);
        vectPtr->emplace_back(*optIdentifier);
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
    if(!loadIniFile(LEVEL_RESSOURCES_DIR_STR + INIFileName, ENCRYPT_KEY_CONF_FILE))
    {
        assert("Error while reading INI file.");
    }
    loadTexturePath();
    loadSpriteData();
    m_pictureData.setUpToDate();
}

//===================================================================
void LevelManager::loadStandardData(const std::string &INIFileName)
{
    if(!loadIniFile(LEVEL_RESSOURCES_DIR_STR + INIFileName, ENCRYPT_KEY_CONF_FILE))
    {
        assert("Error while reading INI file.");
    }
    loadWallData();
    loadWeaponsData();
    loadGeneralSoundData();
    loadGeneralStaticElements(LevelStaticElementType_e::GROUND);
    loadGeneralStaticElements(LevelStaticElementType_e::CEILING);
    loadGeneralStaticElements(LevelStaticElementType_e::OBJECT);
    loadGeneralStaticElements(LevelStaticElementType_e::TELEPORT);
    loadVisualTeleportData();
    loadBarrelsData();
    loadLogData();
    loadExit();
    loadTriggerElements();
    loadDoorData();
    loadVisibleShotDisplayData();
    loadShotImpactDisplayData();
    loadEnemyData();
    loadUtilsData();
    m_weaponINIAssociated.clear();
    m_cardINIAssociated.clear();
}

//===================================================================
void LevelManager::loadFontData(const std::string &INIFileName, Font_e type)
{
    if(!loadIniFile(LEVEL_RESSOURCES_DIR_STR + INIFileName, ENCRYPT_KEY_CONF_FILE))
    {
        assert("Error while reading INI file.");
    }
    //load all sections
    loadSpriteData("", true, type);
}


//===================================================================
void LevelManager::loadPositionStaticElements()
{
    std::map<std::string, StaticLevelElementData>::iterator it = m_groundElement.begin();
    for(; it != m_groundElement.end();)
    {
        if(!fillStandartPositionVect(it->first, it->second.m_TileGamePosition))
        {
            it = removeStaticElement(it->first, LevelStaticElementType_e::GROUND);
        }
        else
        {
            ++it;
        }
    }
    for(it = m_ceilingElement.begin(); it != m_ceilingElement.end();)
    {
        if(!fillStandartPositionVect(it->first, it->second.m_TileGamePosition))
        {
            it = removeStaticElement(it->first, LevelStaticElementType_e::CEILING);
        }
        else
        {
            ++it;
        }
    }
    for(it = m_objectElement.begin(); it != m_objectElement.end();)
    {
        if(!fillStandartPositionVect(it->first, it->second.m_TileGamePosition))
        {
            it = removeStaticElement(it->first, LevelStaticElementType_e::OBJECT);
        }
        else
        {
            ++it;
        }
    }
    it = m_teleportElement.begin();
    if(it != m_teleportElement.end())
    {
        fillTeleportPositions(it->first);
    }
}

//===================================================================
void LevelManager::loadBarrelElements()
{
    //OOOOK refact pos
    m_barrelElement.m_TileGamePosition = getPositionData("Barrel", "GamePosition");
}

//===================================================================
LevelLoadState_e LevelManager::loadLevel(uint32_t levelNum, bool customLevel)
{
    std::string path = customLevel ? LEVEL_RESSOURCES_DIR_STR + "CustomLevels/" +
                                     m_existingCustomLevelsFilename[levelNum] :
                                     LEVEL_RESSOURCES_DIR_STR + std::string("Level") +
                                     std::to_string(levelNum) + std::string ("/level.ini");
    if(!fs::exists(path))
    {
        return LevelLoadState_e::END;
    }
    uint32_t encryptKey = customLevel ? ENCRYPT_KEY_CUSTOM_LEVEL : ENCRYPT_KEY_STANDARD_LEVEL;
    if(!loadIniFile(path, encryptKey))
    {
        std::cout << "ERROR level " << path << " cannot be loaded" << std::endl;
        return LevelLoadState_e::FAIL;
    }
    Level::clearMusicFilePath();
    m_mainWallData.clear();
    if(!loadLevelData())
    {
        std::cout << "ERROR level " << path << " cannot be loaded" << std::endl;
        return LevelLoadState_e::FAIL;
    }
    if(!loadPositionPlayerData(levelNum, customLevel))
    {
        std::cout << "ERROR player data cannot be loaded" << std::endl;
        std::cout << "Level " << path << " cannot be loaded" << std::endl;
        return LevelLoadState_e::FAIL;
    }
    loadPositionWall();
    loadPositionStaticElements();
    loadBarrelElements();
    bool exit = loadPositionExit();
    loadPositionDoorData();
    exit |= loadPositionEnemyData();
    if(!exit)
    {
        std::cout << "ERROR exit data cannot be loaded" << std::endl;
        std::cout << "Level " << path << " cannot be loaded" << std::endl;
        return LevelLoadState_e::FAIL;
    }
    loadPositionCheckpointsData();
    loadPositionSecretsData();
    loadPositionLogsData();
    loadPrologueAndEpilogue();
    if(!loadBackgroundData())
    {
        std::cout << "ERROR background data cannot be loaded" << std::endl;
        std::cout << "Level " << path << " cannot be loaded" << std::endl;
        return LevelLoadState_e::FAIL;
    }
    loadMusicData();
    return LevelLoadState_e::OK;
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
    m_prologue.clear();
    m_epilogue.clear();
    m_epilogueMusic.clear();
}

//===================================================================
bool LevelManager::loadSettingsData()
{
    if(!loadIniFile(LEVEL_RESSOURCES_DIR_STR + "Saves/CustomSettings.ini", {}))
    {
        assert("Error while reading INI file.");
    }
    //AUDIO
    std::optional<std::string> val = m_ini.getValue("Audio", "musicVolume");
    if(val)
    {
        m_settingsData.m_musicVolume = std::stoi(*val);
    }
    val = m_ini.getValue("Audio", "effectsVolume");
    if(val)
    {
        m_settingsData.m_effectsVolume = std::stoi(*val);
    }
    //DISPLAY
    val = m_ini.getValue("Display", "fullscreen");
    if(val)
    {
        std::optional<bool> resBool = toBool(*val);
        assert(resBool);
        m_settingsData.m_fullscreen = *resBool;
    }
    else
    {
        *m_settingsData.m_fullscreen = false;
    }
    val = m_ini.getValue("Display", "resolutionWidth");
    if(val)
    {
        m_settingsData.m_resolutionWidth = std::stoi(*val);
    }
    val = m_ini.getValue("Display", "resolutionHeight");
    if(val)
    {
        m_settingsData.m_resolutionHeight = std::stoi(*val);
    }
    //INPUT
    std::map<std::string, uint32_t>::const_iterator kbIt;
    std::map<std::string, GamepadInputState>::const_iterator gpIt;
    ControlKey_e currentKey;
    if(!m_settingsData.m_arrayKeyboard)
    {
        m_settingsData.m_arrayKeyboard = std::array<MouseKeyboardInputState, static_cast<uint32_t>(ControlKey_e::TOTAL)>();
    }
    for(uint32_t i = 0; i < m_settingsData.m_arrayKeyboard->size(); ++i)
    {
        currentKey = static_cast<ControlKey_e>(i);
        //KEYBOARD
        val = m_ini.getValue("MouseKeyboard", m_inputIDString[i]);
        if(val)
        {
            bool ok = false;
            kbIt = m_inputKeyboardKeyString.find(*val);
            if(kbIt != m_inputKeyboardKeyString.end())
            {
                m_settingsData.m_arrayKeyboard->at(i) = {true, kbIt->second};
                ok = true;
            }
            if(!ok)
            {
                kbIt = m_inputMouseKeyString.find(*val);
                if(kbIt != m_inputMouseKeyString.end())
                {
                    m_settingsData.m_arrayKeyboard->at(i) = {false, kbIt->second};
                }
                else
                {
                    m_settingsData.m_arrayKeyboard->at(i) = MAP_KEYBOARD_DEFAULT_KEY.at(currentKey);
                }
            }
        }
        else
        {
            m_settingsData.m_arrayKeyboard->at(i) = MAP_KEYBOARD_DEFAULT_KEY.at(currentKey);
        }
    }
    if(!m_settingsData.m_arrayGamepad)
    {
        m_settingsData.m_arrayGamepad = std::array<GamepadInputState, static_cast<uint32_t>(ControlKey_e::TOTAL)>();
    }
    for(uint32_t i = 0; i < m_settingsData.m_arrayGamepad->size(); ++i)
    {
        //GAMEPAD
        val = m_ini.getValue("Gamepad", m_inputIDString[i]);
        if(val)
        {
            gpIt = m_inputGamepadKeyString.find(*val);
            if(gpIt != m_inputGamepadKeyString.end())
            {
                m_settingsData.m_arrayGamepad->at(i) = gpIt->second;
            }
            //default
            else
            {
                m_settingsData.m_arrayGamepad->at(i) = MAP_GAMEPAD_DEFAULT_KEY.at(currentKey);
            }
        }
        else
        {
            m_settingsData.m_arrayGamepad->at(i) = MAP_GAMEPAD_DEFAULT_KEY.at(currentKey);
        }
    }
    val = m_ini.getValue("Input", "TurnSensitivity");
    if(val)
    {
        m_settingsData.m_turnSensitivity = std::stoi(*val);
    }
    return true;
}

//===================================================================
void LevelManager::fillSettingsFileFromMemory()
{
    m_ini.clear();
    //AUDIO
    if(m_settingsData.m_musicVolume)
    {
        m_ini.setValue("Audio", "musicVolume", std::to_string(*m_settingsData.m_musicVolume));
    }
    if(m_settingsData.m_effectsVolume)
    {
        m_ini.setValue("Audio", "effectsVolume", std::to_string(*m_settingsData.m_effectsVolume));
    }
    //DISPLAY
    if(m_settingsData.m_resolutionWidth)
    {
        m_ini.setValue("Display", "resolutionWidth", std::to_string(*m_settingsData.m_resolutionWidth));
    }
    if(m_settingsData.m_resolutionHeight)
    {
        m_ini.setValue("Display", "resolutionHeight", std::to_string(*m_settingsData.m_resolutionHeight));
    }
    if(m_settingsData.m_fullscreen)
    {
        m_ini.setValue("Display", "fullscreen", *m_settingsData.m_fullscreen ? "true" : "false");
    }
    if(m_settingsData.m_arrayKeyboard)
    {
        std::string str;
        for(uint32_t i = 0; i < m_settingsData.m_arrayKeyboard->size(); ++i)
        {
            str = m_settingsData.m_arrayKeyboard->at(i).m_keyboard ?
                        INPUT_KEYBOARD_KEY_STRING.at(m_settingsData.m_arrayKeyboard->at(i).m_key) :
                        INPUT_MOUSE_KEY_STRING.at(m_settingsData.m_arrayKeyboard->at(i).m_key);
            m_ini.setValue("MouseKeyboard", m_inputIDString[i], str);
        }
    }
    if(m_settingsData.m_arrayGamepad)
    {
        for(uint32_t i = 0; i < m_settingsData.m_arrayGamepad->size(); ++i)
        {
            m_ini.setValue("Gamepad", m_inputIDString[i], getGamepadKeyIniString(m_settingsData.m_arrayGamepad->at(i)));
        }
    }
}

//===================================================================
void LevelManager::saveAudioSettings(uint32_t musicVolume, uint32_t effectVolume)
{
    m_outputStream.open(LEVEL_RESSOURCES_DIR_STR + "Saves/CustomSettings.ini");
    fillSettingsFileFromMemory();
    m_ini.setValue("Audio", "musicVolume", std::to_string(musicVolume));
    m_ini.setValue("Audio", "effectsVolume", std::to_string(effectVolume));
    m_settingsData.m_musicVolume = musicVolume;
    m_settingsData.m_effectsVolume = effectVolume;
    m_ini.generate(m_outputStream);
    m_outputStream.close();
}

//===================================================================
void LevelManager::saveDisplaySettings(const pairI_t &resolution, bool fullscreen)
{
    m_outputStream.open(LEVEL_RESSOURCES_DIR_STR + "Saves/CustomSettings.ini");
    fillSettingsFileFromMemory();
    m_ini.setValue("Display", "resolutionWidth", std::to_string(resolution.first));
    m_ini.setValue("Display", "resolutionHeight", std::to_string(resolution.second));
    m_ini.setValue("Display", "fullscreen", fullscreen ? "true" : "false");
    m_settingsData.m_resolutionWidth = resolution.first;
    m_settingsData.m_resolutionHeight = resolution.second;
    m_settingsData.m_fullscreen = fullscreen;
    m_ini.generate(m_outputStream);
    m_outputStream.close();
}

//===================================================================
void LevelManager::saveInputSettings(const std::map<ControlKey_e, GamepadInputState> &gamepadArray,
                                     const std::map<ControlKey_e, MouseKeyboardInputState> &keyboardArray)
{
    m_outputStream.open(LEVEL_RESSOURCES_DIR_STR + "Saves/CustomSettings.ini");
    fillSettingsFileFromMemory();
    uint32_t currentIndex;
    std::string valStr;
    uint32_t cmpt = 0;
    for(std::map<ControlKey_e, GamepadInputState>::const_iterator it = gamepadArray.begin(); it != gamepadArray.end(); ++it, ++cmpt)
    {
        currentIndex = static_cast<uint32_t>(it->first);
        getGamepadKeyIniString(it->second);
        m_ini.setValue("Gamepad", m_inputIDString[currentIndex], valStr);
        m_settingsData.m_arrayGamepad->at(cmpt) = it->second;
    }
    cmpt = 0;
    std::string str;
    for(std::map<ControlKey_e, MouseKeyboardInputState>::const_iterator it = keyboardArray.begin();
        it != keyboardArray.end(); ++it, ++cmpt)
    {
        str = it->second.m_keyboard ? INPUT_KEYBOARD_KEY_STRING.at(it->second.m_key) :
                    INPUT_MOUSE_KEY_STRING.at(it->second.m_key);
        currentIndex = static_cast<uint32_t>(it->first);
        m_ini.setValue("MouseKeyboard", m_inputIDString[currentIndex], str);
        m_settingsData.m_arrayKeyboard->at(cmpt) = it->second;
    }
    m_ini.generate(m_outputStream);
    m_outputStream.close();
}

//===================================================================
void LevelManager::saveTurnSensitivitySettings(uint32_t sensitivity)
{
    m_outputStream.open(LEVEL_RESSOURCES_DIR_STR + "Saves/CustomSettings.ini");
    fillSettingsFileFromMemory();
    m_ini.setValue("Input", "TurnSensitivity", std::to_string(sensitivity));
    m_ini.generate(m_outputStream);
    m_outputStream.close();
}

//===================================================================
std::string LevelManager::getGamepadKeyIniString(const GamepadInputState &gamepadInputState)const
{
    std::string valStr;
    if(gamepadInputState.m_standardButton)
    {
        valStr = m_inputGamepadSimpleButtons.at(gamepadInputState.m_keyID);
    }
    else
    {
        valStr = m_inputGamepadAxis.at(gamepadInputState.m_keyID);
        if(*gamepadInputState.m_axisPos)
        {
            valStr += '+';
        }
        else
        {
            valStr += '-';
        }
    }
    return valStr;
}

//===================================================================
std::string LevelManager::saveLevelGameProgress(const MemPlayerConf &playerConfBeginLevel, const MemPlayerConf &playerConfCheckpoint,
                                                uint32_t levelNum, bool beginLevel)
{
    std::string date = getStrDate();
    //remove \n
    date.pop_back();
    m_ini.setValue("Level", "levelNum", std::to_string(levelNum));
    m_ini.setValue("Level", "date", date);
    savePlayerGear(true, playerConfBeginLevel);
    if(!beginLevel)
    {
        savePlayerGear(false, playerConfCheckpoint);
    }
    return date;
}

//===================================================================
void LevelManager::savePlayerGear(bool beginLevel, const MemPlayerConf &playerConf)
{
    std::string sectionName = beginLevel ? "PlayerBeginLevel" : "PlayerCheckpoint";
    m_ini.setValue(sectionName, "life", std::to_string(playerConf.m_life));
    m_ini.setValue(sectionName, "previousWeapon", std::to_string(playerConf.m_previousWeapon));
    m_ini.setValue(sectionName, "currentWeapon", std::to_string(playerConf.m_currentWeapon));
    std::string weaponPosses, weaponAmmoCount;
    assert(!playerConf.m_ammunationsCount.empty());
    assert(playerConf.m_ammunationsCount.size() == playerConf.m_weapons.size());
    for(uint32_t i = 0; i < playerConf.m_ammunationsCount.size(); ++i)
    {
        weaponAmmoCount += std::to_string(playerConf.m_ammunationsCount[i]) + " ";
        weaponPosses += std::to_string(playerConf.m_weapons[i]) + " ";
    }
    m_ini.setValue(sectionName, "weaponPossess", weaponPosses);
    m_ini.setValue(sectionName, "weaponAmmoCount", weaponAmmoCount);
}

//===================================================================
void LevelManager::saveElementsGameProgress(const MemCheckpointElementsState &checkpointData)
{
    m_ini.setValue("Checkpoint", "Num", std::to_string(checkpointData.m_checkpointNum));
    m_ini.setValue("Checkpoint", "Direction", std::to_string(static_cast<uint32_t>(checkpointData.m_direction)));
    m_ini.setValue("Checkpoint", "SecretsFound", std::to_string(checkpointData.m_secretsNumber));
    m_ini.setValue("Checkpoint", "EnemiesKilled", std::to_string(checkpointData.m_enemiesKilled));
    m_ini.setValue("Checkpoint", "PosX", std::to_string(checkpointData.m_checkpointPos.first));
    m_ini.setValue("Checkpoint", "PosY", std::to_string(checkpointData.m_checkpointPos.second));
    saveEnemiesDataGameProgress(checkpointData.m_enemiesData);
    saveStaticElementsDataGameProgress(checkpointData.m_staticElementDeleted);
    saveMoveableWallDataGameProgress(checkpointData.m_moveableWallData);
    saveTriggerWallMoveableWallDataGameProgress(checkpointData.m_triggerWallMoveableWallData);
    saveRevealedMapGameProgress(checkpointData.m_revealedMapData);
    saveCardGameProgress(checkpointData.m_card);
}

//===================================================================
void LevelManager::saveMoveableWallDataGameProgress(const std::map<uint32_t, std::pair<uint32_t, bool>> &moveableWallData)
{
    std::string strShapeNum, strNumberOfTriggers, strReversable;
    std::map<uint32_t, std::pair<uint32_t, bool>>::const_iterator it = moveableWallData.begin();
    for(; it != moveableWallData.end(); ++it)
    {
        strShapeNum += std::to_string(it->first) + " ";
        strNumberOfTriggers += std::to_string(it->second.first) + " ";
        strReversable += it->second.second ? "1 " : "0 ";
    }
    m_ini.setValue("MoveableWall", "ShapeNum", strShapeNum);
    m_ini.setValue("MoveableWall", "NumberOfTriggers", strNumberOfTriggers);
    m_ini.setValue("MoveableWall", "Reversable", strReversable);
}

//===================================================================
void LevelManager::saveTriggerWallMoveableWallDataGameProgress(
        const std::map<uint32_t, std::pair<std::vector<uint32_t>, bool>> &triggerWallMoveableWallData)
{
    std::string strNumberOfTriggers, strCurrentShape;
    uint32_t cmpt = 0;
    for(std::map<uint32_t, std::pair<std::vector<uint32_t>, bool>>::const_iterator it = triggerWallMoveableWallData.begin();
        it != triggerWallMoveableWallData.end(); ++it, ++cmpt)
    {
        strNumberOfTriggers.clear();
        for(uint32_t i = 0; i < it->second.first.size(); ++i)
        {
            strNumberOfTriggers += std::to_string(it->second.first[i]) + " ";
        }
        strCurrentShape = "TriggerWallMoveableWall" + std::to_string(cmpt);
        m_ini.setValue(strCurrentShape, "ShapeNum", std::to_string(it->first));
        m_ini.setValue(strCurrentShape, "NumberOfTriggers", strNumberOfTriggers);
        m_ini.setValue(strCurrentShape, "Reversable", it->second.second ? "1" : "0");
    }
}

//===================================================================
void LevelManager::saveEnemiesDataGameProgress(const std::vector<MemCheckpointEnemiesState> &enemiesData)
{
    std::string strPos, strDead, strObjectPickedUp, strLife, strTmp;
    for(uint32_t i = 0; i < enemiesData.size(); ++i)
    {
        strTmp = enemiesData[i].m_dead ? "1" : "0";
        strDead += strTmp + " ";
        strTmp = enemiesData[i].m_objectPickedUp ? "1" : "0";
        strObjectPickedUp += strTmp + " ";
        strPos += std::to_string(enemiesData[i].m_enemyPos.first) + " " +
                std::to_string(enemiesData[i].m_enemyPos.second) + " ";
        strLife += std::to_string(enemiesData[i].m_life) + " ";
    }
    m_ini.setValue("Enemies", "Dead", strDead);
    m_ini.setValue("Enemies", "ObjectPickedUp", strObjectPickedUp);
    m_ini.setValue("Enemies", "Pos", strPos);
    m_ini.setValue("Enemies", "Life", strLife);
}

//===================================================================
void LevelManager::saveCardGameProgress(const std::set<uint32_t> &cardData)
{
    std::string str;
    for(std::set<uint32_t>::const_iterator it = cardData.begin(); it != cardData.end(); ++it)
    {
        str += std::to_string(*it) + " ";
    }
    m_ini.setValue("Card", "Num", str);
}

//===================================================================
void LevelManager::saveStaticElementsDataGameProgress(const std::set<PairUI_t> &staticElementData)
{
    std::string strPos;
    for(std::set<PairUI_t>::const_iterator it = staticElementData.begin();
        it != staticElementData.end(); ++it)
    {
        strPos += std::to_string(it->first) + " " + std::to_string(it->second) + " ";
    }
    m_ini.setValue("StaticElements", "Pos", strPos);
}

//===================================================================
void LevelManager::saveRevealedMapGameProgress(const std::vector<PairUI_t> &revealedMapData)
{
    std::string strPos;
    for(uint32_t i = 0; i < revealedMapData.size(); ++i)
    {
        strPos += std::to_string(revealedMapData[i].first) + " " +
                std::to_string(revealedMapData[i].second) + " ";
    }
    m_ini.setValue("RevealedMap", "Pos", strPos);
}

//===================================================================
void LevelManager::generateSavedFile(uint32_t numSaveFile)
{
    std::string str;
    std::stringstream stringStream;
    m_ini.generate(stringStream);
    m_outputStream.open(LEVEL_RESSOURCES_DIR_STR + "Saves/save" + std::to_string(numSaveFile) + ".ini");
    str = encrypt(stringStream.str(), ENCRYPT_KEY_CONF_FILE);
    m_outputStream << str;
    m_outputStream.close();
}

//===================================================================
bool LevelManager::loadIniFile(std::string_view path, std::optional<uint32_t> encryptKey)
{
    m_inputStream.open(path.data());
    if(m_inputStream.fail())
    {
        m_inputStream.close();
        return false;
    }
    std::ostringstream ostringStream;
    ostringStream << m_inputStream.rdbuf();
    m_inputStream.close();
    m_ini.clear();
    std::string dataString = encryptKey ? decrypt(ostringStream.str(), *encryptKey) : ostringStream.str();
    std::istringstream istringStream(dataString);
    m_ini.parse(istringStream);
    return true;
}

//===================================================================
std::string LevelManager::saveGameProgress(const MemPlayerConf &playerConfBeginLevel, const MemPlayerConf &playerConfCheckpoint,
                                           uint32_t levelNum, uint32_t numSaveFile, const MemCheckpointElementsState *checkpointData)
{
    m_ini.clear();
    std::string date = saveLevelGameProgress(playerConfBeginLevel, playerConfCheckpoint,  levelNum, !checkpointData);
    if(checkpointData)
    {
        saveElementsGameProgress(*checkpointData);
    }
    generateSavedFile(numSaveFile);
    return date;
}

//===================================================================
std::optional<MemLevelLoadedData> LevelManager::loadSavedGame(uint32_t saveNum)
{
    std::string path = LEVEL_RESSOURCES_DIR_STR + "Saves/save" + std::to_string(saveNum) + ".ini";
    if(saveNum == 0 || !fs::exists(path))
    {
        return {};
    }
    loadIniFile(path, ENCRYPT_KEY_CONF_FILE);
    std::optional<std::string> val;
    val = m_ini.getValue("Level", "levelNum");
    if(!val)
    {
        return {};
    }
    uint32_t levelNum = std::stoi(*val);
    std::optional<MemPlayerConf> playerConfBeginLevel = loadPlayerConf(true),
            playerConfCheckpoint = loadPlayerConf(false);
    if(!playerConfBeginLevel || playerConfBeginLevel->m_ammunationsCount.size() != playerConfBeginLevel->m_weapons.size())
    {
        return {};
    }
    if(playerConfCheckpoint && playerConfCheckpoint->m_ammunationsCount.size() != playerConfCheckpoint->m_weapons.size())
    {
        return {};
    }
    return MemLevelLoadedData{levelNum, playerConfBeginLevel, playerConfCheckpoint, loadCheckpointDataSavedGame()};
}

//===================================================================
std::optional<MemPlayerConf> LevelManager::loadPlayerConf(bool beginLevel)
{
    std::string sectionName = beginLevel ? "PlayerBeginLevel" : "PlayerCheckpoint";
    MemPlayerConf playerConf;
    std::optional<std::string> val;
    val = m_ini.getValue(sectionName, "life");
    if(!val)
    {
        return {};
    }
    playerConf.m_life = std::stoi(*val);
    val = m_ini.getValue(sectionName, "currentWeapon");
    if(!val)
    {
        return {};
    }
    playerConf.m_currentWeapon = std::stoi(*val);
    val = m_ini.getValue(sectionName, "previousWeapon");
    if(!val)
    {
        return {};
    }
    playerConf.m_previousWeapon = std::stoi(*val);
    val = m_ini.getValue(sectionName, "weaponAmmoCount");
    if(!val)
    {
        return {};
    }
    playerConf.m_ammunationsCount = convertStrToVectUI(*val);
    val = m_ini.getValue(sectionName, "weaponPossess");
    if(!val)
    {
        return {};
    }
    playerConf.m_weapons = convertStrToVectBool(*val);
    return playerConf;
}

//===================================================================
std::unique_ptr<MemCheckpointElementsState> LevelManager::loadCheckpointDataSavedGame()
{
    uint32_t checkpointNum, secretsFound, enemiesKilled;
    PairUI_t pos;
    Direction_e direction;
    std::optional<std::string> val;
    val = m_ini.getValue("Checkpoint", "Num");
    if(!val)
    {
        return nullptr;
    }
    checkpointNum = std::stoi(*val);
    val = m_ini.getValue("Checkpoint", "SecretsFound");
    assert(val);
    secretsFound = std::stoi(*val);

    val = m_ini.getValue("Checkpoint", "Direction");
    assert(val);
    direction = static_cast<Direction_e>(std::stoi(*val));

    val = m_ini.getValue("Checkpoint", "EnemiesKilled");
    assert(val);
    enemiesKilled = std::stoi(*val);
    val = m_ini.getValue("Checkpoint", "PosX");
    assert(val);
    pos.first = std::stoi(*val);
    val = m_ini.getValue("Checkpoint", "PosY");
    assert(val);
    pos.second = std::stoi(*val);
    return std::make_unique<MemCheckpointElementsState>(MemCheckpointElementsState{
                    checkpointNum, secretsFound, enemiesKilled, pos, direction,
                    loadEnemiesDataGameProgress(), loadMoveableWallDataGameProgress(),
                    loadTriggerWallMoveableWallDataGameProgress(),
                    loadStaticElementsDataGameProgress(), loadRevealedMapDataGameProgress(), loadCardGameProgress()});
}

//===================================================================
std::map<uint32_t, std::pair<std::vector<uint32_t>, bool>> LevelManager::loadTriggerWallMoveableWallDataGameProgress()
{
    std::map<uint32_t, std::pair<std::vector<uint32_t>, bool>> mapReturn;
    std::optional<std::string> val;
    uint32_t shapeNum;
    std::vector<uint32_t> vectTriggers;
    std::vector<std::string> vectSection = m_ini.getSectionNamesContaining("TriggerWallMoveableWall");
    for(uint32_t i = 0; i < vectSection.size(); ++i)
    {
        val = m_ini.getValue(vectSection[i], "ShapeNum");
        assert(val);
        shapeNum = std::stoi(*val);
        val = m_ini.getValue(vectSection[i], "NumberOfTriggers");
        assert(val);
        vectTriggers = convertStrToVectUI(*val);
        val = m_ini.getValue(vectSection[i], "Reversable");
        assert(val);
        mapReturn.insert({shapeNum, {vectTriggers, std::stoi(*val) == 1 ? true : false}});
    }
    return mapReturn;
}

//===================================================================
std::map<uint32_t, std::pair<uint32_t, bool>> LevelManager::loadMoveableWallDataGameProgress()
{
    std::optional<std::string> val;
    std::vector<uint32_t> vectShapes, vectTriggers;
    std::vector<bool> vectRevers;
    std::map<uint32_t, std::pair<uint32_t, bool>> map;
    val = m_ini.getValue("MoveableWall", "ShapeNum");
    if(!val)
    {
        return {};
    }
    vectShapes = convertStrToVectUI(*val);
    val = m_ini.getValue("MoveableWall", "NumberOfTriggers");
    assert(val);
    vectTriggers = convertStrToVectUI(*val);
    assert(vectShapes.size() == vectTriggers.size());
    val = m_ini.getValue("MoveableWall", "Reversable");
    assert(val);
    vectRevers = convertStrToVectBool(*val);
    assert(vectRevers.size() == vectTriggers.size());
    for(uint32_t i = 0; i < vectShapes.size(); ++i)
    {
        map.insert({vectShapes[i], {vectTriggers[i], vectRevers[i]}});
    }
    return map;
}

//===================================================================
std::vector<MemCheckpointEnemiesState> LevelManager::loadEnemiesDataGameProgress()
{
    std::vector<MemCheckpointEnemiesState> vectData;
    std::optional<std::string> val;
    std::vector<PairFloat_t> vectPairPos;
    //dead
    val = m_ini.getValue("Enemies", "Dead");
    assert(val);
    std::vector<bool> vectDead = convertStrToVectBool(*val);
    //object picked up
    val = m_ini.getValue("Enemies", "ObjectPickedUp");
    assert(val);
    std::vector<bool> vectObject = convertStrToVectBool(*val);
    //pos
    val = m_ini.getValue("Enemies", "Pos");
    assert(val);
    std::vector<float> vectPos = convertStrToVectFloat(*val);
    assert(vectPos.size() % 2 == 0);
    vectPairPos.reserve(vectPos.size() / 2);
    for(uint32_t i = 0; i < vectPos.size(); i += 2)
    {
        vectPairPos.emplace_back(PairFloat_t{vectPos[i], vectPos[i + 1]});
    }
    //life
    val = m_ini.getValue("Enemies", "Life");
    assert(val);
    std::vector<uint32_t> vectLife = convertStrToVectUI(*val);
    assert(vectDead.size() == vectObject.size());
    assert(vectDead.size() == vectPairPos.size());
    assert(vectDead.size() == vectLife.size());
    for(uint32_t i = 0; i < vectDead.size(); ++i)
    {
        vectData.emplace_back(MemCheckpointEnemiesState{0, vectLife[i], vectDead[i],
                                                        vectObject[i], vectPairPos[i]});
    }
    return vectData;
}

//===================================================================
std::set<PairUI_t> LevelManager::loadStaticElementsDataGameProgress()
{
    std::set<PairUI_t> pairPos;
    std::optional<std::string> val = m_ini.getValue("StaticElements", "Pos");
    assert(val);
    std::vector<uint32_t> vectPos = convertStrToVectUI(*val);
    assert(vectPos.size() % 2 == 0);
    for(uint32_t i = 0; i < vectPos.size(); i += 2)
    {
        pairPos.insert(PairUI_t{vectPos[i], vectPos[i + 1]});
    }
    return pairPos;
}

//===================================================================
std::vector<PairUI_t> LevelManager::loadRevealedMapDataGameProgress()
{
    std::vector<PairUI_t> vectRet;
    std::optional<std::string> val = m_ini.getValue("RevealedMap", "Pos");
    std::vector<uint32_t> vectPos = convertStrToVectUI(*val);
    assert(vectPos.size() % 2 == 0);
    vectRet.reserve(vectPos.size() / 2);
    for(uint32_t i = 0; i < vectPos.size(); i += 2)
    {
        vectRet.emplace_back(PairUI_t{vectPos[i], vectPos[i + 1]});
    }
    return vectRet;
}

//===================================================================
std::set<uint32_t> LevelManager::loadCardGameProgress()
{
    std::set<uint32_t> set;
    std::optional<std::string> val = m_ini.getValue("Card", "Num");
    std::vector<uint32_t> vectPos = convertStrToVectUI(*val);
    for(uint32_t i = 0; i < vectPos.size(); ++i)
    {
        set.insert(vectPos[i]);
    }
    return set;
}

//===================================================================
std::array<std::optional<DataLevelWriteMenu>, 3> LevelManager::getExistingLevelNumSaves()
{
    std::string path;
    std::array<std::optional<DataLevelWriteMenu>, 3> ret;
    for(uint32_t i = 1; i < 4; ++i)
    {
        path = LEVEL_RESSOURCES_DIR_STR + "Saves/save" + std::to_string(i) + ".ini";
        if(fs::exists(path))
        {
            ret[i - 1] = DataLevelWriteMenu();
            loadIniFile(path, ENCRYPT_KEY_CONF_FILE);
            std::optional<std::string> val = m_ini.getValue("Level", "levelNum");
            assert(val);
            ret[i - 1]->m_levelNum = std::stoi(*val);
            val = m_ini.getValue("Level", "date");
            assert(val);
            ret[i - 1]->m_date = *val;
            val = m_ini.getValue("Checkpoint", "Num");
            if(val)
            {
                ret[i - 1]->m_checkpointNum = std::stoi(*val);
            }
            else
            {
                ret[i - 1]->m_checkpointNum = 0;
            }
        }
    }
    return ret;
}

//===================================================================
void LevelManager::loadExistingCustomLevelFilename()
{
    if(!fs::exists(fs::path(LEVEL_RESSOURCES_DIR_STR + "CustomLevels/")))
    {
        return;
    }
    for(fs::directory_iterator it =
        fs::directory_iterator(LEVEL_RESSOURCES_DIR_STR + "CustomLevels/");
        it != fs::directory_iterator(); ++it)
    {
        if(it->path().extension().string() == ".clvl")
        {
            m_existingCustomLevelsFilename.push_back(it->path().filename().string());
        }
    }
}

//===================================================================
bool LevelManager::checkSavedGameExists(uint32_t saveNum)const
{
    return fs::exists(LEVEL_RESSOURCES_DIR_STR + "Saves/save" + std::to_string(saveNum) + ".ini");
}

//===================================================================
std::string encrypt(const std::string &str, uint32_t key)
{
    std::string strR = str;
    for(uint32_t i = 0; i < strR.size(); ++i)
    {
        strR[i] += key;
    }
    return strR;
}

//===================================================================
std::string decrypt(const std::string &str, uint32_t key)
{
    std::string strR = str;
    for(uint32_t i = 0; i < strR.size(); ++i)
    {
        strR[i] -= key;
    }
    return strR;
}

//===================================================================
std::optional<bool> toBool(const std::string &str)
{
    if(str == "true")
    {
        return true;
    }
    else if(str == "false")
    {
        return false;
    }
    return {};
}

//===================================================================
//Mon Mar 14 11:51:56 2022
std::string getStrDate()
{
    std::time_t end_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::string str = std::ctime(&end_time), ret;
    ret += str.erase(0, str.find(' '));
    std::transform(ret.begin(), ret.end(), ret.begin(), ::toupper);
    return ret;
}
