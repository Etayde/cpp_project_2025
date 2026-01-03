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
    bool isEdge;
    std::vector<Direction> edges;
public:
    ObstacleBlock(const Point &pos, Obstacle* parent, std::vector<Direction> d = {})
        : StaticObject(pos, 'O', ObjectType::OBSTACLE_BLOCK),
         parentObstacle(parent), blockIndex(-1), isEdge(false), edges(d) {}

    GameObject* clone() const override { return new ObstacleBlock(*this); }
    const char* getName() const override { return "ObstacleBloack"; }
    bool isBlocking() const override { return true; }
    Obstacle* getParent() const { return parentObstacle; }
    int getBlockIndex() const { return blockIndex; }
    std::vector<Direction> getEdges() const { return edges; };

    void setBlockIndex(int index) { blockIndex = index; }
    bool getIsEdge() const { return isEdge; }
    void setEdgeDirections(const std::vector<Direction>& d) { edges = d; isEdge = !d.empty(); }
    void neighborsToEdgeDirections(std::unordered_map<Point, std::vector<Point>>& neighbors);
};

//////////////////////////////////////////        Obstacle         //////////////////////////////////////////

// An movable object that blocks movement
class Obstacle
{
private:
    std::vector<ObstacleBlock *> blocks;
    int weight = blocks.size(); // Weight based on number of blocks
public:
    Obstacle(): blocks() {};

    void initialize(const std::vector<ObstacleBlock *>& obstacleBlocks);

    int getWeight() const { return weight; }
    const std::vector<ObstacleBlock *>& getBlocks() const { return blocks; }
    bool canBeMoved(int force) const { return force >= weight; }
    bool move(Direction dir, Room* room);
    void initEdges(std::unordered_map<Point, std::vector<Point>>& neighbors)
    {
        for (auto& block : blocks) { block->neighborsToEdgeDirections(neighbors); }
    }
};
