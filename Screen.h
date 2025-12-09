#pragma once

//////////////////////////////////////////       INCLUDES & FORWARDS       //////////////////////////////////////////

#include <iostream>
#include "Console.h"
#include "Constants.h"

class Point;

using std::cout, std::endl;

//////////////////////////////////////////          Screen            //////////////////////////////////////////

class Screen
{
private:
    const char *screen[MAX_Y];

public:
    // Construct from layout array
    Screen(const char *layout[MAX_Y])
    {
        for (int i = 0; i < MAX_Y; i++)
            screen[i] = layout[i];
    }

    // Default constructor - empty screen
    Screen()
    {
        for (int i = 0; i < MAX_Y; i++)
            screen[i] = nullptr;
    }

    // Get character at position (returns 'W' if out of bounds)
    char getCharAt(int x, int y) const
    {
        if (x >= 0 && x < MAX_X && y >= 0 && y < MAX_Y && screen[y] != nullptr)
        {
            return screen[y][x];
        }
        return 'W';
    }

    // Get character at Point position
    char getCharAt(const Point &p) const;

    // Check if position is a wall
    bool isWall(int x, int y) const { return getCharAt(x, y) == 'W'; }
    bool isWall(const Point &p) const;

    // Check if there's an object at position
    bool isObject(const Point &p) const;

    // Get object type at position
    ObjectType objectIs(const Point &p) const;

    // Draw entire screen
    void draw() const
    {
        clrscr();
        gotoxy(0, 0);

        for (int i = 0; i < MAX_Y - 1; ++i)
        {
            if (screen[i] != nullptr)
                cout << screen[i] << endl;
        }

        if (screen[MAX_Y - 1] != nullptr)
            cout << screen[MAX_Y - 1];
        cout.flush();
    }
};
