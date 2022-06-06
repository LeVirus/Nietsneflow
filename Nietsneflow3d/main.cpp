#include <Game.hpp>
#include <MainEngine.hpp>

int main()
{
    uint32_t levelIndex = 1, memPreviousLevel;
    Game game;
    game.initEngine();
    game.loadStandardData();
    bool gameLoaded = false, firstLaunch = true, memCustomLevelMode = false;
    LevelState levelState = {LevelState_e::NEW_GAME, {}, false};
    LevelLoadState_e levelRetState = LevelLoadState_e::OK;
    game.loadStandardEntities();
    //PLAYER DEPARTURE NOT SET
    do
    {
        if(firstLaunch)
        {
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
            levelRetState = game.loadLevelData(levelIndex, levelState.m_customLevel, levelState.m_levelState);
            //if no more level
            if(levelRetState == LevelLoadState_e::END)
            {
                break;
            }
            else if(levelRetState == LevelLoadState_e::OK)
            {
                memPreviousLevel = levelIndex;
                memCustomLevelMode = levelState.m_customLevel;
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
            game.loadPlayerEntity();
            levelRetState = game.loadLevelData(levelIndex, levelState.m_customLevel, levelState.m_levelState);
            //if no more level
            if(levelRetState == LevelLoadState_e::END)
            {
                break;
            }
            else if(levelRetState == LevelLoadState_e::OK)
            {
                memPreviousLevel = levelIndex;
                memCustomLevelMode = levelState.m_customLevel;
                if(!gameLoaded)
                {
                    game.loadSavedSettingsData();
                    gameLoaded = true;
                }
            }
            else if(levelRetState == LevelLoadState_e::FAIL)
            {
                levelIndex = memPreviousLevel;
                levelState.m_customLevel = memCustomLevelMode;
            }
        }
        if(levelRetState != LevelLoadState_e::FAIL)
        {
            game.unsetFirstLaunch();
        }
        levelState = game.launchGame(levelIndex, levelState.m_levelState, levelRetState == LevelLoadState_e::FAIL, levelState.m_customLevel);
        switch(levelState.m_levelState)
        {
        case LevelState_e::EXIT:
            break;
        case LevelState_e::NEW_GAME:
            levelIndex = 1;
            continue;
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
            continue;
        }
        }
        //quit
        if(levelState.m_levelState == LevelState_e::EXIT)
        {
            break;
        }
        game.clearLevel(levelState);
    }while(true);
    game.clearLevel(levelState);
    return 0;
}
