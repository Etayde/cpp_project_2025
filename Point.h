#pragma once
#include "Console.h"
#include "Constants.h"

class Point {
public:
    int x;
    int y;
    int diff_x;
    int diff_y;
    char sprite;

    Point() : x(1), y(1), diff_x(0), diff_y(0), sprite(' ') {}

    Point(int x1, int y1) : x(x1), y(y1), diff_x(0), diff_y(0), sprite(' ') {}

    Point(int x1, int y1, int diffx, int diffy, char c)
        : x(x1), y(y1), diff_x(diffx), diff_y(diffy), sprite(c) {}

    void draw() const {
        gotoxy(x, y);
        std::cout << sprite;
        std::cout.flush();
    }

    void draw(char c) const {
        gotoxy(x, y);
        std::cout << c;
        std::cout.flush();
    }

    void move();
    void setDirection(Direction dir);

    int getX() const { return x; }
    int getY() const { return y; }
    char getSprite() const { return sprite; }
    void setSprite(char c) { sprite = c; }
};
