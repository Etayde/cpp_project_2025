//////////////////////////////////////       INCLUDES & FORWARDS       /////////////////////////////////////////////

#include "Obstacle.h"
#include "GameObject.h"
#include "Room.h"
#include "Player.h"
#include "Renderer.h"

//////////////////////////////////////////        initialize       /////////////////////////////////////////////

void Obstacle::initialize(const std::vector<ObstacleBlock *> &obstacleBlocks,
                          std::unordered_map<Point, std::vector<Point>> &neighbors)
{
    blocks = obstacleBlocks;
    weight = static_cast<int>(blocks.size());
    for (size_t i = 0; i < blocks.size(); i++)
    {
        blocks[i]->setBlockIndex(static_cast<int>(i));
    }
    initEdges(neighbors);
}

//////////////////////////////////////////           move           /////////////////////////////////////////////

bool Obstacle::move(Direction dir, Room *room, int force)
{
    if (movedThisFrame)
        return false;

    if (force < weight)
        return false;

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

    for (ObstacleBlock *block : edges[dir])
    {
        Point pos = block->getPosition();
        if (room->isBlocked(pos.getX() + dx, pos.getY() + dy))
        {
            return false;
        }
    }

    for (ObstacleBlock *block : blocks)
    {
        Point oldPos = block->getPosition();
        Point newPos(oldPos.getX() + dx, oldPos.getY() + dy);

        room->setCharAt(oldPos.getX(), oldPos.getY(), ' ');
        Renderer::printAt(oldPos.getX(), oldPos.getY(), ' ');

        block->setPosition(newPos);

        room->setCharAt(newPos.getX(), newPos.getY(), block->getSprite());
    }

    movedThisFrame = true;
    return true;
}

//////////////////////////////////////////  neighborsToEdgeDirections       /////////////////////////////////////////////

std::vector<Direction> ObstacleBlock::neighborsToEdgeDirections(std::unordered_map<Point, std::vector<Point>> &neighbors)
{
    int x = getX();
    int y = getY();

    std::vector<Direction> edgeDirections;
    std::vector<Point> neighborPositions = neighbors[Point(x, y)];

    if (neighborPositions.empty())
        return {};

    for (const Point &np : neighborPositions)
    {
        if (np.getX() == x && np.getY() == y - 1)
            edgeDirections.push_back(Direction::UP);
        else if (np.getX() == x && np.getY() == y + 1)
            edgeDirections.push_back(Direction::DOWN);
        else if (np.getX() == x - 1 && np.getY() == y)
            edgeDirections.push_back(Direction::LEFT);
        else if (np.getX() == x + 1 && np.getY() == y)
            edgeDirections.push_back(Direction::RIGHT);
    }

    return edgeDirections;
}

//////////////////////////////////////////        initEdges         /////////////////////////////////////////////

void Obstacle::initEdges(std::unordered_map<Point, std::vector<Point>> &neighbors)
{
    for (auto &block : blocks)
    {
        if (neighbors.find(block->getPosition()) != neighbors.end())
        {
            std::vector<Direction> blockEdges = block->neighborsToEdgeDirections(neighbors);
            for (const Direction &dir : blockEdges)
            {
                edges[dir].push_back(block);
            }
        }
    }
}

//////////////////////////////////////////      resetPushState       /////////////////////////////////////////////

void Obstacle::resetPushState()
{
    accumulatedForce = 0;
    pushDirection = Direction::STAY;
    pushers.clear();
    movedThisFrame = false;
}

//////////////////////////////////////////         tryPush          /////////////////////////////////////////////

bool Obstacle::tryPush(Direction dir, int force, Room *room, Player *pusher)
{
    if (pushers.empty())
    {
        pushDirection = dir;
        accumulatedForce = force;
        pushers.push_back(pusher);

        if (accumulatedForce >= weight)
            return move(dir, room, accumulatedForce);

        return false;
    }

    if (dir == pushDirection)
    {
        accumulatedForce += force;
        pushers.push_back(pusher);

        if (accumulatedForce >= weight)
            return move(dir, room, accumulatedForce);
    }

    return false;
}

//////////////////////////////////////////  ObstacleBlock::onExplosion  //////////////////////////////////////////

bool ObstacleBlock::onExplosion()
{
    if (parentObstacle)
    {
        parentObstacle->markForReconstruction();
    }
    return true;
}

//////////////////////////////////////////  Obstacle::reconstruct  //////////////////////////////////////////

void Obstacle::reconstruct(Room *room)
{
    std::vector<ObstacleBlock *> remaining;
    for (ObstacleBlock *block : blocks)
    {
        if (block && block->isActive())
            remaining.push_back(block);
    }

    if (remaining.empty())
    {
        return;
    }

    std::vector<std::vector<ObstacleBlock *>> components = findConnectedComponents(remaining);

    if (components.size() == 1)
    {
        blocks = remaining;
        weight = static_cast<int>(blocks.size());
        for (size_t i = 0; i < blocks.size(); i++)
        {
            blocks[i]->setBlockIndex(static_cast<int>(i));
        }
        std::unordered_map<Point, std::vector<Point>> neighbors = buildNeighborsMap(remaining);
        initEdges(neighbors);
        resetPushState();
    }
    else
    {
        blocks = components[0];
        weight = static_cast<int>(blocks.size());
        for (size_t i = 0; i < blocks.size(); i++)
        {
            blocks[i]->setBlockIndex(static_cast<int>(i));
        }
        std::unordered_map<Point, std::vector<Point>> neighbors = buildNeighborsMap(components[0]);
        initEdges(neighbors);
        resetPushState();

        for (size_t j = 1; j < components.size(); j++)
        {
            Obstacle *newObs = new Obstacle();
            newObs->blocks = components[j];
            newObs->weight = static_cast<int>(components[j].size());

            for (size_t k = 0; k < components[j].size(); k++)
            {
                components[j][k]->setBlockIndex(static_cast<int>(k));
                components[j][k]->setParent(newObs);
            }

            std::unordered_map<Point, std::vector<Point>> newNeighbors = buildNeighborsMap(components[j]);
            newObs->initEdges(newNeighbors);
            newObs->resetPushState();

            room->addObstacle(newObs);
        }
    }

    needsReconstructionFlag = false;
}

//////////////////////////////////////////  findConnectedComponents  //////////////////////////////////////////

// Made with AI - I strated with a simpler obstacle reconstruction
// method which had bugs and multiple problems with it's logic.
// I then asked an AI to review my current algorithm and suggest a better solution.
// The AI then provided this BFS-based connected components approach,
// which worked great as far as I've tested, and therefore decided to keep altough
// we didn't properly get to study and fully understand the BFS algorithm yet.

std::vector<std::vector<ObstacleBlock *>> Obstacle::findConnectedComponents(
    const std::vector<ObstacleBlock *> &blocks)
{
    std::vector<std::vector<ObstacleBlock *>> components;
    std::unordered_set<ObstacleBlock *> visited;

    for (ObstacleBlock *start : blocks)
    {
        if (!start || visited.count(start))
            continue;

        // BFS
        std::vector<ObstacleBlock *> component;
        std::queue<ObstacleBlock *> queue;
        queue.push(start);
        visited.insert(start);

        while (!queue.empty())
        {
            ObstacleBlock *curr = queue.front();
            queue.pop();
            component.push_back(curr);

            Point pos = curr->getPosition();
            Point neighbors[4] = {
                Point(pos.getX() + 1, pos.getY()), Point(pos.getX() - 1, pos.getY()),
                Point(pos.getX(), pos.getY() + 1), Point(pos.getX(), pos.getY() - 1)};

            for (const Point &np : neighbors)
            {
                for (ObstacleBlock *other : blocks)
                {
                    if (other && other->isActive() &&
                        other->getPosition() == np &&
                        !visited.count(other))
                    {
                        visited.insert(other);
                        queue.push(other);
                    }
                }
            }
        }

        components.push_back(component);
    }

    return components;
}

//////////////////////////////////////////  buildNeighborsMap  //////////////////////////////////////////

std::unordered_map<Point, std::vector<Point>> Obstacle::buildNeighborsMap(
    const std::vector<ObstacleBlock *> &blocks)
{
    std::unordered_map<Point, std::vector<Point>> neighbors;

    for (ObstacleBlock *block : blocks)
    {
        if (!block)
            continue;

        Point pos = block->getPosition();
        Point adjacent[4] = {
            Point(pos.getX() + 1, pos.getY()), Point(pos.getX() - 1, pos.getY()),
            Point(pos.getX(), pos.getY() + 1), Point(pos.getX(), pos.getY() - 1)};

        for (const Point &adj : adjacent)
        {
            bool isObstacle = false;
            for (ObstacleBlock *other : blocks)
            {
                if (other && other->getPosition() == adj)
                {
                    isObstacle = true;
                    break;
                }
            }

            if (!isObstacle)
            {
                neighbors[pos].push_back(adj);
            }
        }
    }

    return neighbors;
}