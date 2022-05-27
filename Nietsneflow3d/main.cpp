#include <Game.hpp>
#include <MainEngine.hpp>

int main()
{
    uint32_t levelIndex = 1;
    Game game;
    game.initEngine();
    game.loadStandardData();
    bool gameLoaded = false, firstLaunch = true;
    LevelState levelState = {LevelState_e::NEW_GAME, {}, false};
    do
    {
        game.loadStandardEntities();
        if(firstLaunch)
        {
            //PLAYER DEPARTURE NOT SET
            game.loadPlayerEntity();
            if(!gameLoaded)
            {
                game.loadSavedSettingsData();
                gameLoaded = true;
            }
            levelState = game.displayTitleMenu();
            if(levelState.m_levelState == LevelState_e::EXIT)
            {
                break;
            }
            levelIndex = *levelState.m_levelToLoad;
            //if no more level
            if(!game.loadLevelData(levelIndex, levelState.m_customLevel))
            {
                break;
            }
            game.setPlayerDeparture();
            firstLaunch = false;
        }
        else
        {
            //if no more level
            if(!game.loadLevelData(levelIndex, levelState.m_customLevel))
            {
                break;
            }
            if(!gameLoaded)
            {
                game.loadSavedSettingsData();
                gameLoaded = true;
            }
        }
        game.unsetFirstLaunch();
        levelState = game.launchGame(levelIndex, levelState.m_levelState);
        switch(levelState.m_levelState)
        {
        case LevelState_e::EXIT:
            break;
        case LevelState_e::NEW_GAME:
            levelIndex = 1;
            break;
        case LevelState_e::LEVEL_END:
            ++levelIndex;
            break;
        case LevelState_e::GAME_OVER:
            break;
        case LevelState_e::RESTART_FROM_CHECKPOINT:
        case LevelState_e::RESTART_LEVEL:
        case LevelState_e::LOAD_GAME:
        {
            assert(levelState.m_levelToLoad);
            //reloop on the specific level
            levelIndex = *levelState.m_levelToLoad;
            break;
        }
        }
        //quit
        if(levelState.m_levelState == LevelState_e::EXIT)
        {
            break;
        }
        game.clearLevel();
    }while(true);
    game.clearLevel();
    return 0;
}
