#include "Spring.h"
#include "SpringLink.h"
#include "Room.h"
#include "Player.h"
#include "DebugLog.h"
#include <algorithm>

//////////////////////////////////////////      Constructor          //////////////////////////////////////////

Spring::Spring()
    : links(), anchorPosition(-1, -1), compressionDir(Direction::STAY), compressedCount(0)
{
}

//////////////////////////////////////////      Destructor          //////////////////////////////////////////

Spring::~Spring()
{
    // Links are owned by Room's objects vector, don't delete them here
}

//////////////////////////////////////////      Initialize          //////////////////////////////////////////

void Spring::initialize(const std::vector<SpringLink*>& springLinks,
                       const Point& anchor,
                       Direction projectionDir)
{
    links = springLinks;
    anchorPosition = anchor;
    compressionDir = projectionDir;
    compressedCount = 0;

    DebugLog::getStream() << "[SPRING_INIT] Spring with " << links.size()
                          << " links | Anchor:(" << anchor.x << "," << anchor.y << ")"
                          << " | ComprDir: " << static_cast<int>(compressionDir) << std::endl;
}

//////////////////////////////////////////    canCompressLink       //////////////////////////////////////////

bool Spring::canCompressLink(int linkIndex, Direction playerDir) const
{
    // Must be valid index
    if (linkIndex < 0 || linkIndex >= static_cast<int>(links.size()))
    {
        return false;
    }

    // Must move in compression direction (or STAY for launch trigger)
    if (playerDir != compressionDir)
    {
        DebugLog::getStream() << "[SPRING_CAN_COMPRESS] FAIL: Wrong direction - player: "
                              << static_cast<int>(playerDir) << " vs compression: "
                              << static_cast<int>(compressionDir) << std::endl;
        return false;
    }

    // For first link: always allow if direction matches
    if (linkIndex == 0)
    {
        DebugLog::getStream() << "[SPRING_CAN_COMPRESS] SUCCESS: First link, direction matches" << std::endl;
        return true;
    }

    // For subsequent links: all previous links must be compressed
    for (int i = 0; i < linkIndex; i++)
    {
        if (!links[i]->isCollapsed())
        {
            return false;  // Gap in compression sequence
        }
    }

    // Link itself must not already be collapsed
    if (links[linkIndex]->isCollapsed())
    {
        return false;
    }

    DebugLog::getStream() << "[SPRING_CAN_COMPRESS] SUCCESS: All checks passed" << std::endl;
    return true;
}

//////////////////////////////////////////    compressLink          //////////////////////////////////////////

void Spring::compressLink(int linkIndex, Room* room)
{
    if (linkIndex < 0 || linkIndex >= static_cast<int>(links.size()))
        return;

    links[linkIndex]->collapse(room);
    compressedCount++;

    DebugLog::getStream() << "[SPRING_COMPRESS] Link " << linkIndex
                          << " compressed | Total: " << compressedCount
                          << "/" << links.size() << std::endl;
}

//////////////////////////////////////////  isFullyCompressed       //////////////////////////////////////////

bool Spring::isFullyCompressed() const
{
    return compressedCount >= static_cast<int>(links.size());
}

//////////////////////////////////////////   calculateLaunch        //////////////////////////////////////////

Spring::LaunchData Spring::calculateLaunch() const
{
    LaunchData launch;
    launch.shouldLaunch = (compressedCount > 0);
    launch.frames = compressedCount * compressedCount;

    // Launch OPPOSITE of compression direction
    switch (compressionDir)
    {
        case Direction::UP:
            launch.velocityX = 0;
            launch.velocityY = compressedCount;  // Launch DOWN
            launch.direction = Direction::DOWN;
            break;
        case Direction::DOWN:
            launch.velocityX = 0;
            launch.velocityY = -compressedCount;  // Launch UP
            launch.direction = Direction::UP;
            break;
        case Direction::LEFT:
            launch.velocityX = compressedCount;  // Launch RIGHT
            launch.velocityY = 0;
            launch.direction = Direction::RIGHT;
            break;
        case Direction::RIGHT:
            launch.velocityX = -compressedCount;  // Launch LEFT
            launch.velocityY = 0;
            launch.direction = Direction::LEFT;
            break;
        default:
            launch.shouldLaunch = false;
            launch.velocityX = 0;
            launch.velocityY = 0;
            launch.direction = Direction::STAY;
            break;
    }

    DebugLog::getStream() << "[SPRING_CALC_LAUNCH] Velocity:(" << launch.velocityX
                          << "," << launch.velocityY << ") Frames:" << launch.frames 
                          <<  " Dir:" << static_cast<int>(launch.direction) << std::endl;

    return launch;
}

//////////////////////////////////////////  calculateLaunchMomentum //////////////////////////////////////////

Momentum Spring::calculateLaunchMomentum() const
{
    Momentum momentum;
    momentum.setActive(true);
    momentum.setLaunchFramesRemaining(compressedCount * compressedCount);

    // Launch OPPOSITE of compression direction
    switch (compressionDir)
    {
        case Direction::UP:
            momentum.setDY(compressedCount);  // Launch DOWN
            momentum.setLaunchDir(Direction::DOWN);
            break;
        case Direction::DOWN:
            momentum.setDY(-compressedCount);  // Launch UP
            momentum.setLaunchDir(Direction::UP);
            break;
        case Direction::LEFT:
            momentum.setDX(compressedCount);  // Launch RIGHT
            momentum.setLaunchDir(Direction::RIGHT);
            break;
        case Direction::RIGHT:
            momentum.setDX(-compressedCount);  // Launch LEFT
            momentum.setLaunchDir(Direction::LEFT);
            break;
        default:
            break;
    }

    DebugLog::getStream() << "[SPRING_CALC_LAUNCH_MOMENTUM] Momentum:(" << momentum.getDX()
                          << "," << momentum.getDY() << ")" << std::endl;

    return momentum;
}

//////////////////////////////////////////  resetCompression        //////////////////////////////////////////

void Spring::resetCompression(Room* room)
{
    for (SpringLink* link : links)
    {
        link->reset(room);
    }
    compressedCount = 0;

    DebugLog::getStream() << "[SPRING_RESET] Spring reset - all links restored" << std::endl;
}

//////////////////////////////////////////  handlePlayerInteraction   //////////////////////////////////////////

Spring::InteractionResult Spring::handlePlayerInteraction(SpringLink* link, Player* player, Room* room)
{
    if (link == nullptr || player == nullptr)
    {
        return {false, false, Momentum() };
    }

    DebugLog::getStream() << "[PLAYER_SPRING] Player " << player->getId()
                          << " stepped on SPRING_LINK at (" << link->getX() << "," << link->getY() << ")" << std::endl;

    DebugLog::getStream() << "[PLAYER_SPRING] Link#" << link->getLinkIndex()
                          << " | Collapsed: " << (link->isCollapsed() ? "YES" : "NO") << std::endl;

    // Get player's current direction
    Direction moveDir = player->getCurrentDirection();
    DebugLog::getStream() << "[PLAYER_SPRING] Player direction: " << static_cast<int>(moveDir) << std::endl;

    // Check if compression is valid
    if (!canCompressLink(link->getLinkIndex(), moveDir))

    {
        DebugLog::getStream() << "[PLAYER_SPRING] Can't compress link" << std::endl;
        
        if (isCompressed()) 
        {            
            DebugLog::getStream() << "[SPRING_STATE] Current compression: " << compressedCount << std::endl;

            Momentum launch = calculateLaunchMomentum();

            DebugLog::getStream() << "[SPRING_STAE] Momentum calculated" << std::endl;

            resetCompression(room);
            return {true, true, launch };
        } 
        else
        {
            DebugLog::getStream() << "[PLAYER_SPRING] Spring not compressed - player moved" << std::endl;

            return {false, false, Momentum() };
        }
    }

    // Compress this link
    compressLink(link->getLinkIndex(), room);

    DebugLog::getStream() << "[PLAYER_SPRING] Spring compressed" << std::endl;

    // Check if should launch
    bool fullyCompressed = isFullyCompressed();

    if (!fullyCompressed && moveDir == compressionDir)
    {
        // Compressed but not ready to launch
        return {true, false, Momentum()};
    }

    // Launch triggered!
    DebugLog::getStream() << "[PLAYER_SPRING] Launch triggered!" << std::endl;

    Momentum launch = calculateLaunchMomentum();
    bool shouldLaunch = compressedCount > 0;

    if (!shouldLaunch)
    {
        DebugLog::getStream() << "[PLAYER_SPRING] WARNING: shouldLaunch=false" << std::endl;
        return {true, false, Momentum() };
    }

    DebugLog::getStream() << "[SPRING_LAUNCH] Player " << player->getId()
                          << " launched: vel(" << launch.getDX()
                          << "," << launch.getDY()
                          << ") frames:" << launch.getLaunchFramesRemaining() << std::endl;

    // Reset spring IMMEDIATELY after launch
    DebugLog::getStream() << "[PLAYER_SPRING] Resetting spring..." << std::endl;
    resetCompression(room);

    // Return launch data for Player to apply
    return {true, true, launch};
}


bool Spring::playerSTAYcheck(Player& p, SpringLink& link) const{

    if (p.getX() == link.getX() && p.getY() == link.getY() 
        && (!link.isCollapsed())
        && p.getCurrentDirection() == Direction::STAY){
        return true;
    }
    return false;
}


SpringLink* Spring::getPrevLink(const SpringLink* current) const{
    if (current == nullptr)
        return nullptr;
    
    int curr = current->getLinkIndex();
    return curr > 0 ? links[curr - 1] : nullptr;
}

