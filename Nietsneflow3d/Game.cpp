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
bool Game::loadLevelData(uint32_t levelNum)
{
    if(!m_levelManager.loadLevel("level.ini", levelNum))
    {
        return false;
    }
    m_mainEngine.loadLevel(m_levelManager);
    return true;
}

//===================================================================
void Game::initEngine()
{
    m_mainEngine.init(this);
}

//===================================================================
std::tuple<bool, bool, std::optional<uint32_t>> Game::launchGame(uint32_t levelNum, bool gameLoad)
{
    return m_mainEngine.mainLoop(levelNum, gameLoad);
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

//===================================================================
void Game::saveGameProgress(const MemPlayerConf &playerConf, uint32_t levelNum, uint32_t numSaveFile)
{
    m_levelManager.saveGameProgress(playerConf, levelNum, numSaveFile);
}
