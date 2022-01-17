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
    bool launchGame(bool &memGameOver);
private:
    MainEngine m_mainEngine;
    LevelManager m_levelManager;
};
