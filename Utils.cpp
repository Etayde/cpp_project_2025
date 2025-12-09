
//////////////////////////////////////       INCLUDES & FORWARDS       //////////////////////////////////////////
#include "Utils.h"
#include <ctime>
#include <iostream>

//////////////////////////////////////////         drawAt          //////////////////////////////////////////

void drawAt(int x, int y, char c)
{
    gotoxy(x, y);
    std::cout << c << std::flush;
}

//////////////////////////////////////////         getFps          //////////////////////////////////////////

// Calculate frames per second - For Debugging Only
int getFps()
{
    static int lastTime = 0;
    static long loops = 0;
    static int lastFps = 0;

    int curr = std::time(nullptr);

    if (lastTime == 0)
    {
        lastTime = curr;
    }
    else if (lastTime == curr)
    {
        loops++;
    }
    else
    {
        lastFps = loops / (curr - lastTime);
        loops = 0;
        lastTime = curr;
    }

    return lastFps;
}

//////////////////////////////////////////         showFps          //////////////////////////////////////////

// Show frames per second - For Debugging Only
void showFps()
{
    const int x = 0, y = 0, width = 10;
    gotoxy(x, y);

    for (int i = 0; i < width; i++)
    {
        std::cout << ' ';
    }

    gotoxy(x, y);
    std::cout << getFps() << std::flush;
}
