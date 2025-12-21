//////////////////////////////////////       INCLUDES & FORWARDS       //////////////////////////////////////////

#include "Player.h"
#include "Room.h"
#include "Switch.h"
#include "Door.h"

//////////////////////////////////////////     Player Constructors     //////////////////////////////////////////

Player::Player()
    : inventory(nullptr), playerId(0), sprite(' '), prevChar(' '),
      atDoor(false), doorId(-1), alive(true), keyCount(0), waitingAtDoor(false)
{
    pos = Point(1, 1, 0, 0, ' ');
}

Player::Player(int id, int startX, int startY, char playerSprite)
    : inventory(nullptr), playerId(id), sprite(playerSprite), prevChar(' '),
      atDoor(false), doorId(-1), alive(true), keyCount(0), waitingAtDoor(false)
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
      waitingAtDoor(other.waitingAtDoor)
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
