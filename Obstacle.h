#pragma once

////////////////////////////////////////      INCLUDES & FORWARDS       //////////////////////////////////////////

#include "StaticObjects.h"
#include <vector>
#include <unordered_map>
#include <queue>
#include <unordered_set>

class Room;
class Player;
class Obstacle;

///////////////////////////////////////////      ObstacleBlock        //////////////////////////////////////////

class ObstacleBlock : public StaticObject
{
private:
    Obstacle *parentObstacle;
    int blockIndex;
    bool is_edge;

public:
    ObstacleBlock(const Point &pos, Obstacle *parent)
        : StaticObject(pos, '*', ObjectType::OBSTACLE_BLOCK),
          parentObstacle(parent), blockIndex(-1), is_edge(false) {}

    GameObject *clone() const override { return new ObstacleBlock(*this); }
    const char *getName() const override { return "ObstacleBloack"; }
    bool isBlocking() const override { return true; }
    bool onExplosion() override;
    Obstacle *getParent() const { return parentObstacle; }
    int getBlockIndex() const { return blockIndex; }

    void setBlockIndex(int index) { blockIndex = index; }
    void setParent(Obstacle *parent) { parentObstacle = parent; }
    bool isEdge() const { return is_edge; }
    std::vector<Direction> neighborsToEdgeDirections(std::unordered_map<Point, std::vector<Point>> &neighbors);
};

//////////////////////////////////////////        Obstacle         //////////////////////////////////////////

// A movable object that blocks movement
class Obstacle
{
private:
    std::vector<ObstacleBlock *> blocks;
    std::unordered_map<Direction, std::vector<ObstacleBlock *>> edges;
    int weight = blocks.size();

    int accumulatedForce;
    Direction pushDirection;
    std::vector<Player *> pushers;
    bool movedThisFrame;

    bool needsReconstructionFlag = false;

    static std::vector<std::vector<ObstacleBlock *>> findConnectedComponents(
        const std::vector<ObstacleBlock *> &blocks);
    static std::unordered_map<Point, std::vector<Point>> buildNeighborsMap(
        const std::vector<ObstacleBlock *> &blocks);

public:
    Obstacle() : blocks(), accumulatedForce(0), pushDirection(Direction::STAY), movedThisFrame(false)
    {
        edges = {
            {Direction::UP, {}},
            {Direction::DOWN, {}},
            {Direction::LEFT, {}},
            {Direction::RIGHT, {}}};
        pushers.reserve(2);
    };

    void initialize(const std::vector<ObstacleBlock *> &obstacleBlocks,
                    std::unordered_map<Point, std::vector<Point>> &neighbors);

    int getWeight() const { return weight; }
    const std::vector<ObstacleBlock *> &getBlocks() const { return blocks; }
    bool canBeMoved(int force) const { return force >= weight; }
    bool move(Direction dir, Room *room, int force);
    void initEdges(std::unordered_map<Point, std::vector<Point>> &neighbors);

    void resetPushState();
    bool tryPush(Direction dir, int force, Room *room, Player *pusher);

    void markForReconstruction() { needsReconstructionFlag = true; }
    bool needsReconstruction() const { return needsReconstructionFlag; }
    void reconstruct(Room *room);
};
