#pragma once

//////////////////////////////////////       INCLUDES & FORWARDS       //////////////////////////////////////////

#include "StaticObjects.h"
#include <vector>

//////////////////////////////////////////          Spring            //////////////////////////////////////////

// Multi-cell spring that launches players when compressed
class Spring : public StaticObject
{
private:
    Direction launchDirection; // Direction away from wall
    int length;                // Number of cells
    std::vector<Point> cells;  // All cell positions in the spring

public:
    Spring() : StaticObject(), launchDirection(Direction::STAY), length(1)
    {
        sprite = '#';
        type = ObjectType::SPRING;
    }

    Spring(Direction dir, int len, const std::vector<Point> &cellPositions)
        : StaticObject(), launchDirection(dir), length(len), cells(cellPositions)
    {
        sprite = '#';
        type = ObjectType::SPRING;
        // Set position to first cell
        if (!cells.empty())
        {
            position = cells[0];
        }
    }

    GameObject *clone() const override { return new Spring(*this); }
    const char *getName() const override { return "Spring"; }

    bool isBlocking() const override { return false; }
    bool onExplosion() override { return true; }

    Direction getLaunchDirection() const { return launchDirection; }
    int getLength() const { return length; }

    // Check if this spring contains the given cell
    bool containsCell(int x, int y) const
    {
        for (const Point &cell : cells)
        {
            if (cell.x == x && cell.y == y)
                return true;
        }
        return false;
    }

    // Get 0-based index of cell from wall (first cell = 0)
    int getCellIndex(int x, int y) const
    {
        for (size_t i = 0; i < cells.size(); i++)
        {
            if (cells[i].x == x && cells[i].y == y)
                return static_cast<int>(i);
        }
        return -1;
    }
};