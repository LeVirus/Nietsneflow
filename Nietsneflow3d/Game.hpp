#pragma once

#include <MainEngine.hpp>
#include <LevelManager.hpp>

class Game
{
public:
    Game() = default;
    bool loadLevelData(uint32_t levelNum);
    void initEngine();
    void loadStandardData();
    uint32_t displayTitleMenu();
    void loadSavedSettingsData();
    void clearLevel();
    void loadStandardEntities();
    LevelState launchGame(uint32_t levelNum, LevelState_e levelState);
    void saveAudioSettings(uint32_t musicVolume, uint32_t effectVolume);
    void saveDisplaySettings(const pairI_t &resolution, bool fullscreen);
    void saveInputSettings(const std::map<ControlKey_e, GamepadInputState> &gamepadArray,
                                  const std::map<ControlKey_e, uint32_t> &keyboardArray);
    std::string saveGameProgress(const MemPlayerConf &playerConfBeginLevel, const MemPlayerConf &playerConfCheckpoint, uint32_t levelNum, uint32_t numSaveFile,
                          const MemCheckpointElementsState *checkpointData = nullptr);
    inline std::optional<MemLevelLoadedData> loadSavedGame(uint32_t saveNum)
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
