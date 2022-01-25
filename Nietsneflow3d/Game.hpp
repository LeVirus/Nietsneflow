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
    std::tuple<bool, bool, std::optional<uint32_t> > launchGame(uint32_t levelNum);
    void saveAudioSettings(uint32_t musicVolume, uint32_t effectVolume);
    void saveDisplaySettings(const pairI_t &resolution, bool fullscreen);
    void saveInputSettings(const std::map<ControlKey_e, GamepadInputState> &gamepadArray,
                                  const std::map<ControlKey_e, uint32_t> &keyboardArray);
    void saveGameProgress(const MemPlayerConf &playerConf, uint32_t levelNum, std::optional<uint32_t> numSaveFile = {});
    inline std::optional<std::pair<uint32_t, MemPlayerConf>> loadSavedGame(uint32_t saveNum)
    {
        return m_levelManager.loadSavedGame(saveNum);
    }
    inline bool checkSavedGameExists(uint32_t saveNum)const
    {
        return m_levelManager.checkSavedGameExists(saveNum);
    }
private:
    MainEngine m_mainEngine;
    LevelManager m_levelManager;
};
