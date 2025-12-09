
#include "Point.h"

//////////////////////////////////////////           move             //////////////////////////////////////////

// Apply movement vector to position (clamped to game area)
void Point::move()
{
    int newX = x + diff_x;
    int newY = y + diff_y;

    if (newX >= 1 && newX < MAX_X - 1)
        x = newX;
    if (newY >= 1 && newY < MAX_Y_INGAME - 1)
        y = newY;
}

//////////////////////////////////////////       setDirection         //////////////////////////////////////////

// Set movement direction from Direction enum
void Point::setDirection(Direction dir)
{
    switch (dir)
    {
    case Direction::UP:
        diff_x = 0;
        diff_y = -1;
        break;
    case Direction::DOWN:
        diff_x = 0;
        diff_y = 1;
        break;
    case Direction::RIGHT:
        diff_x = 1;
        diff_y = 0;
        break;
    case Direction::LEFT:
        diff_x = -1;
        diff_y = 0;
        break;
    case Direction::STAY:
        diff_x = 0;
        diff_y = 0;
        break;
    }
}
