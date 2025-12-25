#pragma once

//////////////////////////////////////       INCLUDES & FORWARDS       //////////////////////////////////////////

#include "StaticObjects.h"
#include <vector>

class Player;
class Room;

//////////////////////////////////////////      SpringCell           //////////////////////////////////////////

// Individual cell in a spring
struct SpringCell
{
    Point position;
    bool visible;

    SpringCell(const Point& pos) : position(pos), visible(true) {}
};

//////////////////////////////////////////          Spring            //////////////////////////////////////////

// Multi-character spring that compresses and launches players
class Spring : public StaticObject
{
private:
    std::vector<SpringCell> cells;       // All spring cells
    Direction orientation;                // HORIZONTAL or VERTICAL
    Direction projectionDirection;        // Direction away from wall
    Point wallAnchor;                     // Wall attachment position
    Point freeEnd;                        // Free end position (opposite of wall anchor)
    int maxLength;                        // Total cells

    // Simple compression tracking (player IDs, not pointers!)
    int compressingPlayer1Id;             // 0 = not compressing
    int compressingPlayer2Id;             // 0 = not compressing
    int player1Compression;               // How many cells player 1 compressed
    int player2Compression;               // How many cells player 2 compressed

    // Launch state tracking (replaces Player data members!)
    int launchedPlayer1Id;                // 0 = not launched
    int launchedPlayer2Id;                // 0 = not launched
    int player1LaunchFrames;              // Frames remaining for player 1
    int player2LaunchFrames;              // Frames remaining for player 2
    int player1LaunchSpeed;               // Speed for player 1
    int player2LaunchSpeed;               // Speed for player 2

public:
    // Default constructor (for compatibility)
    Spring() : StaticObject(), orientation(Direction::HORIZONTAL),
               projectionDirection(Direction::RIGHT), wallAnchor(0, 0), freeEnd(0, 0),
               maxLength(1), compressingPlayer1Id(0), compressingPlayer2Id(0),
               player1Compression(0), player2Compression(0),
               launchedPlayer1Id(0), launchedPlayer2Id(0),
               player1LaunchFrames(0), player2LaunchFrames(0),
               player1LaunchSpeed(0), player2LaunchSpeed(0)
    {
        sprite = '#';
        type = ObjectType::SPRING;
        cells.push_back(SpringCell(Point(0, 0)));
    }

    // Multi-character spring constructor
    Spring(const std::vector<Point>& positions, Direction orient,
           Direction projDir, const Point& anchor);

    // GameObject interface
    GameObject* clone() const override;
    const char* getName() const override { return "Spring"; }
    bool isBlocking() const override { return false; }
    bool onExplosion() override { return true; }
    void draw() const override;
    void update(Player* player1, Player* player2);  // Called each game cycle

    // Position queries
    bool occupiesPosition(int x, int y) const;
    bool isAtFreeEnd(int x, int y) const;  // Check if position is at free end

    // Getters
    int getMaxLength() const { return maxLength; }
    Direction getProjectionDirection() const { return projectionDirection; }
    Point getFreeEnd() const { return freeEnd; }

    // Compression management (called by Player)
    bool isPlayerCompressing(int playerId) const;
    int getPlayerCompression(int playerId) const;
    int getTotalCompression() const { return player1Compression + player2Compression; }
    bool isPlayerBeingLaunched(int playerId) const;

    void addCompression(int playerId);        // Increment compression for player
    void launchPlayer(int playerId);          // Start launch for player
    void reset();                              // Full reset (room transitions)

private:
    void updateVisual();                      // Update cell visibility based on total compression
    void updateLaunch(Player* player);        // Update player velocity during launch
};
