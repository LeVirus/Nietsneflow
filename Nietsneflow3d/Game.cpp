#include "Game.hpp"

//===================================================================
Game::Game()
{

}

//===================================================================
void Game::loadLevelData(uint32_t levelNum)
{
    m_levelManager.loadTextureData("pictureData.ini", levelNum);
    m_levelManager.loadLevel("level.ini", levelNum);
    m_mainEngine.loadGraphicPicture(m_levelManager.getPictureData());
    m_mainEngine.loadLevelEntities(m_levelManager);
}

//===================================================================
void Game::initEngine()
{
    m_mainEngine.init();
}

//===================================================================
void Game::launchGame()
{
    m_mainEngine.launchLoop();
}
