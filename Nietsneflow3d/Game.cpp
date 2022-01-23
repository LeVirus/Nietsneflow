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

//===================================================================
void Game::loadSavedSettingsData()
{
    if(!m_levelManager.loadSettingsData())
    {
        return;
    }
    m_mainEngine.confGlobalSettings(m_levelManager.getSettingsData());
}

//===================================================================
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
    m_mainEngine.init(this);
}

//===================================================================
std::pair<bool, bool> Game::launchGame()
{
    return m_mainEngine.mainLoop();
}

//===================================================================
void Game::saveAudioSettings(uint32_t musicVolume, uint32_t effectVolume)
{
    m_levelManager.saveAudioSettings(musicVolume, effectVolume);
}

//===================================================================
void Game::saveDisplaySettings(const pairI_t &resolution, bool fullscreen)
{
    m_levelManager.saveDisplaySettings(resolution, fullscreen);

}

//===================================================================
void Game::saveInputSettings(const std::map<ControlKey_e, GamepadInputState> &gamepadArray,
                             const std::map<ControlKey_e, uint32_t> &keyboardArray)
{
    m_levelManager.saveInputSettings(gamepadArray, keyboardArray);
}
