#include "Utils.h"
#include <ctime>
#include <iostream>

void drawAt(int x, int y, char c) {
    gotoxy(x, y);
    std::cout << c << std::flush;
}

int getFps() {
    static int lastTime = 0;
    static long loops = 0;
    static int lastFps = 0;
    
    int curr = std::time(nullptr);
    
    if (lastTime == 0) {
        lastTime = curr;
    } else if (lastTime == curr) {
        loops++;
    } else {
        lastFps = loops / (curr - lastTime);
        loops = 0;
        lastTime = curr;
    }
    
    return lastFps;
}

void showFps() {
    const int x = 0, y = 0, width = 10;
    gotoxy(x, y);
    
    for (int i = 0; i < width; i++) {
        std::cout << ' ';
    }
    
    gotoxy(x, y);
    std::cout << getFps() << std::flush;
}
