#include "Spring.h"
#include "Player.h"

//////////////////////////////////////////      Destructor          //////////////////////////////////////////

Spring::~Spring()
{
    if (anchorPosition != nullptr) {
        delete anchorPosition;
        anchorPosition = nullptr;
    }
}

//////////////////////////////////////////      Initialize          //////////////////////////////////////////

void Spring::initialize(const std::vector<Point>& springCells,
                       const Point& anchor,
                       Direction projectionDir)
{
    // Set anchor position
    if (anchorPosition == nullptr) {
        anchorPosition = new Point(anchor);
    } else {
        *anchorPosition = anchor;
    }

    // Set compression direction
    compressionDir = projectionDir;

    // Populate cells vector
    cells.clear();
    for (const Point& cellPos : springCells) {
        SpringCell cell;
        cell.pos = cellPos;
        cell.compressed = false;
        cells.push_back(cell);
    }

    // Set starting cell (first cell in the spring)
    if (!cells.empty()) {
        startingCell = &cells[0];
    }

    // Reset compression state
    compressionState = 0;
    compressed = false;
}

//////////////////////////////////////////        clone              //////////////////////////////////////////

GameObject* Spring::clone() const
{
    return new Spring(*this);
}

bool Spring::isCompressing(const Player& p) const{
    if (compressed) {
        return true;
    }
    if (!compressed && p.getCurrentDirection() == compressionDir) {
        if (p.getPosition() == startingCell->pos)
        return true;
    }
    return false;
}

void Spring::compressCell() {
    this->cells[compressionState].compressed = true;
    compressionState++;
}

void Spring::reset() {
    for (auto& cell : cells) {
        cell.compressed = false;
    }
    compressionState = 0;
    compressed = false;
}

void Spring::launch(Player* p){
    
    Direction launchDir;
    switch (this->compressionDir) {
        case Direction::UP:
            launchDir = Direction::DOWN;
            break;
        case Direction::DOWN:
            launchDir = Direction::UP;
            break;
        case Direction::LEFT:
            launchDir = Direction::RIGHT;
            break;
        case Direction::RIGHT:
            launchDir = Direction::LEFT;
            break;
        default:
            break;
    }
    p->setDirection(launchDir, compressionState);
    p->launchFramesRemaining = compressionState;

    this->reset();

}

