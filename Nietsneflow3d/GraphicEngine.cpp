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
    m_saveMenuWrite.clear();
    std::string checkpoint;
    for(uint32_t i = 1; i < 4; ++i)
    {
        m_saveMenuWrite += std::to_string(i);
        if(existingLevelNum[i - 1])
        {
            checkpoint = (existingLevelNum[i - 1]->m_checkpointNum == 0) ? "" :
                " CHCKPT " + std::to_string(existingLevelNum[i - 1]->m_checkpointNum);
            m_saveMenuWrite += "  LVL " + std::to_string(existingLevelNum[i - 1]->m_levelNum) +
                    checkpoint + " " + existingLevelNum[i - 1]->m_date;
        }
        m_saveMenuWrite += "\\";
    }
    m_saveMenuWrite += "RETURN";
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
void GraphicEngine::setTransition(bool gamePaused)
{
    for(uint32_t i = 0; i < m_transitionFrameNumber; ++i)
    {
        preDisplay();
        mainDisplay(gamePaused);
        m_colorSystem->setTransition(i, m_transitionFrameNumber);
        postDisplay();
    }
}

//===================================================================
void GraphicEngine::unsetTransition(bool gamePaused)
{
    for(uint32_t i = m_transitionFrameNumber; i > 0; --i)
    {
        preDisplay();
        mainDisplay(gamePaused);
        m_colorSystem->setTransition(i, m_transitionFrameNumber);
        postDisplay();
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
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
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
    m_mapSystem->execSystem();
    m_staticDisplaySystem->execSystem();
}

//===================================================================
bool GraphicEngine::windowShouldClose()
{
    return glfwWindowShouldClose(m_window);
}

//===================================================================
void GraphicEngine::updateSaveNum(uint32_t levelNum, uint32_t saveNum, std::optional<uint32_t> checkpointNum,
                                  const std::string &date)
{
    m_memExistingLevelSave[saveNum - 1]->m_levelNum = levelNum;
    if(!date.empty())
    {
        m_memExistingLevelSave[saveNum - 1]->m_date = date;
    }
    if(checkpointNum)
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
void GraphicEngine::updateAmmoCount(WriteComponent *writeComp,
                                    WeaponComponent *weaponComp)
{
    writeComp->m_str = STR_PLAYER_AMMO + std::to_string(weaponComp->m_weaponsData[
                                                weaponComp->m_currentWeapon].m_ammunationsCount);
    writeComp->m_fontSpriteData = m_ptrFontData->getWriteData(writeComp->m_str, writeComp->m_numTexture);
}

//===================================================================
void GraphicEngine::updatePlayerLife(WriteComponent *writeComp, PlayerConfComponent *playerComp)
{
    writeComp->m_str = STR_PLAYER_LIFE + std::to_string(playerComp->m_life);
    writeComp->m_fontSpriteData = m_ptrFontData->getWriteData(writeComp->m_str, writeComp->m_numTexture);
}

//===================================================================
void GraphicEngine::fillTitleMenuWrite(WriteComponent *writeComp, MenuMode_e menuEntry, MenuMode_e previousMenuEntry)
{
    writeComp->m_upLeftPositionGL.first = -0.3f;
    switch (menuEntry)
    {
    case MenuMode_e::TITLE:
        writeComp->m_str = "TITLE MENU";
        break;
    case MenuMode_e::BASE:
        writeComp->m_str = "MAIN MENU";
        break;
    case MenuMode_e::LOAD_GAME:
        writeComp->m_str = "LOAD GAME";
        break;
    case MenuMode_e::CONFIRM_LOADING_GAME_FORM:
    {
        if(previousMenuEntry == MenuMode_e::LOAD_GAME)
        {
            writeComp->m_str = "LOAD GAME";
        }
        else if(previousMenuEntry == MenuMode_e::LOAD_GAME)
        {
            writeComp->m_str = "NEW GAME";
        }
        break;
    }
    case MenuMode_e::NEW_GAME:
        writeComp->m_str = "NEW GAME";
        break;
    case MenuMode_e::DISPLAY:
        writeComp->m_str = "GRAPHIC MENU";
        break;
    case MenuMode_e::INPUT:
    case MenuMode_e::NEW_KEY:
    case MenuMode_e::CONFIRM_QUIT_INPUT_FORM:
        writeComp->m_str = "INPUT MENU";
        break;
    case MenuMode_e::CONFIRM_RESTART_LEVEL:
        writeComp->m_str = "RESTART LEVEL";
        break;
    case MenuMode_e::CONFIRM_RESTART_FROM_LAST_CHECKPOINT:
        writeComp->m_upLeftPositionGL.first = -0.5f;
        writeComp->m_str = "RESTART FROM LAST CHECKPOINT";
        break;
    case MenuMode_e::SOUND:
        writeComp->m_str = "AUDIO MENU";
        break;
    case MenuMode_e::TRANSITION_LEVEL:
        writeComp->m_str = "";
        break;
    }
    writeComp->m_fontSpriteData = m_ptrFontData->getWriteData(writeComp->m_str, writeComp->m_numTexture);
}

//===================================================================
void GraphicEngine::fillMenuWrite(WriteComponent *writeComp, MenuMode_e menuEntry, uint32_t cursorPos,
                                  const std::tuple<const PlayerConfComponent*, uint32_t, uint32_t> &endLevelData)
{
    if(menuEntry == MenuMode_e::LOAD_GAME || menuEntry == MenuMode_e::NEW_GAME)
    {
        writeComp->m_str = m_saveMenuWrite;
    }
    else if(menuEntry == MenuMode_e::TRANSITION_LEVEL)
    {
        writeComp->m_str = getEndLevelMenuStr(endLevelData);
    }
    else if(menuEntry == MenuMode_e::DISPLAY)
    {
        m_currentDisplayedResolution = m_currentResolution;
        m_displayMenuFullscreenMode = m_fullscreenMode;
        setCurrentResolution(m_currentDisplayedResolution);
        std::map<MenuMode_e, PairPairFloatStr_t>::const_iterator it = MAP_MENU_DATA.find(menuEntry);
        writeComp->m_str = it->second.second;
    }
    else if(menuEntry == MenuMode_e::NEW_KEY)
    {
        std::map<MenuMode_e, PairPairFloatStr_t>::const_iterator it = MAP_MENU_DATA.find(menuEntry);
        writeComp->m_str = it->second.second;
        writeComp->m_str += " " + m_mapInputActionStringAssociated.at(static_cast<InputMenuCursorPos_e>(cursorPos));
    }
    else
    {
        std::map<MenuMode_e, PairPairFloatStr_t>::const_iterator it = MAP_MENU_DATA.find(menuEntry);
        writeComp->m_str = it->second.second;
    }
    writeComp->m_fontSpriteData = m_ptrFontData->getWriteData(writeComp->m_str, writeComp->m_numTexture);
}

//===================================================================
void GraphicEngine::confWriteComponent(WriteComponent *writeComp)
{
    writeComp->m_fontSpriteData = m_ptrFontData->getWriteData(writeComp->m_str, writeComp->m_numTexture);
}

//===================================================================
void GraphicEngine::updateStringWriteEntitiesInputMenu(bool keyboardInputMenuMode, bool defaultInput)
{
    m_staticDisplaySystem->updateStringWriteEntitiesInputMenu(keyboardInputMenuMode, defaultInput);
}

//===================================================================
const std::vector<uint32_t> &GraphicEngine::getBarrelEntitiesToDelete() const
{
    return m_visionSystem->getBarrelEntitiesToDelete();
}

//===================================================================
void GraphicEngine::clearBarrelEntitiesToDelete()
{
    m_visionSystem->clearVectObjectToDelete();
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
    m_firstPersonSystem->setShader(m_vectShader[static_cast<uint32_t>(Shader_e::TEXTURE_S)]);
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
std::string getEndLevelMenuStr(const std::tuple<const PlayerConfComponent*, uint32_t, uint32_t> &endLevelData)
{
    float enemiesKilledPercent =
            (!std::get<0>(endLevelData)->m_enemiesKilled || std::get<2>(endLevelData) == 0) ? EPSILON_FLOAT :
                static_cast<float>(*std::get<0>(endLevelData)->m_enemiesKilled) /
                static_cast<float>(std::get<2>(endLevelData)) * 100.0f,
            secretsFoundPercent =
            (!std::get<0>(endLevelData)->m_secretsFound || std::get<1>(endLevelData) == 0) ? EPSILON_FLOAT :
                static_cast<float>(*std::get<0>(endLevelData)->m_secretsFound) /
                static_cast<float>(std::get<1>(endLevelData)) * 100.0f;
    return "ENEMIES KILLED :         " + std::to_string(static_cast<uint32_t>(enemiesKilledPercent)) +
       "%\\\\SECRETS FOUND :         " + std::to_string(static_cast<uint32_t>(secretsFoundPercent)) +
                    "%\\\\PRESS ENTER TO CONTINUE";
}
