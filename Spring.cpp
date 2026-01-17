//////////////////////////////////////       INCLUDES & FORWARDS       /////////////////////////////////////////////

#include "Spring.h"
#include "SpringLink.h"
#include "Room.h"
#include "Player.h"
///#include <algorithm>

//////////////////////////////////////////      Constructor          //////////////////////////////////////////

Spring::Spring()
    : links(), anchorPosition(-1, -1), compressionDir(Direction::STAY), compressedCount(0)
{
}

//////////////////////////////////////////      Destructor          //////////////////////////////////////////

Spring::~Spring() {}

//////////////////////////////////////////      Clone               //////////////////////////////////////////

Spring *Spring::clone() const
{
    Spring *newSpring = new Spring();
    newSpring->anchorPosition = this->anchorPosition;
    newSpring->compressionDir = this->compressionDir;
    newSpring->compressedCount = this->compressedCount;
    return newSpring;
}

//////////////////////////////////////////      Initialize          //////////////////////////////////////////

void Spring::initialize(const std::vector<SpringLink *> &springLinks,
                        const Point &anchor,
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
    if (linkIndex < 0 || linkIndex >= static_cast<int>(links.size()))
    {
        return false;
    }

    if (playerDir != compressionDir)
    {
        return false;
    }

    if (linkIndex == 0)
    {
        return true;
    }

    for (int i = 0; i < linkIndex; i++)
    {
        if (!links[i]->isCollapsed())
        {
            return false;
        }
    }

    if (links[linkIndex]->isCollapsed())
    {
        return false;
    }

    return true;
}

//////////////////////////////////////////    compressLink          //////////////////////////////////////////

void Spring::compressLink(int linkIndex, Room *room)
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

    switch (compressionDir)
    {
    case Direction::UP:
        launch.velocityX = 0;
        launch.velocityY = compressedCount;
        launch.direction = Direction::DOWN;
        break;
    case Direction::DOWN:
        launch.velocityX = 0;
        launch.velocityY = -compressedCount;
        launch.direction = Direction::UP;
        break;
    case Direction::LEFT:
        launch.velocityX = compressedCount;
        launch.velocityY = 0;
        launch.direction = Direction::RIGHT;
        break;
    case Direction::RIGHT:
        launch.velocityX = -compressedCount;
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

    switch (compressionDir)
    {
    case Direction::UP:
        momentum.setDY(compressedCount);
        momentum.setLaunchDir(Direction::DOWN);
        break;
    case Direction::DOWN:
        momentum.setDY(-compressedCount);
        momentum.setLaunchDir(Direction::UP);
        break;
    case Direction::LEFT:
        momentum.setDX(compressedCount);
        momentum.setLaunchDir(Direction::RIGHT);
        break;
    case Direction::RIGHT:
        momentum.setDX(-compressedCount);
        momentum.setLaunchDir(Direction::LEFT);
        break;
    default:
        break;
    }

    return momentum;
}

//////////////////////////////////////////  resetCompression        //////////////////////////////////////////

void Spring::resetCompression(Room *room)
{
    for (SpringLink *link : links)
    {
        link->reset(room);
    }
    compressedCount = 0;
}

//////////////////////////////////////////  handlePlayerInteraction   //////////////////////////////////////////

Spring::InteractionResult Spring::handlePlayerInteraction(SpringLink *link, Player *player, Room *room)
{
    if (link == nullptr || player == nullptr)
    {
        return {false, false, Momentum()};
    }

    Direction moveDir = player->getCurrentDirection();

    if (!canCompressLink(link->getLinkIndex(), moveDir))
    {
        if (isCompressed())
        {
            Momentum launch = calculateLaunchMomentum();
            resetCompression(room);
            return {true, true, launch};
        }
        else
        {
            return {false, false, Momentum()};
        }
    }

    compressLink(link->getLinkIndex(), room);

    bool fullyCompressed = isFullyCompressed();

    if (!fullyCompressed && moveDir == compressionDir)
    {
        return {true, false, Momentum()};
    }

    Momentum launch = calculateLaunchMomentum();
    bool shouldLaunch = compressedCount > 0;

    if (!shouldLaunch)
    {
        return {true, false, Momentum()};
    }

    resetCompression(room);

    return {true, true, launch};
}

//////////////////////////////////////////    Spring State Helpers        /////////////////////////////////////////////

bool Spring::playerSTAYcheck(Player &p, SpringLink &link) const
{

    if (p.getX() == link.getX() && p.getY() == link.getY() && (!link.isCollapsed()) && p.getCurrentDirection() == Direction::STAY)
    {
        return true;
    }
    return false;
}

SpringLink *Spring::getPrevLink(const SpringLink *current) const
{
    if (current == nullptr)
        return nullptr;

    int curr = current->getLinkIndex();
    return curr > 0 ? links[curr - 1] : nullptr;
}

//////////////////////////////////////////    destroyAllLinks       //////////////////////////////////////////

void Spring::destroyAllLinks()
{
    for (SpringLink *link : links)
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
    for (SpringLink *link : links)
    {
        if (link && link->isActive())
            return false;
    }
    return true;
}
