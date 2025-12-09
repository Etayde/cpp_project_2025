#include "Console.h"
#include "Game.h"
#include <iostream>

int main()
{
    // Initialize console for raw input
    init_console();
    hideCursor();
    clrscr();

    // Create and run the game
    Game game;
    game.run();

    // Cleanup
    showCursor();
    clrscr();
    cleanup_console();

    std::cout << "Thanks for playing!" << std::endl;

    return 0;
}
