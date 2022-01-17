#include <Game.hpp>
#include <MainEngine.hpp>

int main()
{
    Game game;
    game.initEngine();
    game.loadStandardData();
    bool memGameOver, gameLoaded = false;
    for(uint32_t i = 1; i < 3; ++i)
    {
        game.loadLevelData(i);
        if(!gameLoaded)
        {
            game.loadSavedSettingsData();
            gameLoaded = true;
        }
        if(!game.launchGame(memGameOver))
        {
            break;
        }
        if(memGameOver)
        {
            --i;
        }
        game.clearLevel();
    }
    game.clearLevel();
    return 0;
}
