#include "Game.hpp"

Game::Game()
{

}

void Game::loadLevelData()
{
    m_levelManager.loadTextureData("pictureData.ini");
    m_levelManager.loadLevel("level.ini");
}

