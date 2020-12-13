#pragma once

#include <MainEngine.hpp>
#include <LevelManager.hpp>

class Game
{
private:
    MainEngine m_mainEngine;
    LevelManager m_levelManager;
public:
    Game() = default;
    void loadLevelData(uint32_t levelNum);
    void initEngine();
    void launchGame();
};
