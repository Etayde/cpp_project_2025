//////////////////////////////////////       INCLUDES & FORWARDS       //////////////////////////////////////////

#include "Point.h"

//////////////////////////////////////////           move             //////////////////////////////////////////

void Point::move()
{
    int newX = x + diff_x;
    int newY = y + diff_y;

    if (newX >= 1 && newX < MAX_X - 1)
        x = newX;
    if (newY >= 1 && newY < MAX_Y - 1)
        y = newY;
}

//////////////////////////////////////////       setDirection         //////////////////////////////////////////

void Point::setDirection(Direction dir, int speed)
{
    switch (dir)
    {
    case Direction::UP:
        diff_x = 0;
        diff_y = -speed;
        break;
    case Direction::DOWN:
        diff_x = 0;
        diff_y = speed;
        break;
    case Direction::RIGHT:
        diff_x = speed;
        diff_y = 0;
        break;
    case Direction::LEFT:
        diff_x = -speed;
        diff_y = 0;
        break;
    case Direction::STAY:
    case Direction::HORIZONTAL:
    case Direction::VERTICAL:
        diff_x = 0;
        diff_y = 0;
        break;
    }
}
