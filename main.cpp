//////////////////////////////////////       INCLUDES & FORWARDS       //////////////////////////////////////////

#include "Console.h"
#include "Game.h"
#include "NormalGame.h"
#include "LoadedGame.h"

#include <iostream>
#include <string>

int main(int argc, char* argv[])
{
    // Parse command-line arguments
    bool loadMode = false;
    bool saveMode = false;
    bool silentMode = false;
    std::string filename;

    for (int i = 1; i < argc; i++) {
        std::string arg(argv[i]);

        if (arg == "-load" && i + 1 < argc) {
            loadMode = true;
            filename = argv[++i];
        }
        else if (arg == "-save" && i + 1 < argc) {
            saveMode = true;
            filename = argv[++i];
        }
        else if (arg == "-silent") {
            silentMode = true;
        }
    }

    init_console();
    hideCursor();
    clrscr();

    Game* game = nullptr;

    if (loadMode) {
        // LoadedGame respects silent flag
        game = new LoadedGame(filename, silentMode);
    }
    else {
        // NormalGame always visual (ignores silent flag)
        game = new NormalGame();
        if (saveMode) {
            static_cast<NormalGame*>(game)->enableRecording(filename);
        }
    }

    if (game) {
        game->run();
        delete game;
    }

    showCursor();
    clrscr();
    cleanup_console();

    std::cout << "Thanks for playing!" << std::endl;

    return 0;
}
