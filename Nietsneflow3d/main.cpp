#include <Game.hpp>
#include <MainEngine.hpp>

int main()
{
    Game game;
    game.initEngine();
    game.loadLevelData(1);
    game.launchGame();
    return 0;
}
