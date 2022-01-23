#include <Game.hpp>
#include <MainEngine.hpp>

int main()
{
    Game game;
    game.initEngine();
    game.loadStandardData();
    bool gameLoaded = false;
    std::tuple<bool, bool, std::optional<uint32_t>> tuple;
    for(uint32_t i = 1; i < 3; ++i)
    {
        game.loadLevelData(i);
        if(!gameLoaded)
        {
            game.loadSavedSettingsData();
            gameLoaded = true;
        }
        tuple = game.launchGame(i);
        //quit
        if(!std::get<0>(tuple))
        {
            break;
        }
        //game over
        if(std::get<1>(tuple))
        {
            --i;
        }
        if(std::get<2>(tuple))
        {
            i = *std::get<2>(tuple) - 1;
        }
        game.clearLevel();
    }
    game.clearLevel();
    return 0;
}
