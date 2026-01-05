#include "Spring.h"
#include "SpringLink.h"
#include "Room.h"
#include "Player.h"
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

//////////////////////////////////////////      Clone               //////////////////////////////////////////

Spring* Spring::clone() const
{
    Spring* newSpring = new Spring();
    newSpring->anchorPosition = this->anchorPosition;
    newSpring->compressionDir = this->compressionDir;
    newSpring->compressedCount = this->compressedCount;
    // Note: links vector will be populated later by Room after cloning SpringLinks
    return newSpring;
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
        return false;
    }

    // For first link: always allow if direction matches
    if (linkIndex == 0)
    {
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

    return true;
}

//////////////////////////////////////////    compressLink          //////////////////////////////////////////

void Spring::compressLink(int linkIndex, Room* room)
{
    if (linkIndex < 0 || linkIndex >= static_cast<int>(links.size()))
        return;

    links[linkIndex]->collapse(room);
    compressedCount++;
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
}

//////////////////////////////////////////  handlePlayerInteraction   //////////////////////////////////////////

Spring::InteractionResult Spring::handlePlayerInteraction(SpringLink* link, Player* player, Room* room)
{
    if (link == nullptr || player == nullptr)
    {
        return {false, false, Momentum() };
    }

    // Get player's current direction
    Direction moveDir = player->getCurrentDirection();

    // Check if compression is valid
    if (!canCompressLink(link->getLinkIndex(), moveDir))
    {
        if (isCompressed()) 
        {            
            Momentum launch = calculateLaunchMomentum();
            resetCompression(room);
            return {true, true, launch };
        } 
        else
        {
            return {false, false, Momentum() };
        }
    }

    // Compress this link
    compressLink(link->getLinkIndex(), room);

    // Check if should launch
    bool fullyCompressed = isFullyCompressed();

    if (!fullyCompressed && moveDir == compressionDir)
    {
        // Compressed but not ready to launch
        return {true, false, Momentum()};
    }

    // Launch triggered!
    Momentum launch = calculateLaunchMomentum();
    bool shouldLaunch = compressedCount > 0;

    if (!shouldLaunch)
    {
        return {true, false, Momentum() };
    }

    // Reset spring IMMEDIATELY after launch
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

//////////////////////////////////////////    destroyAllLinks       //////////////////////////////////////////

void Spring::destroyAllLinks()
{
    for (SpringLink* link : links)
    {
        if (link && link->isActive())
        {
            link->setActive(false);
        }
    }
}

//////////////////////////////////////////   allLinksInactive       //////////////////////////////////////////

bool Spring::allLinksInactive() const
{
    for (SpringLink* link : links)
    {
        if (link && link->isActive())
            return false;
    }
    return true;
}
