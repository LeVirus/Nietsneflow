#include <Game.hpp>
#include <MainEngine.hpp>

int main()
{
    Game game;
    game.initEngine();
    bool memGameOver;
    for(uint32_t i = 1; i < 3; ++i)
    {
        game.loadLevelData(i);
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
    return 0;
}
