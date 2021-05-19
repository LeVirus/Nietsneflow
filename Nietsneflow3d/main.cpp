#include <Game.hpp>
#include <MainEngine.hpp>

int main()
{
    Game game;
    game.initEngine();
    for(uint32_t i = 1; i < 3; ++i)
    {
        game.loadLevelData(i);
        if(!game.launchGame())
        {
            break;
        }
        game.clearLevel();
    }
    return 0;
}
