
#pragma once

//////////////////////////////////////////       INCLUDES & FORWARDS       //////////////////////////////////////////

#include "Console.h"
#include "Constants.h"

//////////////////////////////////////////          Point             //////////////////////////////////////////

// 2D point
class Point
{
public:
    int x;      // X coordinate
    int y;      // Y coordinate
    int diff_x; // Horizontal movement per tick
    int diff_y; // Vertical movement per tick
    char sprite;

public:
    Point() : x(1), y(1), diff_x(0), diff_y(0), sprite(' ') {}
    Point(int x1, int y1) : x(x1), y(y1), diff_x(0), diff_y(0), sprite(' ') {}
    Point(int x1, int y1, int diffx, int diffy, char c)
        : x(x1), y(y1), diff_x(diffx), diff_y(diffy), sprite(c) {}

    // Draw sprite at current position
    void draw() const
    {
        gotoxy(x, y);
        std::cout << sprite << std::flush;
    }

    // Draw custom char at current position
    void draw(char c) const
    {
        gotoxy(x, y);
        std::cout << c << std::flush;
    }

    // Apply movement vector
    void move();

    // Set movement direction
    void setDirection(Direction dir);

    // Getters
    int getX() const { return x; }
    int getY() const { return y; }
    char getSprite() const { return sprite; }

    // Setters
    void setSprite(char c) { sprite = c; }

    // Comparison operators
    bool operator==(const Point &other) const { return x == other.x && y == other.y; }
    bool operator!=(const Point &other) const { return !(*this == other); }
};
