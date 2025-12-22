#pragma once

//////////////////////////////////////       INCLUDES & FORWARDS       //////////////////////////////////////////

#include "StaticObjects.h"
#include <vector>

class Player;
class Room;

//////////////////////////////////////////          Spring            //////////////////////////////////////////

// Multi-character spring that compresses and launches players
class Spring : public StaticObject
{
private:
    int length;                           // Number of '#' characters in spring
    Direction orientation;                // HORIZONTAL or VERTICAL
    Direction projectionDirection;        // Direction spring pushes (away from wall)
    Point wallAnchor;                     // Position where spring attaches to wall
    int compressionState;                 // How many chars currently compressed (0 to length)
    std::vector<Point> positions;         // All positions occupied by spring chars

    // Compression tracking
    Player* compressingPlayer;            // First player compressing (nullptr if none)
    Player* secondCompressingPlayer;      // Second player if dual compression
    int playerCompressionAmount;          // Compression by first player
    int secondPlayerCompressionAmount;    // Compression by second player

public:
    // Default constructor (for compatibility)
    Spring() : StaticObject(), length(1), orientation(Direction::HORIZONTAL),
               projectionDirection(Direction::RIGHT), wallAnchor(0, 0),
               compressionState(0), compressingPlayer(nullptr),
               secondCompressingPlayer(nullptr), playerCompressionAmount(0),
               secondPlayerCompressionAmount(0)
    {
        sprite = '#';
        type = ObjectType::SPRING;
        positions.push_back(Point(0, 0));
    }

    // Multi-character spring constructor
    Spring(const std::vector<Point>& springPositions, Direction orient,
           Direction projDir, const Point& anchor)
        : StaticObject(springPositions.empty() ? Point(0, 0) : springPositions[0], '#', ObjectType::SPRING),
          length(springPositions.size()), orientation(orient),
          projectionDirection(projDir), wallAnchor(anchor),
          compressionState(0), positions(springPositions),
          compressingPlayer(nullptr), secondCompressingPlayer(nullptr),
          playerCompressionAmount(0), secondPlayerCompressionAmount(0)
    {
    }

    GameObject *clone() const override { return new Spring(*this); }
    const char *getName() const override { return "Spring"; }

    bool isBlocking() const override { return false; }
    bool onExplosion() override { return true; }

    // Position queries
    bool occupiesPosition(int x, int y) const
    {
        for (const Point& p : positions)
        {
            if (p.x == x && p.y == y)
                return true;
        }
        return false;
    }

    // Getters
    int getLength() const { return length; }
    int getCompressionState() const { return compressionState; }
    Direction getOrientation() const { return orientation; }
    Direction getProjectionDirection() const { return projectionDirection; }

    // State management
    void compress(int numChars)
    {
        if (numChars >= 0 && numChars <= length)
            compressionState = numChars;
    }

    void release() { compressionState = 0; }

    // Override draw to handle compression visualization
    void draw() const override;

    // Spring compression and launch methods
    void startCompression(Player* player);
    bool continueCompression(Player* player, Direction moveDir);
    void releaseForPlayer(Player* player);
    bool updateLaunchedPlayer(Player* player, Room* room);
    bool isCompressedByPlayer(const Player* player) const;
    bool isTwoPlayerCompression() const;
};