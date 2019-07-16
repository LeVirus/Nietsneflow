#include "Game.hpp"

//===================================================================
Game::Game()
{

}

//===================================================================
void Game::loadLevelData()
{
    m_levelManager.loadTextureData("pictureData.ini", 1);
    m_levelManager.loadLevel("level.ini", 1);
}

//===================================================================
void Game::initEngine()
{
//    m_mainEngine.testGraphicEngine();
}

