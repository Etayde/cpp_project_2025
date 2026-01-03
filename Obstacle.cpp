#include "Obstacle.h"
#include "Room.h"


void Obstacle::initialize(const std::vector<ObstacleBlock*>& obstacleBlocks)
{
    blocks = obstacleBlocks;
    weight = static_cast<int>(blocks.size());
    for (size_t i = 0; i < blocks.size(); i++)
    {
        blocks[i]->setBlockIndex(static_cast<int>(i));
    }
}

bool Obstacle::move(Direction dir, Room* room)
{
    // Calculate new positions for all blocks
    std::vector<Point> newPositions;
    for (ObstacleBlock* block : blocks)
    {
        Point pos = block->getPosition();
        switch (dir)
        {
        case Direction::UP:
            if (room)
            pos.y -= 1;
            break;
        case Direction::DOWN:
            pos.y += 1;
            break;
        case Direction::LEFT:
            pos.x -= 1;
            break;
        case Direction::RIGHT:
            pos.x += 1;
            break;
        default:
            break;
        }
        newPositions.push_back(pos);
    }

    // Check if all new positions are free
    for (const Point& pos : newPositions)
    {
        if (room->isBlocked(pos.x, pos.y))
        {
            return false; // Movement blocked
        }
    }

    // Move all blocks to new positions
    for (size_t i = 0; i < blocks.size(); i++)
    {
        blocks[i]->setPosition(newPositions[i]);
    }

    return true; // Movement successful
}

void ObstacleBlock::neighborsToEdgeDirections(std::unordered_map<Point, std::vector<Point>>& neighbors)
{
    int x = position.x;
    int y = position.y;

    std::vector<Direction> edgeDirections;
    std::vector<Point> neighborPositions = neighbors[Point(x, y)];

    if (neighborPositions.empty())
        return;

    for (const Point& np : neighborPositions)
    {
        if (np.x == x && np.y == y - 1)
            edgeDirections.push_back(Direction::UP);
        else if (np.x == x && np.y == y + 1)
            edgeDirections.push_back(Direction::DOWN);
        else if (np.x == x - 1 && np.y == y)
            edgeDirections.push_back(Direction::LEFT);
        else if (np.x == x + 1 && np.y == y)
            edgeDirections.push_back(Direction::RIGHT);
    }

    setEdgeDirections(edgeDirections);

    return;
}