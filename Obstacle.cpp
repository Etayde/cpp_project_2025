#include "Obstacle.h"
#include "GameObject.h"
#include "PickableObject.h"
#include "Room.h"
#include "Player.h"


void Obstacle::initialize(const std::vector<ObstacleBlock*>& obstacleBlocks, 
                            std::unordered_map<Point, std::vector<Point>>& neighbors)
{
    blocks = obstacleBlocks;
    weight = static_cast<int>(blocks.size());
    for (size_t i = 0; i < blocks.size(); i++)
    {
        blocks[i]->setBlockIndex(static_cast<int>(i));
    }
    initEdges(neighbors);
}

bool Obstacle::move(Direction dir, Room* room, int force)
{
    // Validate force requirement
    if (force < weight)
        return false;  // Insufficient force

    // Calculate offset based on direction
    int dx = 0, dy = 0;
    switch (dir)
    {
    case Direction::UP:
        dy = -1;
        break;
    case Direction::DOWN:
        dy = 1;
        break;
    case Direction::LEFT:
        dx = -1;
        break;
    case Direction::RIGHT:
        dx = 1;
        break;
    default:
        break;
    }

    // Check collision: only edge blocks matter for obstacle boundary
    for (ObstacleBlock* block : edges[dir])
    {
        Point pos = block->getPosition();
        if (room->isBlocked(pos.x + dx, pos.y + dy))
        {
            return false; // Movement blocked
        }
    }

    // Move ALL blocks by the same offset
    for (ObstacleBlock* block : blocks)
    {
        Point pos = block->getPosition();
        block->setPosition(Point(pos.x + dx, pos.y + dy));
    }

    return true; // Movement successful
}


std::vector<Direction> ObstacleBlock::neighborsToEdgeDirections(std::unordered_map<Point, std::vector<Point>>& neighbors)
{
    int x = position.x;
    int y = position.y;

    std::vector<Direction> edgeDirections;
    std::vector<Point> neighborPositions = neighbors[Point(x, y)];

    if (neighborPositions.empty())
        return {};

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

    return edgeDirections;
}

void Obstacle::initEdges(std::unordered_map<Point, std::vector<Point>>& neighbors)
{
    for (auto& block : blocks)
    {
        if (neighbors.find(block->getPosition()) != neighbors.end())
        {
            std::vector<Direction> blockEdges = block->neighborsToEdgeDirections(neighbors);
            for (const Direction& dir : blockEdges)
            {
                edges[dir].push_back(block);
            }
        }
    }
}

void Obstacle::resetPushState()
{
    accumulatedForce = 0;
    pushDirection = Direction::STAY;
    pushers.clear();
}

bool Obstacle::tryPush(Direction dir, int force, Room* room, Player* pusher)
{
    // First push this frame - initialize direction
    if (pushers.empty())
    {
        pushDirection = dir;
        accumulatedForce = force;
        pushers.push_back(pusher);

        // Check if this force alone is enough
        if (accumulatedForce >= weight)
            return move(dir, room, accumulatedForce);

        return false;  // Not enough force yet
    }

    // Subsequent push - must be same direction
    if (dir == pushDirection)
    {
        accumulatedForce += force;
        pushers.push_back(pusher);

        // Check if combined force is now enough
        if (accumulatedForce >= weight)
            return move(dir, room, accumulatedForce);
    }

    return false;  // Either wrong direction or still not enough force
}