#include <Game.hpp>
#include <MainEngine.hpp>

int main()
{
    Game game;
    game.initEngine();
    game.loadStandardData();
    bool gameLoaded = false;
    std::pair<bool, bool> pair;
    for(uint32_t i = 1; i < 3; ++i)
    {
        game.loadLevelData(i);
        if(!gameLoaded)
        {
            game.loadSavedSettingsData();
            gameLoaded = true;
        }
        pair = game.launchGame();
        //quit
        if(!pair.first)
        {
            break;
        }
        //game over
        if(pair.second)
        {
            --i;
        }
        game.clearLevel();
    }
    game.clearLevel();
    return 0;
}
