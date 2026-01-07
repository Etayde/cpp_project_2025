
#pragma once

//////////////////////////////////////////       INCLUDES & FORWARDS       //////////////////////////////////////////

#include "Console.h"
#include "Constants.h"

//////////////////////////////////////////          Point             //////////////////////////////////////////

// 2D point
class Point
{
public:
    int x;
    int y;
    int diff_x;
    int diff_y;
    char sprite;

public:
    Point() : x(1), y(1), diff_x(0), diff_y(0), sprite(' ') {}
    Point(int x1, int y1) : x(x1), y(y1), diff_x(0), diff_y(0), sprite(' ') {}
    Point(int x1, int y1, int diffx, int diffy, char c)
        : x(x1), y(y1), diff_x(diffx), diff_y(diffy), sprite(c) {}
    Point(const Point &other)
        : x(other.x), y(other.y), diff_x(other.diff_x), diff_y(other.diff_y), sprite(other.sprite) {}

    void draw() const
    {
        gotoxy(x, y);
        std::cout << sprite << std::flush;
    }

    void draw(char c) const
    {
        gotoxy(x, y);
        std::cout << c << std::flush;
    }

    void move();

    void setDirection(Direction dir, int speed = 1);

    // Getters
    int getX() const { return x; }
    int getY() const { return y; }
    char getSprite() const { return sprite; }

    // Setters
    void setSprite(char c) { sprite = c; }
    void operator=(const Point &other)
    {
        x = other.x;
        y = other.y;
    }

    bool operator==(const Point &other) const { return x == other.x && y == other.y; }
    bool operator!=(const Point &other) const { return !(*this == other); }
};

// Hash function for Point to use in unordered_map - created with AI
namespace std
{
    template <>
    struct hash<Point>
    {
        size_t operator()(const Point &p) const
        {
            return hash<int>()(p.x) ^ (hash<int>()(p.y) << 1);
        }
    };
}
