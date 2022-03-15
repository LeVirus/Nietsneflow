#pragma once

#include <functional>
#include <vector>
#include <string>
#include <PictureData.hpp>
#include <LevelManager.hpp>
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
    void loadExistingLevelNumSaves(const std::array<std::optional<DataLevelWriteMenu>, 3> &existingLevelNum);
    void confSystems();
    void loadPictureData(const PictureData &pictureData, const FontData &fontData);
    void runIteration(bool gamePaused);
    bool windowShouldClose();
    void updateSaveNum(uint32_t levelNum, uint32_t saveNum, std::optional<uint32_t> checkpointNum,
                       const std::string &date = "");
    void linkSystems(ColorDisplaySystem *colorSystem, MapDisplaySystem *mapSystem,
                     FirstPersonDisplaySystem *firstPersonSystem, VisionSystem *visionSystem,
                     StaticDisplaySystem *staticDisplaySystem);
    void updateAmmoCount(WriteComponent *writeComp, WeaponComponent *weaponComp);
    void updatePlayerLife(WriteComponent *writeComp, PlayerConfComponent *playerComp);
    void fillTitleMenuWrite(WriteComponent *writeComp, MenuMode_e menuEntry);
    void fillMenuWrite(WriteComponent *writeComp, MenuMode_e menuEntry, uint32_t cursorPos = 0);
    void confWriteComponent(WriteComponent *writeComp);
    void updateStringWriteEntitiesInputMenu(bool keyboardInputMenuMode, bool defaultInput = true);
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
    inline bool fullscreenMode()const
    {
        return m_fullscreenMode;
    }
    void toogleMenuEntryFullscreen();
    void setCurrentResolution(uint32_t resolution);
    void setSizeResolution(const pairI_t &resolution);
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
    void updateGraphicCheckpointData(const MemCheckpointElementsState *checkpointData, uint32_t numSaveFile);
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
    uint32_t m_currentResolution = 0, m_currentDisplayedResolution = m_currentResolution;
    std::vector<Shader> m_vectShader;
    //PictureData
    std::vector<Texture> m_vectTexture;
    std::vector<SpriteData> const *m_ptrSpriteData = nullptr;
    std::vector<std::pair<pairI_t, std::string>> m_memGraphicResolution;
    FontData const *m_ptrFontData = nullptr;
    //Systems
    ColorDisplaySystem *m_colorSystem = nullptr;
    MapDisplaySystem *m_mapSystem = nullptr;
    FirstPersonDisplaySystem *m_firstPersonSystem = nullptr;
    VisionSystem *m_visionSystem = nullptr;
    StaticDisplaySystem *m_staticDisplaySystem = nullptr;
    uint32_t m_transitionFrameNumber = 30;
    std::array<std::optional<DataLevelWriteMenu>, 3> m_memExistingLevelSave;
    std::string m_saveMenuWrite;
    bool m_fullscreenMode = false, m_displayMenuFullscreenMode = m_fullscreenMode;
    const std::map<InputMenuCursorPos_e, std::string> m_mapInputActionStringAssociated = {
        {InputMenuCursorPos_e::ACTION, "ACTION"},
        {InputMenuCursorPos_e::MOVE_BACKWARD, "MOVE BACKWARD"},
        {InputMenuCursorPos_e::MOVE_FORWARD, "MOVE FORWARD"},
        {InputMenuCursorPos_e::NEXT_WEAPON, "NEXT WEAPON"},
        {InputMenuCursorPos_e::PREVIOUS_WEAPON, "PREVIOUS WEAPON"},
        {InputMenuCursorPos_e::SHOOT, "SHOOT"},
        {InputMenuCursorPos_e::STRAFE_LEFT, "STRAFE LEFT"},
        {InputMenuCursorPos_e::STRAFE_RIGHT, "STRAFE RIGHT"},
        {InputMenuCursorPos_e::TURN_LEFT, "TURN LEFT"},
        {InputMenuCursorPos_e::TURN_RIGHT, "TURN RIGHT"}
    };
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
