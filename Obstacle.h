#pragma once

////////////////////////////////////////      INCLUDES & FORWARDS       //////////////////////////////////////////

#include "StaticObjects.h"
#include <vector>
#include <unordered_map>

class Room;
class Player;
class Obstacle;

///////////////////////////////////////////      ObstacleBlock        //////////////////////////////////////////

class ObstacleBlock : public StaticObject
{
private:
    Obstacle* parentObstacle;
    int blockIndex;
    bool is_edge;
public:
    ObstacleBlock(const Point &pos, Obstacle* parent)
        : StaticObject(pos, '*', ObjectType::OBSTACLE_BLOCK),
         parentObstacle(parent), blockIndex(-1), is_edge(false) {}

    GameObject* clone() const override { return new ObstacleBlock(*this); }
    const char* getName() const override { return "ObstacleBloack"; }
    bool isBlocking() const override { return true; }
    Obstacle* getParent() const { return parentObstacle; }
    int getBlockIndex() const { return blockIndex; }

    void setBlockIndex(int index) { blockIndex = index; }
    bool isEdge() const { return is_edge; }
    std::vector<Direction> neighborsToEdgeDirections(std::unordered_map<Point, std::vector<Point>>& neighbors);
};

//////////////////////////////////////////        Obstacle         //////////////////////////////////////////

// An movable object that blocks movement
class Obstacle
{
private:
    std::vector<ObstacleBlock *> blocks;
    std::unordered_map<Direction, std::vector<ObstacleBlock*>> edges;
    int weight = blocks.size(); // Weight based on number of blocks

    // Push state tracking (per frame)
    int accumulatedForce;           // Total force applied this frame
    Direction pushDirection;        // Direction being pushed this frame
    std::vector<Player*> pushers;   // Players who contributed to the push

public:
    Obstacle(): blocks(), accumulatedForce(0), pushDirection(Direction::STAY)
    {
        edges = {
            {Direction::UP, {}},
            {Direction::DOWN, {}},
            {Direction::LEFT, {}},
            {Direction::RIGHT, {}}
        };
        pushers.reserve(2);  // Max 2 players
    };

    void initialize(const std::vector<ObstacleBlock *>& obstacleBlocks,
                            std::unordered_map<Point, std::vector<Point>>& neighbors);

    int getWeight() const { return weight; }
    const std::vector<ObstacleBlock *>& getBlocks() const { return blocks; }
    bool canBeMoved(int force) const { return force >= weight; }
    bool move(Direction dir, Room* room, int force);
    void initEdges(std::unordered_map<Point, std::vector<Point>>& neighbors);

    // Push state management
    void resetPushState();
    bool tryPush(Direction dir, int force, Room* room, Player* pusher);
};
