#pragma once

//////////////////////////////////////       INCLUDES & FORWARDS       //////////////////////////////////////////

#include "StaticObjects.h"
#include "Room.h"
#include "Screen.h"
#include <vector>

class Player;

////////////////////////////////////////        SpringCell         //////////////////////////////////////////
struct SpringCell
{
    Point pos;
    Direction projectionDirection;
    bool collapsed;  // Set to true when player steps on this cell
    bool startPoint;
    bool anchor;

    SpringCell() : pos(Point(-1, -1)), projectionDirection(Direction::STAY),
                   collapsed(false), startPoint(false), anchor(false) {}
};

//////////////////////////////////////////          Spring            //////////////////////////////////////////

// Spring object - to be reimplemented
class Spring : public StaticObject
{
private:
    std::vector<SpringCell> cells;
    Point* anchorPosition;
    SpringCell* startingCell;
    int compressionState;
    Direction compressionDir;
    bool compressed;

public:
    // Default constructor
    Spring() : StaticObject(), cells(),
               anchorPosition(nullptr), startingCell(nullptr),
               compressionState(0), compressionDir(Direction::STAY),
               compressed(false)
    {
        sprite = '#';
        type = ObjectType::SPRING;
    }

    // Constructor with position
    Spring(const Point& pos) : StaticObject(pos, '#', ObjectType::SPRING),
                                cells(),
                                anchorPosition(nullptr),
                                startingCell(nullptr),
                                compressionState(0),
                                compressionDir(Direction::STAY),
                                compressed(false) {}

    // Destructor
    ~Spring();

    // Initialize spring with cells, anchor, and projection direction
    void initialize(const std::vector<Point>& springCells,
                    const Point& anchor,
                    Direction projectionDir);

    // GameObject inherited interface - must be implemented
    GameObject* clone() const override;
    const char* getName() const override { return "Spring"; }
    bool isBlocking() const override { return false; }
    bool onExplosion() override { return true; }
    bool isCompressed() const { return compressed; }
    Point getAnchorPosition() const { return *anchorPosition; }

    // Debug getters
    int getCompressionState() const { return compressionState; }
    int getCellCount() const { return cells.size(); }
    Point getCellPosition(int index) const {
        if (index >= 0 && index < static_cast<int>(cells.size())) {
            return cells[index].pos;
        }
        return Point(-1, -1);
    }

    bool isCompressing(const Player& p, int checkX, int checkY) const ;

    void sortSpringCells();
    void compressCell();

    void launch(Player* p);

    void reset();
    
};
