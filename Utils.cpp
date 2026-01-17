//////////////////////////////////////       INCLUDES & FORWARDS       //////////////////////////////////////////

#include "Utils.h"
#include "Renderer.h"
#include <ctime>
#include <iostream>

//////////////////////////////////////////         drawAt          //////////////////////////////////////////

void drawAt(int x, int y, char c)
{
    Renderer::printAt(x, y, c);
    Renderer::flush();
}

//////////////////////////////////////////         getFps          //////////////////////////////////////////

int getFps()
{
    static int lastTime = 0;
    static long loops = 0;
    static int lastFps = 0;

    int curr = std::time(nullptr);

    if (lastTime == 0) lastTime = curr;
    else if (lastTime == curr) loops++;
    else
    {
        lastFps = loops / (curr - lastTime);
        loops = 0;
        lastTime = curr;
    }

    return lastFps;
}

//////////////////////////////////////////         showFps          //////////////////////////////////////////

void showFps()
{
    const int x = 0, y = 0, width = 10;
    Renderer::gotoxy(x, y);

    for (int i = 0; i < width; i++) Renderer::print(' ');

    Renderer::gotoxy(x, y);
    Renderer::print(getFps());
    Renderer::flush();
}
