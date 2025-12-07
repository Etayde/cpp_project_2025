#include "Point.h"

void Point::move() {
    int newX = x + diff_x;
    int newY = y + diff_y;
    
    // Clamp to screen bounds (game area only, 1 to MAX_X-2 and 1 to MAX_Y_INGAME-1)
    if (newX >= 1 && newX < MAX_X - 1) {
        x = newX;
    }
    if (newY >= 1 && newY < MAX_Y_INGAME - 1) {
        y = newY;
    }
}

void Point::setDirection(Direction dir) {
    switch (dir) {
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
