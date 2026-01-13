//////////////////////////////////////       INCLUDES & FORWARDS       //////////////////////////////////////////

#include "Game.h"

int main(int argc, char* argv[])
{
    Game* game = Game::createFromArgs(argc, argv);

    if (game) {
        game->run();
        delete game;
    }

    return 0;
}
