#include <Game.hpp>
#include <MainEngine.hpp>

int main()
{
    Game game;
    game.loadLevelData();
    game.initEngine();
    game.launchGame();
    return 0;
}
