//////////////////////////////////////       INCLUDES & FORWARDS       //////////////////////////////////////////

#include "Player.h"
#include "Room.h"
#include "Switch.h"
#include "Door.h"
#include "Spring.h"
#include "SpringLink.h"
#include "Riddle.h"
#include "DebugLog.h"

//////////////////////////////////////////     Player Constructors     //////////////////////////////////////////

Player::Player()
    : inventory(nullptr), playerId(0), sprite(' '), prevChar(' '),
      atDoor(false), doorId(-1), alive(true), keyCount(0), lives(3),
      waitingAtDoor(false), requestPause(false), launchFramesRemaining(0)
{
    pos = Point(1, 1, 0, 0, ' ');
}

Player::Player(int id, int startX, int startY, char playerSprite)
    : inventory(nullptr), playerId(id), sprite(playerSprite), prevChar(' '),
      atDoor(false), doorId(-1), alive(true), keyCount(0), lives(3),
      waitingAtDoor(false), requestPause(false), launchFramesRemaining(0)
{
    pos = Point(startX, startY, 0, 0, playerSprite);
}

//////////////////////////////////////////      Player Destructor      //////////////////////////////////////////

Player::~Player()
{
    clearInventory();
}

//////////////////////////////////////////   Player Copy Constructor   //////////////////////////////////////////

Player::Player(const Player &other)
    : pos(other.pos), inventory(nullptr), playerId(other.playerId),
      sprite(other.sprite), prevChar(other.prevChar), atDoor(other.atDoor),
      doorId(other.doorId), alive(other.alive), keyCount(other.keyCount),
      lives(other.lives), waitingAtDoor(other.waitingAtDoor),
      requestPause(other.requestPause), launchFramesRemaining(other.launchFramesRemaining)
{
    copyInventoryFrom(other);
}

//////////////////////////////////////////  Player Assignment Operator //////////////////////////////////////////

Player &Player::operator=(const Player &other)
{
    if (this != &other)
    {
        clearInventory();

        pos = other.pos;
        playerId = other.playerId;
        sprite = other.sprite;
        prevChar = other.prevChar;
        atDoor = other.atDoor;
        doorId = other.doorId;
        alive = other.alive;
        keyCount = other.keyCount;
        lives = other.lives;
        waitingAtDoor = other.waitingAtDoor;
        requestPause = other.requestPause;
        launchFramesRemaining = other.launchFramesRemaining;

        copyInventoryFrom(other);
    }
    return *this;
}

//////////////////////////////////////////       Private Helpers       //////////////////////////////////////////

void Player::clearInventory()
{
    delete inventory;
    inventory = nullptr;
}

// Deep copy using polymorphic clone()
void Player::copyInventoryFrom(const Player &other)
{
    if (other.inventory != nullptr)
    {
        inventory = other.inventory->clone();
    }
}

//////////////////////////////////////////           move             //////////////////////////////////////////

// Move player in current direction, handle collisions and interactions
bool Player::move(Room *room, Riddle** activeRiddle, Player** activePlayer, Player* otherPlayer)
{
    if (room == nullptr)
        return false;

    // Not moving - just redraw
    if (pos.diff_x == 0 && pos.diff_y == 0)
    {
        draw(room);
        return false;
    }

    // Collision prediction during launch
    if (launchFramesRemaining > 0)
    {
        int stopX, stopY;
        if (predictCollisionAlongTrajectory(room, stopX, stopY))
        {
            stopAtPosition(stopX, stopY);
            draw(room);
            return false;
        }
    }

    int nextX = pos.x + pos.diff_x;
    int nextY = pos.y + pos.diff_y;

    // Check player collision ONLY when launched
    if (launchFramesRemaining > 0 &&
        otherPlayer != nullptr && otherPlayer->isAlive() &&
        otherPlayer->pos.x == nextX && otherPlayer->pos.y == nextY)
    {
        // Transfer momentum to other player
        otherPlayer->pos.diff_x = pos.diff_x;
        otherPlayer->pos.diff_y = pos.diff_y;
        otherPlayer->launchFramesRemaining = launchFramesRemaining;

        // Stop this player
        pos.diff_x = 0;
        pos.diff_y = 0;
        launchFramesRemaining = 0;
        draw(room);
        return false;
    }

    // Check absolute screen bounds
    if (nextX < 0 || nextX >= MAX_X || nextY < 1 || nextY >= MAX_Y_INGAME - 1)
    {
        pos.diff_x = 0;
        pos.diff_y = 0;
        draw(room);
        return false;
    }

    // Check if outside normal playable bounds (1-78)
    bool outsideNormalBounds = (nextX < 1 || nextX >= MAX_X - 1);

    if (outsideNormalBounds)
    {
        // Only allow if there's a door at this position
        GameObject *obj = room->getObjectAt(nextX, nextY);
        if (obj == nullptr || obj->getType() != ObjectType::DOOR)
        {
            pos.diff_x = 0;
            pos.diff_y = 0;
            draw(room);
            return false;
        }
    }

    // Check wall collision
    if (checkWallCollision(nextX, nextY, room))
    {
        pos.diff_x = 0;
        pos.diff_y = 0;
        draw(room);
        return false;
    }

    // Check object interaction
    if (checkObjectInteraction(nextX, nextY, room, activeRiddle, activePlayer))
    {
        pos.diff_x = 0;
        pos.diff_y = 0;
        draw(room);
        return false;
    }

    // Erase from current position
    gotoxy(pos.x, pos.y);
    std::cout << prevChar << std::flush;

    // Store character at new position
    GameObject *obj = room->getObjectAt(nextX, nextY);
    char nextChar = room->getCharAt(nextX, nextY);

    if (obj != nullptr && obj->getType() == ObjectType::DOOR)
    {
        prevChar = obj->getSprite();
    }
    else
    {
        prevChar = (nextChar == ' ' || nextChar == sprite) ? ' ' : nextChar;
    }

    // Update position and draw
    pos.x = nextX;
    pos.y = nextY;

    draw(room);

    return true;
}

//////////////////////////////////////////           draw               //////////////////////////////////////////

// Draw player (hide if in dark zone without visibility)
void Player::draw(Room *room)
{
    gotoxy(pos.x, pos.y);

    // Don't draw if waiting at door (player has "crossed through")
    if (waitingAtDoor)
    {
        std::cout << prevChar << std::flush;
        return;
    }

    // Don't draw player if in dark zone without visibility
    if (room != nullptr && room->isInDarkZone(pos.x, pos.y) && !room->isVisible(pos.x, pos.y))
    {
        std::cout << ' ' << std::flush;
    }
    else
    {
        std::cout << sprite << std::flush;
    }
}

//////////////////////////////////////////        pickupItem           //////////////////////////////////////////

// Pick up an item (ownership transferred via clone)
bool Player::pickupItem(GameObject *item)
{
    if (item == nullptr || hasItem())
        return false;
    if (!item->isPickable())
        return false;

    if (item->getType() == ObjectType::KEY)
        keyCount++;

    inventory = item->clone();
    item->setActive(false);

    updateInventoryDisplay();
    return true;
}

//////////////////////////////////////////         dropItem            //////////////////////////////////////////

// Drop current item, returns position or (-1,-1) if failed
Point Player::dropItem(Room *room)
{
    Point dropPos(-1, -1);

    if (!hasItem() || room == nullptr)
        return dropPos;

    // Find empty spot around player
    const int checkOffsets[][2] = {
        {0, 1}, {0, -1}, {1, 0}, {-1, 0}, {1, 1}, {1, -1}, {-1, 1}, {-1, -1}};

    int dropX = -1, dropY = -1;
    bool found = false;

    for (int i = 0; i < 8 && !found; i++)
    {
        int testX = pos.x + checkOffsets[i][0];
        int testY = pos.y + checkOffsets[i][1];

        if (testX >= 1 && testX < MAX_X - 1 && testY >= 1 && testY < MAX_Y_INGAME - 1)
        {
            char c = room->getCharAt(testX, testY);
            GameObject *existing = room->getObjectAt(testX, testY);

            if ((c == ' ' || c == '.') && existing == nullptr)
            {
                dropX = testX;
                dropY = testY;
                found = true;
            }
        }
    }

    if (!found)
        return dropPos;

    // Create dropped item
    GameObject *droppedItem = inventory->clone();
    droppedItem->setPosition(dropX, dropY);
    droppedItem->setActive(true);

    if (room->addObject(droppedItem))
    {
        dropPos.x = dropX;
        dropPos.y = dropY;

        if (inventory->getType() == ObjectType::KEY)
            keyCount--;

        clearInventory();
        updateInventoryDisplay();

        gotoxy(dropX, dropY);
        std::cout << droppedItem->getSprite() << std::flush;
    }
    else
    {
        delete droppedItem;
    }

    return dropPos;
}

//////////////////////////////////////////       performAction         //////////////////////////////////////////

void Player::performAction(Action action, Room* room)
{
    // Debug: Log launchFramesRemaining at start of performAction
    DebugLog::getStream() << "[PERFORM_ACTION_START] Player " << playerId
                          << " launchFrames: " << launchFramesRemaining
                          << " | Action: " << static_cast<int>(action) << std::endl;

    // Check if currently launched
    if (launchFramesRemaining > 0)
    {
        DebugLog::getStream() << "[PLAYER_PERFORM_ACTION] Player " << playerId
                              << " is launched (frames:" << launchFramesRemaining
                              << " vel:" << pos.diff_x << "," << pos.diff_y
                              << ") | Action: " << static_cast<int>(action) << std::endl;

        Direction inputDir = actionToDirection(action);

        // Block invalid inputs (opposite direction or STAY)
        if (!canApplyInputDuringLaunch(inputDir))
        {
            DebugLog::getStream() << "[PLAYER_PERFORM_ACTION] Input blocked during launch" << std::endl;
            return;  // Ignore this input
        }

        // Allow perpendicular movement
        Direction launchDir = getLaunchDirection();
        if (isPerpendicularToLaunch(inputDir, launchDir))
        {
            DebugLog::getStream() << "[PLAYER_PERFORM_ACTION] Applying perpendicular velocity" << std::endl;
            applyPerpendicularVelocity(inputDir);
        }
        return;
    }

    // Normal movement (not launched)
    DebugLog::getStream() << "[PLAYER_PERFORM_ACTION] Player " << playerId
                          << " normal movement | Action: " << static_cast<int>(action) << std::endl;

    switch (action)
    {
    case Action::MOVE_UP:
        pos.setDirection(Direction::UP);
        break;
    case Action::MOVE_DOWN:
        pos.setDirection(Direction::DOWN);
        break;
    case Action::MOVE_LEFT:
        pos.setDirection(Direction::LEFT);
        break;
    case Action::MOVE_RIGHT:
        pos.setDirection(Direction::RIGHT);
        break;
    case Action::STAY:
        pos.setDirection(Direction::STAY);
        break;
    case Action::DROP_ITEM:
        break; // Handled by game loop
    default:
        pos.setDirection(Direction::STAY);
        break;
    }
}

//////////////////////////////////////////   updateInventoryDisplay    //////////////////////////////////////////

// Show keys count and current item in UI area
void Player::updateInventoryDisplay()
{
    int invX = (playerId == 1) ? InventoryUI::PLAYER1_X : InventoryUI::PLAYER2_X;
    int invY = InventoryUI::Y_POS;

    gotoxy(invX, invY);
    std::cout << "Keys:" << keyCount << " [";

    if (hasItem())
        std::cout << inventory->getSprite();
    else
        std::cout << " ";

    std::cout << "]   " << std::flush;
}

//////////////////////////////////////////           useKey           //////////////////////////////////////////

// Use one key (returns true if successful)
bool Player::useKey()
{
    if (keyCount > 0)
    {
        keyCount--;
        updateInventoryDisplay();
        return true;
    }
    return false;
}

Direction Player::getCurrentDirection() const
{
    if (pos.diff_x == 0 && pos.diff_y == 0)
        return Direction::STAY;
    if (pos.diff_y < 0)
        return Direction::UP;
    if (pos.diff_y > 0)
        return Direction::DOWN;
    if (pos.diff_x < 0)
        return Direction::LEFT;
    if (pos.diff_x > 0)
        return Direction::RIGHT;
    return Direction::STAY;
}

Direction Player::actionToDirection(Action action) const
{
    switch (action)
    {
    case Action::MOVE_UP:    return Direction::UP;
    case Action::MOVE_DOWN:  return Direction::DOWN;
    case Action::MOVE_LEFT:  return Direction::LEFT;
    case Action::MOVE_RIGHT: return Direction::RIGHT;
    case Action::STAY:       return Direction::STAY;
    default:                 return Direction::STAY;
    }
}

//////////////////////////////////////////    checkWallCollision    //////////////////////////////////////////

// Check if next position has a wall - returns true if wall blocks movement
bool Player::checkWallCollision(int nextX, int nextY, Room* room)
{
    if (room == nullptr)
        return false;

    char nextChar = room->getCharAt(nextX, nextY);
    if (nextChar != 'W' && nextChar != '=')
        return false; // No wall

    // Wall blocks movement
    return true;
}

//////////////////////////////////////////  checkObjectInteraction  //////////////////////////////////////////

// Check object interaction at next position - returns true if object blocks movement
bool Player::checkObjectInteraction(int nextX, int nextY, Room* room, Riddle** activeRiddle, Player** activePlayer)
{
    if (room == nullptr)
        return false;

    GameObject* obj = room->getObjectAt(nextX, nextY);

    if (obj == nullptr || !obj->isActive())
    {
        // No active object - reset door state
        atDoor = false;
        doorId = -1;
        return false;
    }

    ObjectType objType = obj->getType();

    // Riddles - auto-trigger when stepped on
    if (objType == ObjectType::RIDDLE)
    {
        Riddle* riddle = dynamic_cast<Riddle*>(obj);
        if (riddle != nullptr)
        {
            // Store riddle IMMEDIATELY in Game's aRiddle (for pause persistence)
            if (activeRiddle != nullptr && activePlayer != nullptr)
            {
                *activeRiddle = riddle;
                *activePlayer = this;
            }

            // Enter riddle (blocks game) - pass 'this' to track triggering player
            RiddleResult result = riddle->enterRiddle(room, this);

            if (result == RiddleResult::SOLVED)
            {
                // Remove riddle from room entirely
                room->removeObjectAt(nextX, nextY);
                // Clear aRiddle since solved
                if (activeRiddle != nullptr) *activeRiddle = nullptr;
                if (activePlayer != nullptr) *activePlayer = nullptr;
                return false;  // Allow movement onto position
            }
            else if (result == RiddleResult::ESCAPED)
            {
                // Signal pause to game loop (aRiddle stays set for resuming)
                requestPause = true;
                return true;  // Block movement
            }
            else
            {
                // Failed - block movement, riddle stays, clear aRiddle
                if (activeRiddle != nullptr) *activeRiddle = nullptr;
                if (activePlayer != nullptr) *activePlayer = nullptr;
                return true;
            }
        }
    }

    // Switches - toggle and stop (handle BEFORE blocking check)
    if (objType == ObjectType::SWITCH_OFF || objType == ObjectType::SWITCH_ON)
    {
        Switch* sw = (Switch*)(obj);
        if (sw != nullptr)
        {
            sw->toggle();
            room->setCharAt(obj->getX(), obj->getY(), sw->getSprite());
            gotoxy(obj->getX(), obj->getY());
            std::cout << sw->getSprite() << std::flush;
            room->updatePuzzleState();
        }
        return true; // Blocks movement
    }

    // SpringLink compression and launch
    if (objType == ObjectType::SPRING_LINK)
    {
        DebugLog::getStream() << "[PLAYER_SPRING] Player " << playerId
                              << " stepped on SPRING_LINK at (" << nextX << "," << nextY << ")" << std::endl;

        SpringLink* link = dynamic_cast<SpringLink*>(obj);
        if (link == nullptr)
        {
            DebugLog::getStream() << "[PLAYER_SPRING] ERROR: Failed to cast to SpringLink!" << std::endl;
            return false;
        }

        DebugLog::getStream() << "[PLAYER_SPRING] Link#" << link->getLinkIndex()
                              << " | Collapsed: " << (link->isCollapsed() ? "YES" : "NO") << std::endl;

        Spring* spring = link->getParentSpring();
        if (spring == nullptr)
        {
            DebugLog::getStream() << "[PLAYER_SPRING] ERROR: Link has no parent Spring!" << std::endl;
            return false;
        }

        // Get player's current direction (uses pos.diff_x and pos.diff_y)
        Direction moveDir = getCurrentDirection();
        DebugLog::getStream() << "[PLAYER_SPRING] Player direction: " << static_cast<int>(moveDir)
                              << " (diff_x:" << pos.diff_x << " diff_y:" << pos.diff_y << ")" << std::endl;

        // Check if compression is valid
        if (spring->canCompressLink(link->getLinkIndex(), moveDir))
        {
            DebugLog::getStream() << "[PLAYER_SPRING] Compression valid - compressing link" << std::endl;

            // Compress this link
            spring->compressLink(link->getLinkIndex(), room);

            DebugLog::getStream() << "[PLAYER_SPRING] After compression - Level: "
                                  << spring->getCompressionLevel() << "/"
                                  << spring->getLinkCount() << std::endl;

            // Check if should launch
            bool fullyCompressed = spring->isFullyCompressed();
            bool stayPressed = (moveDir == Direction::STAY);

            DebugLog::getStream() << "[PLAYER_SPRING] Launch check - FullyCompressed: "
                                  << (fullyCompressed ? "YES" : "NO")
                                  << " | STAY pressed: " << (stayPressed ? "YES" : "NO") << std::endl;

            if (fullyCompressed || stayPressed)
            {
                DebugLog::getStream() << "[PLAYER_SPRING] Launch triggered!" << std::endl;

                Spring::LaunchData launch = spring->calculateLaunch();

                if (launch.shouldLaunch)
                {
                    // Apply launch velocity
                    pos.diff_x = launch.velocityX;
                    pos.diff_y = launch.velocityY;
                    launchFramesRemaining = launch.frames;

                    DebugLog::getStream() << "[SPRING_LAUNCH] Player " << playerId
                                          << " launched: vel(" << launch.velocityX
                                          << "," << launch.velocityY
                                          << ") frames:" << launch.frames << std::endl;
                }
                else
                {
                    DebugLog::getStream() << "[PLAYER_SPRING] WARNING: shouldLaunch=false" << std::endl;
                }

                // Reset spring IMMEDIATELY after launch
                DebugLog::getStream() << "[PLAYER_SPRING] Resetting spring..." << std::endl;
                spring->resetCompression(room);
            }
        }
        else
        {
            DebugLog::getStream() << "[PLAYER_SPRING] Compression not valid - passing through" << std::endl;
        }

        return false;  // SpringLinks are non-blocking
    }

    // Blocking objects
    if (obj->isBlocking())
    {
        return true; // Blocks movement
    }

    // Pickable objects - pick up if inventory empty
    if (obj->isPickable() && !hasItem())
    {
        if (objType == ObjectType::KEY)
            keyCount++;

        inventory = obj->clone();
        obj->setActive(false);
        room->setCharAt(nextX, nextY, ' ');

        updateInventoryDisplay();
    }

    // Doors - mark player at door
    if (objType == ObjectType::DOOR)
    {
        atDoor = true;
        Door* door = dynamic_cast<Door*>(obj);
        doorId = (door != nullptr) ? door->getDoorId() : (obj->getSprite() - '0');
    }
    else
    {
        atDoor = false;
        doorId = -1;
    }

    return false; // Doesn't block movement
}

//////////////////////////////////////////   fullyCompressedSpring   //////////////////////////////////////////

bool Player::fullyCompressedSpring(const Spring& s, int checkX, int checkY) const
{
    // TODO: Implement spring compression check
    (void)s;
    (void)checkX;
    (void)checkY;
    return false;
}

//////////////////////////////////////////   getLaunchDirection   //////////////////////////////////////////

Direction Player::getLaunchDirection() const
{
    // During launch, the dominant velocity component indicates direction
    if (abs(pos.diff_x) >= abs(pos.diff_y))
    {
        return (pos.diff_x > 0) ? Direction::RIGHT : Direction::LEFT;
    }
    else
    {
        return (pos.diff_y > 0) ? Direction::DOWN : Direction::UP;
    }
}

//////////////////////////////////////////   isPerpendicularToLaunch   //////////////////////////////////////////

bool Player::isPerpendicularToLaunch(Direction inputDir, Direction launchDir) const
{
    // Horizontal launches: perpendicular is UP/DOWN
    if (launchDir == Direction::LEFT || launchDir == Direction::RIGHT)
    {
        return (inputDir == Direction::UP || inputDir == Direction::DOWN);
    }

    // Vertical launches: perpendicular is LEFT/RIGHT
    if (launchDir == Direction::UP || launchDir == Direction::DOWN)
    {
        return (inputDir == Direction::LEFT || inputDir == Direction::RIGHT);
    }

    return false;
}

//////////////////////////////////////////   isCellBlocking   //////////////////////////////////////////

bool Player::isCellBlocking(int x, int y, Room* room) const
{
    if (room == nullptr)
        return true;

    // Check bounds
    if (x < 0 || x >= MAX_X || y < 1 || y >= MAX_Y_INGAME - 1)
        return true;

    // Check wall collision
    char cellChar = room->getCharAt(x, y);
    if (cellChar == 'W' || cellChar == '=')
        return true;

    // Check blocking objects
    GameObject* obj = room->getObjectAt(x, y);
    if (obj != nullptr && obj->isActive() && obj->isBlocking())
        return true;

    return false;
}

//////////////////////////////////////////   canApplyInputDuringLaunch   //////////////////////////////////////////

bool Player::canApplyInputDuringLaunch(Direction inputDir) const
{
    Direction launchDir = getLaunchDirection();

    // Always block STAY command during launch
    if (inputDir == Direction::STAY)
        return false;

    // Get opposite direction of launch
    Direction oppositeDir;
    switch (launchDir)
    {
        case Direction::UP:    oppositeDir = Direction::DOWN; break;
        case Direction::DOWN:  oppositeDir = Direction::UP; break;
        case Direction::LEFT:  oppositeDir = Direction::RIGHT; break;
        case Direction::RIGHT: oppositeDir = Direction::LEFT; break;
        default: return false;
    }

    // Block if input is opposite to launch direction
    if (inputDir == oppositeDir)
        return false;

    // Block if input is same as launch direction (redundant)
    if (inputDir == launchDir)
        return false;

    // Allow perpendicular directions
    return true;
}

//////////////////////////////////////////   applyPerpendicularVelocity   //////////////////////////////////////////

void Player::applyPerpendicularVelocity(Direction perpendicularDir)
{
    // Set perpendicular component to speed 1
    // Keep launch component unchanged

    Direction launchDir = getLaunchDirection();

    // If launch is horizontal (LEFT/RIGHT)
    if (launchDir == Direction::LEFT || launchDir == Direction::RIGHT)
    {
        // Launch velocity in diff_x stays same
        // Apply perpendicular in diff_y
        if (perpendicularDir == Direction::UP)
            pos.diff_y = -1;
        else if (perpendicularDir == Direction::DOWN)
            pos.diff_y = 1;
    }
    // If launch is vertical (UP/DOWN)
    else if (launchDir == Direction::UP || launchDir == Direction::DOWN)
    {
        // Launch velocity in diff_y stays same
        // Apply perpendicular in diff_x
        if (perpendicularDir == Direction::LEFT)
            pos.diff_x = -1;
        else if (perpendicularDir == Direction::RIGHT)
            pos.diff_x = 1;
    }
}

//////////////////////////////////////////   predictCollisionAlongTrajectory   //////////////////////////////////////////

bool Player::predictCollisionAlongTrajectory(Room* room, int& stopX, int& stopY) const
{
    // Bresenham-style line traversal from current position to next position
    int currentX = pos.x;
    int currentY = pos.y;
    int targetX = pos.x + pos.diff_x;
    int targetY = pos.y + pos.diff_y;

    int dx = abs(targetX - currentX);
    int dy = abs(targetY - currentY);
    int sx = (currentX < targetX) ? 1 : -1;
    int sy = (currentY < targetY) ? 1 : -1;

    int err = dx - dy;
    int checkX = currentX;
    int checkY = currentY;

    // Store last valid position
    stopX = currentX;
    stopY = currentY;

    while (true)
    {
        // Check next step
        int e2 = 2 * err;
        int nextCheckX = checkX;
        int nextCheckY = checkY;

        if (e2 > -dy)
        {
            err -= dy;
            nextCheckX += sx;
        }
        if (e2 < dx)
        {
            err += dx;
            nextCheckY += sy;
        }

        // Reached target without collision
        if (nextCheckX == targetX && nextCheckY == targetY)
        {
            // Check if target itself is blocking
            if (isCellBlocking(targetX, targetY, room))
            {
                return true;  // Collision at target
            }
            return false;  // No collision
        }

        // Check if this cell is blocking
        if (isCellBlocking(nextCheckX, nextCheckY, room))
        {
            return true;  // Collision detected, stopX/stopY has last safe position
        }

        // Update last safe position
        stopX = nextCheckX;
        stopY = nextCheckY;
        checkX = nextCheckX;
        checkY = nextCheckY;
    }
}

//////////////////////////////////////////   stopAtPosition   //////////////////////////////////////////

void Player::stopAtPosition(int x, int y)
{
    // Update position to safe cell
    pos.x = x;
    pos.y = y;

    // CRITICAL: Reset velocity to zero
    pos.diff_x = 0;
    pos.diff_y = 0;

    // Reset launch state
    launchFramesRemaining = 0;
}


