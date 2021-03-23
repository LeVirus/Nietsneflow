#pragma once

#include <functional>
#include <vector>
#include <string>
#include <PictureData.hpp>
#include <OpenGLUtils/Shader.hpp>
#include <OpenGLUtils/Texture.hpp>

using pairStr_t = std::pair<std::string, std::string>;
using pairI_t = std::pair<int, int>;
using vectPairStr_t = std::vector<pairStr_t>;
using vectStr_t = std::vector<std::string>;

struct GLFWwindow;
class PictureData;
class ColorDisplaySystem;
class MapDisplaySystem;
class FirstPersonDisplaySystem;
class VisionSystem;
class StaticDisplaySystem;

class GraphicEngine
{
private:
    GLFWwindow* m_window = nullptr;
    pairI_t m_screenSize = {800, 600};
    std::vector<Shader> m_vectShader;
    //PictureData
    std::vector<Texture> m_vectTexture;
    std::vector<SpriteData> const *m_ptrSpriteData = nullptr;
    //Systems
    ColorDisplaySystem *m_colorSystem = nullptr;
    MapDisplaySystem *m_mapSystem = nullptr;
    FirstPersonDisplaySystem *m_firstPersonSystem = nullptr;
    VisionSystem *m_visionSystem = nullptr;
    StaticDisplaySystem *m_staticDisplaySystem = nullptr;
private:
    void initGLWindow();
    void initGlad();
    void initGLShader();
    void initGLTexture();
    void setShaderToLocalSystems();
    void loadTexturesPath(const vectStr_t &vectTextures);
    void loadGroundAndCeiling(const GroundCeilingData &groundData,
                              const GroundCeilingData &ceilingData);
    void loadSprites(const std::vector<SpriteData> &vectSprites);
public:
    GraphicEngine();
    void confSystems();
    void loadPictureData(const PictureData &pictureData);
    void runIteration();
    bool windowShouldClose();
    void linkSystems(ColorDisplaySystem *colorSystem,
                     MapDisplaySystem *mapSystem,
                     FirstPersonDisplaySystem *firstPersonSystem,
                     VisionSystem *visionSystem,
                     StaticDisplaySystem *staticDisplaySystem);

    inline MapDisplaySystem &getMapDisplaySystem()
    {
        return *m_mapSystem;
    }

    inline FirstPersonDisplaySystem &getFirstPersonSystem()
    {
        return *m_firstPersonSystem;
    }

    inline GLFWwindow &getGLWindow()
    {
        return *m_window;
    }
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
