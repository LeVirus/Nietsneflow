#pragma once

#include <MainEngine.hpp>
#include <LevelManager.hpp>

class Game
{
public:
    Game() = default;
    void loadLevelData(uint32_t levelNum);
    void initEngine();
    inline void clearLevel()
    {
        m_mainEngine.clearLevel();
    }
    bool launchGame();
private:
    MainEngine m_mainEngine;
    LevelManager m_levelManager;
};
