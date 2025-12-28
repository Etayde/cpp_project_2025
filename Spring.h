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
    bool compressed;
    bool startPoint;
    bool anchor;

    SpringCell() : pos(Point(-1, -1)), projectionDirection(Direction::STAY),
                   compressed(false), startPoint(false), anchor(false) {}
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
    Spring(const Point& pos) : StaticObject(pos, '#', ObjectType::SPRING) {}

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

    bool isCompressing(const Player& p) const ;

    void sortSpringCells();
    void compressCell();

    void launch(Player* p);

    void reset();
    
};
