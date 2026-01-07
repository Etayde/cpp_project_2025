//////////////////////////////////////       INCLUDES & FORWARDS       //////////////////////////////////////////

#include "Console.h"
#include "Game.h"

#include <iostream>

int main()
{

    init_console();
    hideCursor();
    clrscr();

    Game game;
    game.run();

    showCursor();
    clrscr();
    cleanup_console();

    std::cout << "Thanks for playing!" << std::endl;

    return 0;
}
