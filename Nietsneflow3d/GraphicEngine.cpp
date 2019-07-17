#include "GraphicEngine.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cassert>
#include <constants.hpp>
#include <PictureData.hpp>

//===================================================================
GraphicEngine::GraphicEngine()
{
    initGLWindow();
    initGlad();
    initGLShader();
}

//===================================================================
void GraphicEngine::loadPictureData(const PictureData &pictureData)
{
    loadTexturesPath(pictureData.getTexturePath());
    loadSprites(pictureData.getSpriteData());
}

//TEST
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
  // make sure the viewport matches the new window dimensions; note that width and
  // height will be significantly larger than specified on retina displays.
  glViewport(0, 0, width, height);
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
    m_window = glfwCreateWindow(/*SCR_WIDTH*/600, /*SCR_HEIGHT*/800, "Nietsneflow", NULL, NULL);
    if(!m_window)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
    }
    glfwMakeContextCurrent(m_window);
    glfwSetFramebufferSizeCallback(m_window, framebuffer_size_callback);



    //TEST
//    while (!glfwWindowShouldClose(m_window))
//      {
//        // input
//        // -----
//        if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
//            glfwSetWindowShouldClose(m_window, true);

//        // render
//        // ------
//        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
//        glClear(GL_COLOR_BUFFER_BIT);

//        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
//        // -------------------------------------------------------------------------------
//        glfwSwapBuffers(m_window);
//        glfwPollEvents();
//      }
    //FIN test
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
    m_vectShader.reserve(Shader_e::TOTAL_SHADER);
    for(uint32_t i = Shader_e::CEILING_FLOOR; i < Shader_e::TOTAL_SHADER; ++i)
    {
        std::map<Shader_e, std::string>::const_iterator it =
                SHADER_ID_MAP.find(static_cast<Shader_e>(i));
        std::string base = SHADER_DIR_STR + it->second;
        m_vectShader.emplace_back(Shader(base + std::string(".vs"),
                                         base + std::string(".fs")));
    }
    assert(m_vectShader.size() == Shader_e::TOTAL_SHADER && "Bad shader files.");
}

//===================================================================
void GraphicEngine::loadTexturesPath(const vectStr_t &vectTextures)
{
    size_t size = vectTextures.size();
    assert(size == Texture_t::TOTAL_TEXTURE);
    m_vectTexture.reserve(size);
    for(uint32_t i = 0; i < size; ++i)
    {
        m_vectTexture.emplace_back(TEXTURES_DIR_STR + vectTextures[i]);
    }
}

//===================================================================
void GraphicEngine::loadGroundAndCeiling(const GroundCeilingData &groundData,
                                         const GroundCeilingData &ceilingData)
{
    m_groundData = &groundData;
    m_ceilingData = &ceilingData;
}

//===================================================================
void GraphicEngine::loadSprites(const std::vector<SpriteData> &vectSprites)
{
    m_ptrSpriteData = &vectSprites;
}
