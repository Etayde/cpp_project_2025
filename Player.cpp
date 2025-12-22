//////////////////////////////////////       INCLUDES & FORWARDS       //////////////////////////////////////////

#include "Player.h"
#include "Room.h"
#include "Switch.h"
#include "Door.h"
#include "Spring.h"

//////////////////////////////////////////     Player Constructors     //////////////////////////////////////////

Player::Player()
    : inventory(nullptr), playerId(0), sprite(' '), prevChar(' '),
      atDoor(false), doorId(-1), alive(true), keyCount(0), waitingAtDoor(false),
      inSpringMotion(false), springMomentum(0), springDirection(Direction::STAY),
      springFramesRemaining(0), activeSpring(nullptr), springCompressionProgress(0)
{
    pos = Point(1, 1, 0, 0, ' ');
}

Player::Player(int id, int startX, int startY, char playerSprite)
    : inventory(nullptr), playerId(id), sprite(playerSprite), prevChar(' '),
      atDoor(false), doorId(-1), alive(true), keyCount(0), waitingAtDoor(false),
      inSpringMotion(false), springMomentum(0), springDirection(Direction::STAY),
      springFramesRemaining(0), activeSpring(nullptr), springCompressionProgress(0)
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
      waitingAtDoor(other.waitingAtDoor),
      inSpringMotion(other.inSpringMotion), springMomentum(other.springMomentum),
      springDirection(other.springDirection), springFramesRemaining(other.springFramesRemaining),
      activeSpring(other.activeSpring), springCompressionProgress(other.springCompressionProgress)
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
        waitingAtDoor = other.waitingAtDoor;

        inSpringMotion = other.inSpringMotion;
        springMomentum = other.springMomentum;
        springDirection = other.springDirection;
        springFramesRemaining = other.springFramesRemaining;
        activeSpring = other.activeSpring;
        springCompressionProgress = other.springCompressionProgress;

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
bool Player::move(Room *room)
{
    if (room == nullptr)
        return false;

    // Spring motion takes priority
    if (inSpringMotion)
    {
        return moveWithSpringMomentum(room);
    }

    // Not moving - just redraw
    if (pos.diff_x == 0 && pos.diff_y == 0)
    {
        draw(room);
        return false;
    }

    int nextX = pos.x + pos.diff_x;
    int nextY = pos.y + pos.diff_y;

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
    char nextChar = room->getCharAt(nextX, nextY);
    if (nextChar == 'W' || nextChar == '=')
    {
        pos.diff_x = 0;
        pos.diff_y = 0;
        draw(room);
        return false;
    }

    // Check object collision/interaction
    GameObject *obj = room->getObjectAt(nextX, nextY);

    if (obj != nullptr && obj->isActive())
    {
        ObjectType objType = obj->getType();

        // Switches - toggle and stop (handle BEFORE blocking check)
        if (objType == ObjectType::SWITCH_OFF || objType == ObjectType::SWITCH_ON)
        {
            Switch *sw = (Switch *)(obj);
            if (sw != nullptr)
            {
                sw->toggle();
                room->setCharAt(obj->getX(), obj->getY(), sw->getSprite());
                gotoxy(obj->getX(), obj->getY());
                std::cout << sw->getSprite() << std::flush;
                room->updatePuzzleState();
            }
            pos.diff_x = 0;
            pos.diff_y = 0;
            draw(room);
            return false;
        }

        // Blocking objects
        if (obj->isBlocking())
        {
            pos.diff_x = 0;
            pos.diff_y = 0;
            draw(room);
            return false;
        }

        // Pickable objects - pick up if inventory empty
        if (obj->isPickable() && !hasItem())
        {
            if (objType == ObjectType::KEY)
                keyCount++;

            inventory = obj->clone();
            obj->setActive(false);
            room->setCharAt(nextX, nextY, ' ');
            nextChar = ' ';

            updateInventoryDisplay();
        }

        // Doors - mark player at door
        if (objType == ObjectType::DOOR)
        {
            atDoor = true;
            Door *door = dynamic_cast<Door *>(obj);
            doorId = (door != nullptr) ? door->getDoorId() : (obj->getSprite() - '0');
        }
    }
    else
    {
        atDoor = false;
        doorId = -1;
    }

    // Erase from current position
    gotoxy(pos.x, pos.y);
    std::cout << prevChar << std::flush;

    // Store character at new position
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

    // Check if player is on an active spring (need to check activeSpring first,
    // because getObjectAt only returns the spring for its first position!)
    if (activeSpring != nullptr && activeSpring->occupiesPosition(nextX, nextY))
    {
        // Continue compressing the same spring
        Direction moveDir = getCurrentDirection();
        Direction springProj = activeSpring->getProjectionDirection();

        // Check if moving toward wall (opposite to projection direction)
        bool isCompressing = false;
        if (springProj == Direction::UP && moveDir == Direction::DOWN) isCompressing = true;
        if (springProj == Direction::DOWN && moveDir == Direction::UP) isCompressing = true;
        if (springProj == Direction::LEFT && moveDir == Direction::RIGHT) isCompressing = true;
        if (springProj == Direction::RIGHT && moveDir == Direction::LEFT) isCompressing = true;

        if (isCompressing && springCompressionProgress < activeSpring->getLength())
        {
            springCompressionProgress++;
            activeSpring->compress(springCompressionProgress);
            activeSpring->draw(); // Update visual display

            // Check if we're about to hit a wall - if so, release
            int wallCheckX = nextX + pos.diff_x;
            int wallCheckY = nextY + pos.diff_y;
            char wallChar = room->getCharAt(wallCheckX, wallCheckY);
            if (wallChar == 'W' || wallChar == '=')
            {
                releaseSpring();
            }
        }
    }
    // Check if player moved onto a NEW spring
    else if (obj != nullptr && obj->getType() == ObjectType::SPRING)
    {
        Spring* spring = dynamic_cast<Spring*>(obj);
        if (spring != nullptr && spring->occupiesPosition(nextX, nextY))
        {
            // Just stepped on spring
            beginSpringCompression(spring);
        }
    }
    // Player left the spring
    else if (activeSpring != nullptr)
    {
        activeSpring = nullptr;
        springCompressionProgress = 0;
    }

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

void Player::performAction(Action action)
{
    // Check if player is on spring and tries to change direction or stay
    if (activeSpring != nullptr)
    {
        Direction currentDir = getCurrentDirection();
        Direction newDir = actionToDirection(action);

        // Release conditions: STAY action or direction change attempt
        if (action == Action::STAY ||
            (newDir != Direction::STAY && newDir != currentDir))
        {
            releaseSpring();
            pos.setDirection(Direction::STAY);
            return;
        }
    }

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

//////////////////////////////////////////      Spring Mechanics       //////////////////////////////////////////

void Player::beginSpringCompression(Spring* spring)
{
    if (spring == nullptr)
        return;

    activeSpring = spring;
    springCompressionProgress = 1; // First step counts as compressing 1 char
    spring->compress(1);
    spring->draw(); // Update visual display
}

void Player::releaseSpring()
{
    if (activeSpring == nullptr)
        return;

    // Calculate launch parameters based on compression
    int compressedChars = springCompressionProgress;

    if (compressedChars > 0)
    {
        // Launch the player
        springMomentum = compressedChars;
        springFramesRemaining = compressedChars * compressedChars;
        springDirection = activeSpring->getProjectionDirection();
        inSpringMotion = true;

        // Visual: release spring to original state
        activeSpring->release();
        activeSpring->draw();
    }

    // Clear spring state
    activeSpring = nullptr;
    springCompressionProgress = 0;
}

bool Player::moveWithSpringMomentum(Room* room)
{
    if (springFramesRemaining <= 0)
    {
        // Spring motion complete
        inSpringMotion = false;
        springMomentum = 0;
        springFramesRemaining = 0;
        pos.diff_x = 0;
        pos.diff_y = 0;
        draw(room);
        return false;
    }

    // Calculate movement delta based on momentum and direction
    int dx = 0, dy = 0;

    switch (springDirection)
    {
    case Direction::UP:    dy = -springMomentum; break;
    case Direction::DOWN:  dy = springMomentum; break;
    case Direction::LEFT:  dx = -springMomentum; break;
    case Direction::RIGHT: dx = springMomentum; break;
    default: break;
    }

    // Try to move the full momentum distance
    int actualMoveX = 0, actualMoveY = 0;
    bool blocked = false;

    // Move step by step to check for obstacles
    for (int step = 1; step <= abs(dx) && !blocked; step++)
    {
        int testX = pos.x + (dx > 0 ? step : -step);
        int testY = pos.y;

        if (canMoveToPosition(testX, testY, room))
        {
            actualMoveX = (dx > 0 ? step : -step);
        }
        else
        {
            blocked = true;
        }
    }

    for (int step = 1; step <= abs(dy) && !blocked; step++)
    {
        int testX = pos.x + actualMoveX;
        int testY = pos.y + (dy > 0 ? step : -step);

        if (canMoveToPosition(testX, testY, room))
        {
            actualMoveY = (dy > 0 ? step : -step);
        }
        else
        {
            blocked = true;
        }
    }

    // Perform the move
    if (actualMoveX != 0 || actualMoveY != 0)
    {
        // Erase from current position
        gotoxy(pos.x, pos.y);
        std::cout << prevChar << std::flush;

        // Update position
        pos.x += actualMoveX;
        pos.y += actualMoveY;

        // Store new prevChar
        prevChar = room->getCharAt(pos.x, pos.y);
        if (prevChar == sprite)
            prevChar = ' ';

        draw(room);
    }

    if (blocked)
    {
        // Stop spring motion if we hit something
        inSpringMotion = false;
        springMomentum = 0;
        springFramesRemaining = 0;
        pos.diff_x = 0;
        pos.diff_y = 0;
    }
    else
    {
        springFramesRemaining--;
    }

    return true;
}

void Player::transferMomentum(Player* other)
{
    if (other == nullptr)
        return;

    // Transfer momentum to other player
    other->inSpringMotion = true;
    other->springMomentum = this->springMomentum;
    other->springDirection = this->springDirection;
    other->springFramesRemaining = this->springFramesRemaining;

    // Stop this player's motion
    this->inSpringMotion = false;
    this->springMomentum = 0;
    this->springFramesRemaining = 0;
}

bool Player::canMoveToPosition(int x, int y, Room* room)
{
    if (room == nullptr)
        return false;

    // Check bounds
    if (x < 0 || x >= MAX_X || y < 1 || y >= MAX_Y_INGAME - 1)
        return false;

    // Check walls
    char c = room->getCharAt(x, y);
    if (c == 'W' || c == '=')
        return false;

    // Check blocking objects
    GameObject* obj = room->getObjectAt(x, y);
    if (obj != nullptr && obj->isBlocking())
        return false;

    return true;
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
