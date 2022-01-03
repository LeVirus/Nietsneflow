#include "GraphicEngine.hpp"
#include "FontData.hpp"
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
    glfwSetWindowMonitor(m_window, m_fullscreenMode ? glfwGetPrimaryMonitor() : nullptr,
                         0, 0, m_screenSize.first, m_screenSize.second, GLFW_DONT_CARE);
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
void GraphicEngine::fillMenuWrite(WriteComponent *writeComp, MenuMode_e menuEntry)
{
    std::map<MenuMode_e, PairPairFloatStr_t>::const_iterator it = MAP_MENU_DATA.find(menuEntry);
    writeComp->m_str = it->second.second;
    writeComp->m_fontSpriteData = m_ptrFontData->getWriteData(writeComp->m_str, writeComp->m_numTexture);
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
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

    // glfw window creation
    // --------------------
    m_window = glfwCreateWindow(m_screenSize.first, m_screenSize.second, "Nietsneflow", nullptr, nullptr);
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
