#include <Game.hpp>
#include <MainEngine.hpp>

int main()
{
    Game game;
    game.loadLevelData();
    game.initEngine();
    return 0;
}
