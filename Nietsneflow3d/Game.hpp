#ifndef GAME_H
#define GAME_H

#include <MainEngine.hpp>
#include <LevelManager.hpp>

class Game
{
private:
    MainEngine m_mainEngine;
    LevelManager m_levelManager;
public:
    Game();
    void loadLevelData();
    void initEngine();
    void launchGame();
};

#endif // GAME_H
