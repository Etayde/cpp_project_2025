#pragma once

//////////////////////////////////////       INCLUDES & FORWARDS       //////////////////////////////////////////

#include "Point.h"
#include "Constants.h"
#include "Momentum.h"
#include <vector>

class SpringLink;
class Room;
class Player;

//////////////////////////////////////////          Spring            //////////////////////////////////////////

// Spring manager class - coordinates SpringLink objects for compression and launch
class Spring
{
private:
    std::vector<SpringLink*> links;
    Point anchorPosition;
    Direction compressionDir;
    int compressedCount;
    Player* currCompressor;

public:
    // Launch calculation result
    struct LaunchData {
        bool shouldLaunch;
        int velocityX;
        int velocityY;
        int frames;
        Direction direction;

    };

    // Player interaction result
    struct InteractionResult {
        bool compressed;      // Was compression performed?
        bool launched;        // Was a launch triggered?
        Momentum momentum; // Momentum data if launch occurred
    };

    // Constructor/Destructor
    Spring();
    ~Spring();

    // Initialization - call from Room during scanning
    void initialize(const std::vector<SpringLink*>& springLinks,
                   const Point& anchor,
                   Direction projectionDir);

    // Compression validation
    bool canCompressLink(int linkIndex, Direction playerDir) const;

    // Compression execution
    void compressLink(int linkIndex, Room* room);

    // State queries
    bool isFullyCompressed() const;
    int getCompressionLevel() const { return compressedCount; }
    int getLinkCount() const { return static_cast<int>(links.size()); }
    bool playerSTAYcheck(Player& p, SpringLink& link) const;

    // Launch mechanics
    LaunchData calculateLaunch() const;
    Momentum calculateLaunchMomentum() const;
    void resetCompression(Room* room);

    // Player interaction - encapsulates compression and launch logic
    InteractionResult handlePlayerInteraction(SpringLink* link, Player* player, Room* room);

    // Getters
    Direction getCompressionDir() const { return compressionDir; }

};
