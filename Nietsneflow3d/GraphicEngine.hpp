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
struct WriteComponent;
struct PlayerConfComponent;
class PictureData;
class MapDisplaySystem;
class FirstPersonDisplaySystem;
class VisionSystem;
class StaticDisplaySystem;
class FontData;
class ColorDisplaySystem;

using VectSpriteDataRef_t = std::vector<std::reference_wrapper<SpriteData>>;

class GraphicEngine
{
public:
    GraphicEngine();
    void confSystems();
    void loadPictureData(const PictureData &pictureData, const FontData &fontData);
    void runIteration(bool gamePaused);
    bool windowShouldClose();
    void linkSystems(ColorDisplaySystem *colorSystem, MapDisplaySystem *mapSystem,
                     FirstPersonDisplaySystem *firstPersonSystem, VisionSystem *visionSystem,
                     StaticDisplaySystem *staticDisplaySystem);
    void updateAmmoCount(WriteComponent *writeComp, PlayerConfComponent *playerComp);
    void updatePlayerLife(WriteComponent *writeComp, PlayerConfComponent *playerComp);
    void fillMenuWrite(WriteComponent *writeComp, const std::string &menuEntry);
    inline MapDisplaySystem &getMapSystem()
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
    void memColorSystemEntity(uint32_t entity);
    void memGroundBackgroundFPSSystemEntity(uint32_t entity);
    void memCeilingBackgroundFPSSystemEntity(uint32_t entity);
    void setTransition(bool gamePaused);
    void unsetTransition(bool gamePaused);
    void mainDisplay(bool gamePaused);
private:
    void preDisplay();
    void postDisplay();
    void displayGameIteration();
    void initGLWindow();
    void initGlad();
    void initGLShader();
    void initGLTexture(const vectStr_t &texturePath);
    void setShaderToLocalSystems();
    void loadGroundAndCeiling(const GroundCeilingData &groundData,
                              const GroundCeilingData &ceilingData);
    void loadSprites(const std::vector<SpriteData> &vectSprites,
                     const FontData &fontData);
private:
    GLFWwindow* m_window = nullptr;
    pairI_t m_screenSize = {800, 600};
    std::vector<Shader> m_vectShader;
    //PictureData
    std::vector<Texture> m_vectTexture;
    std::vector<SpriteData> const *m_ptrSpriteData = nullptr;
    FontData const *m_ptrFontData = nullptr;
    //Systems
    ColorDisplaySystem *m_colorSystem = nullptr;
    MapDisplaySystem *m_mapSystem = nullptr;
    FirstPersonDisplaySystem *m_firstPersonSystem = nullptr;
    VisionSystem *m_visionSystem = nullptr;
    StaticDisplaySystem *m_staticDisplaySystem = nullptr;
    uint32_t m_transitionFrameNumber = 30;
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
