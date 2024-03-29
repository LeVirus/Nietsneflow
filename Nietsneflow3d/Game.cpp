#include "Game.hpp"

//===================================================================
void Game::loadStandardData()
{
    m_levelManager.loadTextureData("pictureData.ini");
    m_levelManager.loadStandardData("standardData.ini");
    m_levelManager.loadFontData("fontData.ini", Font_e::BASE);
    m_levelManager.loadFontData("fontSelected.ini", Font_e::SELECTED);
    m_levelManager.loadFontData("fontStandard.ini", Font_e::STANDARD);
    m_mainEngine.loadGraphicPicture(m_levelManager.getPictureData(), m_levelManager.getFontData());
    m_mainEngine.loadExistingLevelNumSaves(m_levelManager.getExistingLevelNumSaves());
    m_levelManager.loadExistingCustomLevelFilename();
    m_mainEngine.loadExistingCustomLevel(m_levelManager.getExistingCustomLevel());
    m_mainEngine.confSystems();
}

//===================================================================
LevelState Game::displayTitleMenu()
{
    return m_mainEngine.displayTitleMenu(m_levelManager);
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
void Game::clearLevel(const LevelState &levelRet)
{
    if(levelRet.m_levelState == LevelState_e::RESTART_LEVEL || (levelRet.m_customLevel && levelRet.m_levelState == LevelState_e::LOAD_GAME) ||
            (m_mainEngine.isLoadFromCheckpoint() &&
             levelRet.m_levelState != LevelState_e::RESTART_FROM_CHECKPOINT &&
             levelRet.m_levelState != LevelState_e::GAME_OVER &&
             levelRet.m_levelState != LevelState_e::LOAD_GAME))
    {
        m_mainEngine.clearCheckpointData();
    }
    m_mainEngine.clearLevel();
}

//===================================================================
void Game::loadStandardEntities()
{
    m_mainEngine.loadColorEntities();
    m_mainEngine.loadStaticSpriteEntities(m_levelManager);
}

//===================================================================
void Game::loadPlayerEntity()
{
    m_mainEngine.loadPlayerEntity(m_levelManager);
}

//===================================================================
void Game::setPlayerDeparture()
{
    m_mainEngine.setPlayerDeparture(m_levelManager.getLevel().getPlayerDeparture(),
                                    m_levelManager.getLevel().getPlayerDepartureDirection());

}

//===================================================================
LevelLoadState_e Game::loadLevelData(uint32_t levelNum, bool customLevel, LevelState_e levelState)
{
    m_levelManager.clearExistingPositionsElement();
    LevelLoadState_e levelLoadState = m_levelManager.loadLevel(levelNum, customLevel);
    if(levelLoadState != LevelLoadState_e::OK)
    {
        if(levelLoadState != LevelLoadState_e::END)
        {
            m_mainEngine.setInfoDataWrite("ERROR : LEVEL COULD NOT BE LOADED");
        }
        return levelLoadState;
    }
    clearLevel({levelState, levelNum, customLevel});
    loadStandardEntities();
    //PLAYER DEPARTURE NOT SET
//    loadPlayerEntity();
    m_mainEngine.loadLevel(m_levelManager);
    return levelLoadState;
}

//===================================================================
void Game::initEngine()
{
    m_mainEngine.init(this);
}

//===================================================================
LevelState Game::launchGame(uint32_t levelNum, LevelState_e levelState, bool afterLoadFailure, bool customLevel)
{
    return m_mainEngine.mainLoop(levelNum, levelState, afterLoadFailure, customLevel);
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
                             const std::map<ControlKey_e, MouseKeyboardInputState> &keyboardArray)
{
    m_levelManager.saveInputSettings(gamepadArray, keyboardArray);
}

//===================================================================
void Game::saveTurnSensitivitySettings(uint32_t sensitivity)
{
    m_levelManager.saveTurnSensitivitySettings(sensitivity);
}

//===================================================================
std::string Game::saveGameProgressINI(const MemPlayerConf &playerConfBeginLevel, const MemPlayerConf &playerConfCheckpoint, uint32_t levelNum, uint32_t numSaveFile,
                                   const MemCheckpointElementsState *checkpointData)
{
    return m_levelManager.saveGameProgress(playerConfBeginLevel, playerConfCheckpoint, levelNum, numSaveFile, checkpointData);
}
