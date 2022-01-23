#pragma once

#include <MainEngine.hpp>
#include <LevelManager.hpp>

class Game
{
public:
    Game() = default;
    void loadLevelData(uint32_t levelNum);
    void initEngine();
    void loadStandardData();
    void loadSavedSettingsData();
    void clearLevel();
    std::pair<bool, bool> launchGame();
    void saveAudioSettings(uint32_t musicVolume, uint32_t effectVolume);
    void saveDisplaySettings(const pairI_t &resolution, bool fullscreen);
    void saveInputSettings(const std::map<ControlKey_e, GamepadInputState> &gamepadArray,
                                  const std::map<ControlKey_e, uint32_t> &keyboardArray);
private:
    MainEngine m_mainEngine;
    LevelManager m_levelManager;
};
