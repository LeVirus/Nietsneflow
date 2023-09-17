#include "GraphicEngine.hpp"
#include "FontData.hpp"
#include "MainEngine.hpp"
#include <iostream>
#include <cassert>
#include <constants.hpp>
#include <OpenGLUtils/glheaders.hpp>
#include <ECS/Systems/ColorDisplaySystem.hpp>
#include <ECS/Systems/MapDisplaySystem.hpp>
#include <ECS/Systems/VisionSystem.hpp>
#include <ECS/Systems/FirstPersonDisplaySystem.hpp>
#include <ECS/Systems/StaticDisplaySystem.hpp>
#include <ECS/Components/PlayerConfComponent.hpp>
#include <ECS/Components/WriteComponent.hpp>
#include <ECS/Components/WeaponComponent.hpp>

//===================================================================
GraphicEngine::GraphicEngine()
{
    initGLWindow();
    initGlad();
    initGLShader();
}

//===================================================================
void GraphicEngine::loadExistingLevelNumSaves(const std::array<std::optional<DataLevelWriteMenu>, 3> &existingLevelNum)
{
    m_memExistingLevelSave = existingLevelNum;
    m_saveStandardLevelMenuWrite.clear();
    std::string checkpoint;
    for(uint32_t i = 0; i < 3; ++i)
    {
        m_saveStandardLevelMenuWrite += std::to_string(i + 1);
        if(existingLevelNum[i])
        {
            checkpoint = (existingLevelNum[i]->m_checkpointNum == 0 || m_restartLevelMode) ? "" :
                " Chckpt " + std::to_string(existingLevelNum[i]->m_checkpointNum);
            m_saveStandardLevelMenuWrite += "  Lvl " + std::to_string(existingLevelNum[i]->m_levelNum) +
                    checkpoint + " " + existingLevelNum[i]->m_date;
        }
        m_saveStandardLevelMenuWrite += "\\";
    }
    m_saveStandardLevelMenuWrite += "Return";
    m_restartLevelMode = false;
}

//===================================================================
void GraphicEngine::loadExistingCustomLevel(const std::vector<std::string> &customLevels)
{
    if(customLevels.empty())
    {
        m_existingCustomLevelsMenuWrite.emplace_back(std::pair<std::string, uint32_t>{"Return", 0});
        return;
    }
    uint32_t currentSectionCursor = 0, size = customLevels.size() / CUSTOM_MENU_SECTION_SIZE +
            ((customLevels.size() % CUSTOM_MENU_SECTION_SIZE == 0) ? 0 : 1),
            currentSection = 0;
    std::string levelName, strFinal, longNameContract = "...CLVL";
    m_existingCustomLevelsMenuWrite.clear();
    m_existingCustomLevelsMenuWrite.resize(size);
    for(uint32_t i = 0; i < customLevels.size(); ++i)
    {
        levelName = customLevels[i];
        //mem level full name
        if(levelName.size() > 15)
        {
            levelName.resize(15);
            std::copy(longNameContract.begin(), longNameContract.end(), levelName.begin() + levelName.size() - longNameContract.size());
        }
        std::transform(levelName.begin(), levelName.end(), levelName.begin(), [](uint16_t c){ return std::toupper(c); });
        strFinal += std::to_string(i + 1) + " " + levelName + "\\";
        if(currentSectionCursor == (CUSTOM_MENU_SECTION_SIZE - 1) || i == customLevels.size() - 1)
        {
            strFinal += (size > 1) ? "Previous\\Next\\Return" : "Return";
            m_existingCustomLevelsMenuWrite[currentSection] = {strFinal, currentSectionCursor + ((size > 1) ? 3 : 1)};
            ++currentSection;
            currentSectionCursor = 0;
            strFinal.clear();
        }
        else
        {
            ++currentSectionCursor;
        }
    }
}

//===================================================================
void GraphicEngine::confSystems()
{
    setShaderToLocalSystems();
    m_mapSystem->setVectTextures(m_vectTexture);
    m_firstPersonSystem->setVectTextures(m_vectTexture);
    m_staticDisplaySystem->setVectTextures(m_vectTexture);
    m_staticDisplaySystem->memFontDataPtr(m_ptrFontData);
}

//===================================================================
void GraphicEngine::loadPictureData(const PictureData &pictureData, const FontData &fontData)
{
    loadSprites(pictureData.getSpriteData(), fontData);
    initGLTexture(pictureData.getTexturePath());
}

//===================================================================
void GraphicEngine::runIteration(bool gamePaused)
{
    preDisplay();
    mainDisplay(gamePaused);
    postDisplay();
}

//===================================================================
void GraphicEngine::setTransition(bool gamePaused, bool redTransition)
{
    uint32_t transitionTotal;
    if(redTransition)
    {
        transitionTotal = m_redTransitionFrameNumber;
        m_colorSystem->setRedTransition();
    }
    else
    {
        transitionTotal = m_transitionFrameNumber;
    }
    for(uint32_t i = 0; i < transitionTotal; ++i)
    {
        preDisplay();
        mainDisplay(gamePaused);
        m_colorSystem->setTransition(i, transitionTotal);
        postDisplay();
    }
}

//===================================================================
void GraphicEngine::unsetTransition(bool gamePaused, bool unsetRedTransition)
{
    for(uint32_t i = m_transitionFrameNumber; i > 0; --i)
    {
        preDisplay();
        mainDisplay(gamePaused);
        m_colorSystem->setTransition(i, m_transitionFrameNumber);
        postDisplay();
    }
    if(unsetRedTransition)
    {
        m_colorSystem->unsetRedTransition();
    }
}

//===================================================================
void GraphicEngine::mainDisplay(bool gamePaused)
{
    if(!gamePaused)
    {
        displayGameIteration();
    }
    else
    {
        m_staticDisplaySystem->displayMenu();
    }
}

//===================================================================
void GraphicEngine::toogleFullScreen()
{
    m_fullscreenMode = !m_fullscreenMode;
    m_displayMenuFullscreenMode = m_fullscreenMode;
    m_staticDisplaySystem->updateMenuEntryFullscreen(m_fullscreenMode);
    glfwSetWindowMonitor(m_window, m_fullscreenMode ? glfwGetPrimaryMonitor() : nullptr,
                         0, 0, m_memGraphicResolution[m_currentResolution].first.first,
                         m_memGraphicResolution[m_currentResolution].first.second, GLFW_DONT_CARE);
}

//===================================================================
void GraphicEngine::preDisplay()
{
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

//===================================================================
void GraphicEngine::postDisplay()
{
    glfwSwapBuffers(m_window);
    glfwPollEvents();
}

//===================================================================
void GraphicEngine::displayGameIteration()
{
    m_colorSystem->execSystem();
    m_visionSystem->execSystem();
    m_firstPersonSystem->execSystem();
    m_staticDisplaySystem->execSystem();
    m_mapSystem->execSystem();
}

//===================================================================
bool GraphicEngine::windowShouldClose()
{
    return glfwWindowShouldClose(m_window);
}

//===================================================================
void GraphicEngine::updateSaveNum(uint32_t levelNum, uint32_t saveNum, std::optional<uint32_t> checkpointNum,
                                  const std::string &date, bool beginLevel)
{
    m_memExistingLevelSave[saveNum - 1]->m_levelNum = levelNum;
    if(!date.empty())
    {
        m_memExistingLevelSave[saveNum - 1]->m_date = date;
    }
    if(beginLevel)
    {
        m_memExistingLevelSave[saveNum - 1]->m_checkpointNum = 0;
    }
    else if(checkpointNum)
    {
        m_memExistingLevelSave[saveNum - 1]->m_checkpointNum = *checkpointNum;
    }
    loadExistingLevelNumSaves(m_memExistingLevelSave);
}

//===================================================================
void GraphicEngine::linkSystems(ColorDisplaySystem *colorSystem, MapDisplaySystem *mapSystem,
                                FirstPersonDisplaySystem *firstPersonSystem, VisionSystem *visionSystem,
                                StaticDisplaySystem *staticDisplaySystem)
{
    m_colorSystem = colorSystem;
    m_mapSystem = mapSystem;
    m_firstPersonSystem = firstPersonSystem;
    m_visionSystem = visionSystem;
    m_staticDisplaySystem = staticDisplaySystem;
}



//===================================================================
void GraphicEngine::updateAmmoCount(WriteComponent &writeComp, WeaponComponent &weaponComp)
{
    writeComp.m_vectMessage[0].second = std::to_string(weaponComp.m_weaponsData[
                                                weaponComp.m_currentWeapon].m_ammunationsCount);
    writeComp.m_fontSpriteData[0] = m_ptrFontData->getWriteData(writeComp.m_vectMessage[0].second, writeComp, Font_e::STANDARD);
}

//===================================================================
void GraphicEngine::updatePlayerLife(WriteComponent &writeComp, PlayerConfComponent &playerComp)
{
    writeComp.m_vectMessage[0].second = std::to_string(playerComp.m_life);
    writeComp.m_fontSpriteData[0] = m_ptrFontData->getWriteData(writeComp.m_vectMessage[0].second, writeComp, Font_e::STANDARD);
}

//===================================================================
void GraphicEngine::fillTitleMenuWrite(WriteComponent &writeComp, MenuMode_e menuEntry, MenuMode_e previousMenuEntry)
{
    writeComp.m_upLeftPositionGL.first = -0.3f;
    switch (menuEntry)
    {
    case MenuMode_e::TITLE:
        writeComp.m_vectMessage[0].second = "ARNIHS";
        break;
    case MenuMode_e::BASE:
        writeComp.m_vectMessage[0].second = "MAIN MENU";
        break;
    case MenuMode_e::LOAD_GAME:
        writeComp.m_vectMessage[0].second = "LOAD GAME";
        break;
    case MenuMode_e::LOAD_CUSTOM_LEVEL:
        writeComp.m_vectMessage[0].second = "LOAD CUSTOM GAME";
        break;
    case MenuMode_e::CONFIRM_LOADING_GAME_FORM:
    {
        if(previousMenuEntry == MenuMode_e::LOAD_GAME)
        {
            writeComp.m_vectMessage[0].second = "LOAD GAME";
        }
        else if(previousMenuEntry == MenuMode_e::LOAD_GAME)
        {
            writeComp.m_vectMessage[0].second = "NEW GAME";
        }
        break;
    }
    case MenuMode_e::NEW_GAME:
        writeComp.m_vectMessage[0].second = "NEW GAME";
        break;
    case MenuMode_e::DISPLAY:
        writeComp.m_vectMessage[0].second = "GRAPHIC MENU";
        break;
    case MenuMode_e::INPUT:
    case MenuMode_e::NEW_KEY:
    case MenuMode_e::CONFIRM_QUIT_INPUT_FORM:
        writeComp.m_vectMessage[0].second = "INPUT MENU";
        break;
    case MenuMode_e::CONFIRM_RESTART_LEVEL:
        writeComp.m_vectMessage[0].second = "RESTART LEVEL";
        break;
    case MenuMode_e::CONFIRM_QUIT_GAME:
        writeComp.m_vectMessage[0].second = "QUIT GAME?";
        break;
    case MenuMode_e::CONFIRM_RESTART_FROM_LAST_CHECKPOINT:
        writeComp.m_upLeftPositionGL.first = -0.5f;
        writeComp.m_vectMessage[0].second = "RESTART FROM LAST CHECKPOINT";
        break;
    case MenuMode_e::SOUND:
        writeComp.m_vectMessage[0].second = "AUDIO MENU";
        break;
    case MenuMode_e::TRANSITION_LEVEL:
        writeComp.m_vectMessage[0].second = "";
        break;
    case MenuMode_e::LEVEL_PROLOGUE:
    case MenuMode_e::LEVEL_EPILOGUE:
        break;
    }
    writeComp.m_fontSpriteData[0] = m_ptrFontData->getWriteData(writeComp.m_vectMessage[0].second, writeComp, Font_e::STANDARD);
}

//===================================================================
void GraphicEngine::fillMenuWrite(WriteComponent &writeComp, MenuMode_e menuEntry, uint32_t cursorPos,
                                  const std::tuple<PlayerConfComponent *, uint32_t, uint32_t> &endLevelData)
{
    assert(!writeComp.m_vectMessage.empty());
    if(menuEntry == MenuMode_e::LOAD_GAME || menuEntry == MenuMode_e::NEW_GAME)
    {
        writeComp.m_vectMessage[0].second = m_saveStandardLevelMenuWrite;
    }
    else if(menuEntry == MenuMode_e::LOAD_CUSTOM_LEVEL)
    {
        writeComp.m_vectMessage[0].second = m_existingCustomLevelsMenuWrite[std::get<0>(endLevelData)->m_currentCustomLevelCusorMenu].first;
    }
    else if(menuEntry == MenuMode_e::TRANSITION_LEVEL)
    {
        writeComp.m_vectMessage[0].second = getEndLevelMenuStr(endLevelData);
    }
    else if(menuEntry == MenuMode_e::DISPLAY)
    {
        m_currentDisplayedResolution = m_currentResolution;
        m_displayMenuFullscreenMode = m_fullscreenMode;
        setCurrentResolution(m_currentDisplayedResolution);
        std::map<MenuMode_e, PairPairFloatStr_t>::const_iterator it = MAP_MENU_DATA.find(menuEntry);
        writeComp.m_vectMessage[0].second = it->second.second;
    }
    else if(menuEntry == MenuMode_e::NEW_KEY)
    {
        std::map<MenuMode_e, PairPairFloatStr_t>::const_iterator it = MAP_MENU_DATA.find(menuEntry);
        writeComp.m_vectMessage[0].second = it->second.second;
        writeComp.m_vectMessage[0].second += " " + m_mapInputActionStringAssociated.at(static_cast<InputMenuCursorPos_e>(cursorPos));
    }
    else if(menuEntry == MenuMode_e::LEVEL_PROLOGUE)
    {
        writeComp.m_vectMessage[0].second = m_levelPrologue + "\\\\Press Enter To Continue";
    }
    else if(menuEntry == MenuMode_e::LEVEL_EPILOGUE)
    {
        writeComp.m_vectMessage[0].second = m_levelEpilogue;
    }
    else
    {
        std::map<MenuMode_e, PairPairFloatStr_t>::const_iterator it = MAP_MENU_DATA.find(menuEntry);
        writeComp.m_vectMessage[0].second = it->second.second;
        writeComp.m_upLeftPositionGL.first = it->second.first.first;
        writeComp.m_vectMessage[0].first = it->second.first.first;
    }
    writeComp.m_fontSpriteData[0] = m_ptrFontData->getWriteData(writeComp.m_vectMessage[0].second,
            writeComp, Font_e::STANDARD);
}

//===================================================================
void GraphicEngine::confMenuSelectedLine(PlayerConfComponent &playerConf, WriteComponent &writeMenuSelectedComp,
                                         WriteComponent &writeMenuComp)
{
    if(writeMenuSelectedComp.m_vectMessage.empty())
    {
        writeMenuSelectedComp.addTextLine({{}, ""});
    }
    bool loadLevel = (playerConf.m_menuMode == MenuMode_e::NEW_GAME || playerConf.m_menuMode == MenuMode_e::LOAD_GAME);
    //Reinit base menu writing
    if(playerConf.m_menuMode == MenuMode_e::LOAD_CUSTOM_LEVEL)
    {
        writeMenuComp.m_vectMessage[0].second = m_existingCustomLevelsMenuWrite[playerConf.m_currentCustomLevelCusorMenu].first;
    }
    else if(loadLevel)
    {
        writeMenuComp.m_vectMessage[0].second = m_saveStandardLevelMenuWrite;
    }
    else
    {
        writeMenuComp.m_vectMessage[0].second = MAP_MENU_DATA.at(playerConf.m_menuMode).second;
    }
    std::pair<std::string, PairUI_t> ret = loadLevel ? getLineFromList(m_saveStandardLevelMenuWrite, playerConf.m_currentCursorPos) :
        getLineFromList(writeMenuComp.m_vectMessage[0].second, playerConf.m_currentCursorPos);

    if(playerConf.m_menuMode == MenuMode_e::LOAD_CUSTOM_LEVEL)
    {
        getLineFromList(m_existingCustomLevelsMenuWrite[playerConf.m_currentCustomLevelCusorMenu].first, playerConf.m_currentCustomLevelCusorMenu);
    }
    else if(loadLevel)
    {
        getLineFromList(m_saveStandardLevelMenuWrite, playerConf.m_currentCursorPos);
    }
    else
    {
        getLineFromList(writeMenuComp.m_vectMessage[0].second, playerConf.m_currentCursorPos);
    }
    //fill selected menu entry
    writeMenuSelectedComp.m_vectMessage[0].second = ret.first;
    if(playerConf.m_menuMode == MenuMode_e::TRANSITION_LEVEL || playerConf.m_menuMode == MenuMode_e::LEVEL_EPILOGUE ||
            playerConf.m_menuMode == MenuMode_e::LEVEL_PROLOGUE)
    {
        writeMenuSelectedComp.m_vectMessage[0].first = {};
    }
    else
    {
        writeMenuSelectedComp.m_vectMessage[0].first = writeMenuComp.m_vectMessage[0].first;
    }
    //remove base menu selected entry
    writeMenuComp.m_vectMessage[0].second.erase(ret.second.first, ret.second.second);
    writeMenuSelectedComp.m_fontSpriteData[0] = m_ptrFontData->getWriteData(writeMenuSelectedComp.m_vectMessage[0].second,
            writeMenuSelectedComp, Font_e::SELECTED);
}

//===================================================================
std::pair<std::string, PairUI_t> getLineFromList(const std::string &str, uint32_t lineNumber)
{
    bool endString = false;
    std::string::size_type posA = 0, posB = str.find("\\");
    if(posB == std::string::npos)
    {
        posB = str.size() - posA;
        return {str.substr(posA, posB), {posA, posB}};
    }
    ++posB;
    for(uint32_t i = 0; i < lineNumber; ++i)
    {
        posA = posB;
        posB = str.find("\\", ++posB);
        if(posB == std::string::npos)
        {
            posB = str.size();
            endString = true;
            break;
        }
        ++posB;
    }
    posB -= posA;
    if(endString)
    {
        return {str.substr(posA, posB), {posA, posB}};
    }
    return {str.substr(posA, posB), {posA, posB - 1}};
}

//===================================================================
void GraphicEngine::confWriteComponent(WriteComponent &writeComp)
{
    writeComp.m_fontSpriteData.clear();
    for(uint32_t i = 0; i < writeComp.m_vectMessage.size(); ++i)
    {
        writeComp.m_fontSpriteData.emplace_back(m_ptrFontData->getWriteData(writeComp.m_vectMessage[i].second, writeComp, Font_e::STANDARD));
    }
}

//===================================================================
void GraphicEngine::updateStringWriteEntitiesInputMenu(bool keyboardInputMenuMode, bool defaultInput)
{
    m_staticDisplaySystem->updateStringWriteEntitiesInputMenu(keyboardInputMenuMode, defaultInput);
}

//===================================================================
const std::vector<uint32_t> &GraphicEngine::getBarrelEntitiesToDelete()const
{
    return m_visionSystem->getBarrelEntitiesToDelete();
}

//===================================================================
void GraphicEngine::clearBarrelEntitiesToDelete()
{
    m_visionSystem->clearVectObjectToDelete();
}

//===================================================================
void GraphicEngine::updatePrologueAndEpilogue(const std::string &prologue, const std::string &epilogue)
{
    m_levelPrologue = prologue;
    m_levelEpilogue = epilogue;
}

//===================================================================
void GraphicEngine::memPlayerDatas(uint32_t playerEntity)
{
    m_firstPersonSystem->memPlayerEntity(playerEntity);
    m_staticDisplaySystem->memPlayerEntity(playerEntity);
}

//===================================================================
std::optional<uint32_t> GraphicEngine::getCustomLevelsMenuSize(uint32_t index)const
{
    if(index >= m_existingCustomLevelsMenuWrite.size())
    {
        return {};
    }
    return m_existingCustomLevelsMenuWrite[index].second;
}

//===================================================================
void GraphicEngine::toogleMenuEntryFullscreen()
{
    m_displayMenuFullscreenMode = !m_displayMenuFullscreenMode;
    m_staticDisplaySystem->updateMenuEntryFullscreen(m_displayMenuFullscreenMode);
}

//===================================================================
void GraphicEngine::setCurrentResolution(uint32_t resolution)
{
    m_currentDisplayedResolution = resolution;
    m_staticDisplaySystem->updateDisplayMenuResolution(m_memGraphicResolution[m_currentDisplayedResolution].second);
}

//===================================================================
void GraphicEngine::setSizeResolution(const pairI_t &resolution)
{
    for(uint32_t i = 0; i < m_memGraphicResolution.size(); ++i)
    {
        if(m_memGraphicResolution[i].first == resolution)
        {
            m_currentResolution = i;
            m_currentDisplayedResolution = i;
            glfwSetWindowSize(m_window, m_memGraphicResolution[m_currentResolution].first.first,
                              m_memGraphicResolution[m_currentResolution].first.second);
            break;
        }
    }
}

//===================================================================
void GraphicEngine::validDisplayMenu()
{
    if(m_currentResolution != m_currentDisplayedResolution)
    {
        m_currentResolution = m_currentDisplayedResolution;
        glfwSetWindowSize(m_window, m_memGraphicResolution[m_currentResolution].first.first,
                          m_memGraphicResolution[m_currentResolution].first.second);
    }
    if(m_fullscreenMode != m_displayMenuFullscreenMode)
    {
        toogleFullScreen();
    }
}

//===================================================================
void GraphicEngine::memGraphicResolutions()
{
    //get resolution
    int totalMonitor, resolutionCount;
    GLFWmonitor** monitors = glfwGetMonitors(&totalMonitor);
    assert(totalMonitor > 0);
    m_memGraphicResolution.clear();
    //one monitor
    const GLFWvidmode* modes = glfwGetVideoModes(monitors[0], &resolutionCount);
    m_memGraphicResolution.reserve(resolutionCount);
    pairI_t pair;
    for (int i = 0; i < resolutionCount; i++)
    {
        pair = {modes[i].width, modes[i].height};
        if(i > 0 && pair == m_memGraphicResolution.back().first)
        {
            continue;
        }
        m_memGraphicResolution.emplace_back(
                    std::pair<pairI_t, std::string>{pair, std::to_string(modes[i].width) + " X " + std::to_string(modes[i].height)});
    }
    assert(!m_memGraphicResolution.empty());
    //init basic resolution to 0
}

//===================================================================
void GraphicEngine::updateMusicVolumeBar(uint32_t volume)
{
    m_colorSystem->updateMusicVolumeBar(volume);
}

//===================================================================
void GraphicEngine::updateEffectsVolumeBar(uint32_t volume)
{
    m_colorSystem->updateEffectsVolumeBar(volume);
}

//===================================================================
void GraphicEngine::updateTurnSensitivityBar(uint32_t turnSensitivity)
{
    m_colorSystem->updateTurnSensitivityBar(turnSensitivity);
}

//===================================================================
void GraphicEngine::clearSystems()
{
    m_firstPersonSystem->clearBackgroundData();
    m_colorSystem->clearEntities();
    m_visionSystem->clearMemMultiSpritesWall();
}

//===================================================================
void GraphicEngine::memColorSystemEntity(uint32_t entity)
{
    m_colorSystem->addColorSystemEntity(entity);
}

//===================================================================
void GraphicEngine::memFogColorEntity(uint32_t entity)
{
    m_colorSystem->addFogColorEntity(entity);
}

//===================================================================
void GraphicEngine::memGroundBackgroundFPSSystemEntity(uint32_t entity, bool simpleTexture)
{
    m_firstPersonSystem->memGroundBackgroundEntity(entity, simpleTexture);
}

//===================================================================
void GraphicEngine::memCeilingBackgroundFPSSystemEntity(uint32_t entity, bool simpleTexture)
{
    m_firstPersonSystem->memCeilingBackgroundEntity(entity, simpleTexture);
}

//===================================================================
void GraphicEngine::initGLWindow()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif
    memGraphicResolutions();
    // glfw window creation
    // --------------------
    m_window = glfwCreateWindow(m_memGraphicResolution[m_currentResolution].first.first,
                                m_memGraphicResolution[m_currentResolution].first.second, "Nietsneflow", nullptr, nullptr);
    if(!m_window)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
    }
    glfwMakeContextCurrent(m_window);
    glfwSetFramebufferSizeCallback(m_window, framebuffer_size_callback);
}

//===================================================================
void GraphicEngine::initGlad()
{
    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
    }
}

//===================================================================
void GraphicEngine::updateGraphicCheckpointData(const MemCheckpointElementsState *checkpointData,
                                                uint32_t numSaveFile)
{
    m_memExistingLevelSave[numSaveFile - 1]->m_checkpointNum = checkpointData->m_checkpointNum;
}

//===================================================================
void GraphicEngine::initGLShader()
{
    m_vectShader.reserve(static_cast<uint32_t>(Shader_e::TOTAL_SHADER_S));
    for(uint32_t i = 0; i < static_cast<uint32_t>(Shader_e::TOTAL_SHADER_S); ++i)
    {
        std::map<Shader_e, std::string>::const_iterator it =
                SHADER_ID_MAP.find(static_cast<Shader_e>(i));
        std::string base = SHADER_DIR_STR + it->second;
        m_vectShader.emplace_back(Shader(base + std::string(".vs"),
                                         base + std::string(".fs")));
    }
    //take texture transparency into account
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
}

//===================================================================
void GraphicEngine::initGLTexture(const vectStr_t &texturePath)
{
    m_vectTexture.reserve(texturePath.size());
    for(uint32_t i = 0; i < texturePath.size(); ++i)
    {
        m_vectTexture.emplace_back(Texture(TEXTURES_DIR_STR + texturePath[i]));
    }
}

//===================================================================
void GraphicEngine::setShaderToLocalSystems()
{
    assert(m_colorSystem && "colorSystem is null");
    assert(m_mapSystem && "mapSystem is null");
    m_colorSystem->setShader(m_vectShader[static_cast<uint32_t>(Shader_e::COLOR_S)]);
    m_mapSystem->setShader(m_vectShader[static_cast<uint32_t>(Shader_e::TEXTURE_S)]);
    m_firstPersonSystem->addShaders(m_vectShader[static_cast<uint32_t>(Shader_e::COLORED_TEXTURE_S)],
            m_vectShader[static_cast<uint32_t>(Shader_e::TEXTURE_S)]);
    m_staticDisplaySystem->setShader(m_vectShader[static_cast<uint32_t>(Shader_e::TEXTURE_S)]);
}

//===================================================================
void GraphicEngine::loadSprites(const std::vector<SpriteData> &vectSprites,
                                const FontData &fontData)
{
    m_ptrSpriteData = &vectSprites;
    m_ptrFontData = &fontData;
}

//===================================================================
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
  // make sure the viewport matches the new window dimensions; note that width and
  // height will be significantly larger than specified on retina displays.
  glViewport(0, 0, width, height);
}

//===================================================================
std::string getEndLevelMenuStr(const std::tuple<PlayerConfComponent*, uint32_t, uint32_t> &endLevelData)
{
  if(std::get<0>(endLevelData)->m_life == 0)
    {
        return "You are dead\\Press Enter to Restart";
    }
    float enemiesKilledPercent =
            (!std::get<0>(endLevelData)->m_enemiesKilled || std::get<2>(endLevelData) == 0) ? EPSILON_FLOAT :
                static_cast<float>(*std::get<0>(endLevelData)->m_enemiesKilled) /
                static_cast<float>(std::get<2>(endLevelData)) * 100.0f,
            secretsFoundPercent =
            (!std::get<0>(endLevelData)->m_secretsFound || std::get<1>(endLevelData) == 0) ? EPSILON_FLOAT :
                static_cast<float>(*std::get<0>(endLevelData)->m_secretsFound) /
                static_cast<float>(std::get<1>(endLevelData)) * 100.0f;
    return "Enemies Killed:         " + std::to_string(static_cast<uint32_t>(enemiesKilledPercent)) +
       "%\\\\Secrets Found:         " + std::to_string(static_cast<uint32_t>(secretsFoundPercent)) +
                    "%\\\\Press Enter to Continue";
}
