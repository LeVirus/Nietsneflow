#include "Game.hpp"

//===================================================================
void Game::loadStandardData()
{
    m_levelManager.loadTextureData("pictureData.ini");
    m_levelManager.loadStandardData("standardData.ini");
    m_levelManager.loadFontData("fontData.ini");
    m_mainEngine.loadGraphicPicture(m_levelManager.getPictureData(), m_levelManager.getFontData());
    m_mainEngine.confSystems();
}

void Game::clearLevel()
{
    m_mainEngine.clearLevel();
    m_levelManager.clearExistingPositionsElement();
}

//===================================================================
void Game::loadLevelData(uint32_t levelNum)
{
    m_levelManager.loadLevel("level.ini", levelNum);
    m_mainEngine.loadLevel(m_levelManager);
}

//===================================================================
void Game::initEngine()
{
    m_mainEngine.init();
}

//===================================================================
bool Game::launchGame(bool &memGameOver)
{
    return m_mainEngine.mainLoop(memGameOver);
}
