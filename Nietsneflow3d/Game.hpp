#pragma once

#include <MainEngine.hpp>
#include <LevelManager.hpp>

class Game
{
public:
    Game() = default;
    LevelLoadState_e loadLevelData(uint32_t levelNum, bool customLevel);
    void initEngine();
    void loadStandardData();
    LevelState displayTitleMenu();
    void loadSavedSettingsData();
    void clearLevel(const LevelState &levelRet);
    void loadStandardEntities();
    void loadPlayerEntity();
    void setPlayerDeparture();
    LevelState launchGame(uint32_t levelNum, LevelState_e levelState, bool afterLoadFailure);
    void saveAudioSettings(uint32_t musicVolume, uint32_t effectVolume);
    void saveDisplaySettings(const pairI_t &resolution, bool fullscreen);
    void saveInputSettings(const std::map<ControlKey_e, GamepadInputState> &gamepadArray,
                           const std::map<ControlKey_e, MouseKeyboardInputState> &keyboardArray);
    void saveTurnSensitivitySettings(uint32_t sensitivity);
    std::string saveGameProgressINI(const MemPlayerConf &playerConfBeginLevel, const MemPlayerConf &playerConfCheckpoint, uint32_t levelNum, uint32_t numSaveFile,
                          const MemCheckpointElementsState *checkpointData = nullptr);
    inline std::optional<MemLevelLoadedData> loadSavedGame(uint32_t saveNum)
    {
        return m_levelManager.loadSavedGame(saveNum);
    }
    inline bool checkSavedGameExists(uint32_t saveNum)const
    {
        return m_levelManager.checkSavedGameExists(saveNum);
    }
    inline void unsetFirstLaunch()
    {
        m_mainEngine.unsetFirstLaunch();
    }
    inline void setUnsetPaused()
    {
        m_mainEngine.setUnsetPaused();
    }
private:
    MainEngine m_mainEngine;
    LevelManager m_levelManager;
};
