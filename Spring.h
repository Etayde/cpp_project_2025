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

// Spring manager class
class Spring
{
private:
    std::vector<SpringLink *> links;
    Point anchorPosition;
    Direction compressionDir;
    int compressedCount;

public:
    struct LaunchData
    {
        bool shouldLaunch;
        int velocityX;
        int velocityY;
        int frames;
        Direction direction;
    };

    struct InteractionResult
    {
        bool compressed;
        bool launched;
        Momentum momentum;
    };

    Spring();
    ~Spring();

    Spring *clone() const;

    void initialize(const std::vector<SpringLink *> &springLinks,
                    const Point &anchor,
                    Direction projectionDir);

    InteractionResult handlePlayerInteraction(SpringLink *link, Player *player, Room *room);

    // Getters
    Direction getCompressionDir() const { return compressionDir; }
    bool isCompressed() const { return compressedCount > 0; }

    // Destruction
    void destroyAllLinks();
    bool allLinksInactive() const;

private:
    bool canCompressLink(int linkIndex, Direction playerDir) const;

    void compressLink(int linkIndex, Room *room);

    bool isFullyCompressed() const;
    int getCompressionLevel() const { return compressedCount; }
    int getLinkCount() const { return static_cast<int>(links.size()); }
    bool playerSTAYcheck(Player &p, SpringLink &link) const;

    LaunchData calculateLaunch() const;
    Momentum calculateLaunchMomentum() const;
    void resetCompression(Room *room);

    // Getters
    SpringLink *getPrevLink(const SpringLink *current) const;
};
