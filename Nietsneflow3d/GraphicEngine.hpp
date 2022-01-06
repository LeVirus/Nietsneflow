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
using VectSpriteDataRef_t = std::vector<std::reference_wrapper<SpriteData>>;

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
struct WeaponComponent;

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
    void updateAmmoCount(WriteComponent *writeComp, WeaponComponent *weaponComp);
    void updatePlayerLife(WriteComponent *writeComp, PlayerConfComponent *playerComp);
    void fillMenuWrite(WriteComponent *writeComp, MenuMode_e menuEntry);
    void confWriteComponent(WriteComponent *writeComp);
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
    inline const std::vector<std::pair<pairI_t, std::string>> &getResolutions()const
    {
        return m_memGraphicResolution;
    }
    inline uint32_t getCurrentResolutionNum()const
    {
        return m_currentResolution;
    }
    inline uint32_t getCurrentDisplayedResolutionNum()const
    {
        return m_currentDisplayedResolution;
    }
    inline uint32_t getMaxResolutionNum()const
    {
        return m_memGraphicResolution.size() - 1;
    }
    void decreaseMenuDisplayQuality();
    void increaseMenuDisplayQuality();
    void toogleMenuEntryFullscreen();
    void setCurrentMenuResolution(uint32_t resolution);
    void updateMusicVolumeBar(uint32_t volume);
    void updateEffectsVolumeBar(uint32_t volume);
    void clearSystems();
    void memColorSystemEntity(uint32_t entity);
    void memGroundBackgroundFPSSystemEntity(uint32_t entity, bool simpleTexture);
    void memCeilingBackgroundFPSSystemEntity(uint32_t entity, bool simpleTexture);
    void setTransition(bool gamePaused);
    void unsetTransition(bool gamePaused);
    void mainDisplay(bool gamePaused);
    void toogleFullScreen();
    void validDisplayMenu();
private:
    void preDisplay();
    void postDisplay();
    void memGraphicResolutions();
    void displayGameIteration();
    void initGLWindow();
    void initGlad();
    void initGLShader();
    void initGLTexture(const vectStr_t &texturePath);
    void setShaderToLocalSystems();
    void loadSprites(const std::vector<SpriteData> &vectSprites, const FontData &fontData);
private:
    GLFWwindow* m_window = nullptr;
    uint32_t m_currentResolution = 0, m_currentDisplayedResolution = m_currentResolution,
    m_currentQuality = 1, m_currentDisplayedQuality = m_currentQuality;
    std::vector<Shader> m_vectShader;
    //PictureData
    std::vector<Texture> m_vectTexture;
    std::vector<SpriteData> const *m_ptrSpriteData = nullptr;
    std::vector<std::pair<pairI_t, std::string>> m_memGraphicResolution;
    std::vector<std::string> m_qualityResolution = {"LOW", "MEDIUM", "HIGH"};
    FontData const *m_ptrFontData = nullptr;
    //Systems
    ColorDisplaySystem *m_colorSystem = nullptr;
    MapDisplaySystem *m_mapSystem = nullptr;
    FirstPersonDisplaySystem *m_firstPersonSystem = nullptr;
    VisionSystem *m_visionSystem = nullptr;
    StaticDisplaySystem *m_staticDisplaySystem = nullptr;
    uint32_t m_transitionFrameNumber = 30;
    bool m_fullscreenMode = false, m_displayMenuFullscreenMode = m_fullscreenMode;
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
