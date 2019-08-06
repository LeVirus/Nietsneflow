#include <Game.hpp>
#include <MainEngine.hpp>

int main()
{
    Game game;
    game.loadLevelData(1);
    game.initEngine();
    game.launchGame();
    return 0;
}
