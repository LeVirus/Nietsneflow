#include <Game.hpp>
#include <MainEngine.hpp>

int main()
{
    uint32_t levelIndex = 1, memPreviousLevel;
    Game game;
    game.initEngine();
    game.loadStandardData();
    bool gameLoaded = false, firstLaunch = true;
    LevelState levelState = {LevelState_e::NEW_GAME, {}, false};
    LevelLoadState_e levelRetState = LevelLoadState_e::OK;
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
            levelRetState = game.loadLevelData(levelIndex, levelState.m_customLevel);
            //if no more level
            if(levelRetState == LevelLoadState_e::END)
            {
                break;
            }
            else if(levelRetState == LevelLoadState_e::OK)
            {
                memPreviousLevel = levelIndex;
                game.setPlayerDeparture();
                firstLaunch = false;
            }
            else if(levelRetState == LevelLoadState_e::FAIL)
            {
                continue;
            }
        }
        else
        {
            if(levelRetState != LevelLoadState_e::FAIL)
            {
                levelRetState = game.loadLevelData(levelIndex, levelState.m_customLevel);
                //if no more level
                if(levelRetState == LevelLoadState_e::END)
                {
                    break;
                }
                else if(levelRetState == LevelLoadState_e::OK)
                {
                    memPreviousLevel = levelIndex;
                    if(!gameLoaded)
                    {
                        game.loadSavedSettingsData();
                        gameLoaded = true;
                    }
                }
                else if(levelRetState == LevelLoadState_e::FAIL)
                {
                    levelIndex = memPreviousLevel;
                }
            }
        }
        if(levelRetState != LevelLoadState_e::FAIL)
        {
            game.unsetFirstLaunch();
        }
        levelState = game.launchGame(levelIndex, levelState.m_levelState, levelRetState == LevelLoadState_e::FAIL);
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
        if(levelRetState != LevelLoadState_e::FAIL)
        {
            game.clearLevel(levelState);
        }
    }while(true);
    game.clearLevel(levelState);
    return 0;
}
