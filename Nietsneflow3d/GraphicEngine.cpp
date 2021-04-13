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

//===================================================================
GraphicEngine::GraphicEngine()
{
    initGLWindow();
    initGlad();
    initGLShader();
    initGLTexture();
}

//===================================================================
void GraphicEngine::confSystems()
{
    setShaderToLocalSystems();
    m_mapSystem->setVectTextures(m_vectTexture);
    m_firstPersonSystem->setVectTextures(m_vectTexture);
    m_staticDisplaySystem->setVectTextures(m_vectTexture);
    m_staticDisplaySystem->setTextureWeaponNum(Texture_e::GLOBAL_T);
}

//===================================================================
void GraphicEngine::loadPictureData(const PictureData &pictureData, const FontData &fontData)
{
    loadSprites(pictureData.getSpriteData(), fontData);
}

//===================================================================
void GraphicEngine::runIteration()
{
    // render
    // ------
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    m_colorSystem->execSystem();
    m_visionSystem->execSystem();
    m_firstPersonSystem->execSystem();
    m_staticDisplaySystem->execSystem();
    m_mapSystem->execSystem();
    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
    // -------------------------------------------------------------------------------
    glfwSwapBuffers(m_window);
    glfwPollEvents();
}

//===================================================================
bool GraphicEngine::windowShouldClose()
{
    return glfwWindowShouldClose(m_window);
}

//===================================================================
void GraphicEngine::linkSystems(ColorDisplaySystem *colorSystem,
                                MapDisplaySystem *mapSystem,
                                FirstPersonDisplaySystem *firstPersonSystem,
                                VisionSystem *visionSystem,
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
                                    PlayerConfComponent *playerComp)
{
    std::string write = "AMMO::" + std::to_string(playerComp->m_ammunations[
                                                static_cast<uint32_t>(playerComp->m_currentWeapon)]);
    writeComp->m_fontSpriteData = m_ptrFontData->getWriteData(write);
}

//===================================================================
void GraphicEngine::updatePlayerLife(WriteComponent *writeComp, PlayerConfComponent *playerComp)
{
    std::string write = "LIFE::" + std::to_string(playerComp->m_life);
    writeComp->m_fontSpriteData = m_ptrFontData->getWriteData(write);
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
//    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
}

//===================================================================
void GraphicEngine::initGLTexture()
{
    m_vectTexture.reserve(static_cast<uint32_t>(Texture_e::TOTAL_TEXTURE_T));
    for(uint32_t i = 0; i < static_cast<uint32_t>(Texture_e::TOTAL_TEXTURE_T); ++i)
    {
        std::map<Texture_e, std::string>::const_iterator it =
                TEXTURE_ID_PATH_MAP.find(static_cast<Texture_e>(i));
        std::string path = TEXTURES_DIR_STR + it->second;
                m_vectTexture.emplace_back(Texture(path));
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
