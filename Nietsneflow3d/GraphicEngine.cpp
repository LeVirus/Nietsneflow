#include "GraphicEngine.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
//#define STB_IMAGE_IMPLEMENTATION
//#include <stb_image.h>
#include <filesystem>
#include <cassert>
#include <constants.hpp>
#include <PictureData.hpp>

namespace fs = std::filesystem;

//===================================================================
GraphicEngine::GraphicEngine()
{
    initGLWindow();
    initGlad();
    initGLShader();
}

//===================================================================
void GraphicEngine::loadTextureAndSpriteData(const PictureData &pictureData)
{
    loadTextures(pictureData.getTexturePath());
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
    loadShaderPathsFromFS();
    size_t size = m_vectShaderPath.size();
    assert(size == Shader_e::TOTAL_SHADER);
    m_vectShader.reserve(size);
    for(uint32_t i = Shader_e::CEILING_FLOOR; i < Shader_e::TOTAL_SHADER; ++i)
    {
        for(uint32_t j = 0; j < m_vectShaderPath.size(); ++j)
        {
            const std::string &ref = m_vectShaderPath[j].first;
            std::map<Shader_e, std::string>::const_iterator it =
                    SHADER_ID_MAP.find(static_cast<Shader_e>(i));
            std::string sub = ref.substr(ref.find_last_of('/') + 1,
                                         it->second.size());
            if(sub == it->second)
            {
                m_vectShader.emplace_back(Shader(m_vectShaderPath[j].first,
                                                 m_vectShaderPath[j].second));
                break;
            }
        }
    }
    assert(m_vectShader.size() == Shader_e::TOTAL_SHADER && "Bad shader files.");
}

//===================================================================
void GraphicEngine::loadTextures(const vectStr_t &vectTextures)
{
    size_t size = vectTextures.size();
    m_vectTexture.reserve(size);
    for(uint32_t i = 0; i < size; ++i)
    {
        m_vectTexture.emplace_back(TEXTURES_DIR_STR + vectTextures[i]);
    }
}

//===================================================================
void GraphicEngine::loadShaderPathsFromFS()
{
    std::vector<std::string> vectStr;
    for(const fs::path &path : fs::directory_iterator(SHADER_DIR_STR))
    {
        std::string ext = path.extension();
        if(ext != ".fs" && ext != ".vs")
        {
            assert("Bad shader files in FS.");
        }
        std::string nameOnly = path.stem();
        std::string filename = path.filename();
        std::string fullPath = SHADER_DIR_STR;
        fullPath += filename;
        std::string fragPath = nameOnly + std::string(".fs");
        std::string vertPath = nameOnly + std::string(".vs");
        std::vector<std::string>::const_iterator it;
        if(ext == ".vs")
        {
            it = std::find(vectStr.begin(), vectStr.end(), fragPath);
        }
        else
        {
            it = std::find(vectStr.begin(), vectStr.end(), vertPath);
        }
        if(it != vectStr.end())
        {
            m_vectShaderPath.emplace_back(pairStr_t{SHADER_DIR_STR + vertPath, SHADER_DIR_STR + fragPath});
            vectStr.erase(it);
        }
        else
        {
            vectStr.emplace_back(filename);
        }
    }
    assert((vectStr.empty()) && "Error: incorrect number of file shader.");
}
