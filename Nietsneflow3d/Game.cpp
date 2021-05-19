#include "Game.hpp"

//===================================================================
void Game::loadLevelData(uint32_t levelNum)
{
    m_levelManager.loadTextureData("pictureData.ini");
    m_levelManager.loadLevel("level.ini", levelNum);
    m_levelManager.loadFontData("fontData.ini");
    m_mainEngine.loadGraphicPicture(m_levelManager.getPictureData(), m_levelManager.getFontData());
    m_mainEngine.loadLevelEntities(m_levelManager);
    m_mainEngine.confSystems();
}

//===================================================================
void Game::initEngine()
{
    m_mainEngine.init();
}

//===================================================================
bool Game::launchGame()
{
    return m_mainEngine.mainLoop();
}
