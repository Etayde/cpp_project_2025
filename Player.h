
#pragma once

//////////////////////////////////////////       INCLUDES & FORWARDS       //////////////////////////////////////////

#include "Console.h"
#include "Constants.h"
#include "Point.h"
#include "GameObject.h"

class Room;

//////////////////////////////////////////      PlayerKeyBinding       //////////////////////////////////////////

// Maps keyboard key to player action
struct PlayerKeyBinding
{
    char key;
    int playerID;
    Action action;
};

// Key bindings for both players
// Player 1: WAXDS for movement, E to drop
// Player 2: IJKLM for movement, O to drop
static const PlayerKeyBinding keyBindings[] = {
    {'w', 1, Action::MOVE_UP}, {'W', 1, Action::MOVE_UP}, {'x', 1, Action::MOVE_DOWN}, {'X', 1, Action::MOVE_DOWN}, {'a', 1, Action::MOVE_LEFT}, {'A', 1, Action::MOVE_LEFT}, {'d', 1, Action::MOVE_RIGHT}, {'D', 1, Action::MOVE_RIGHT}, {'s', 1, Action::STAY}, {'S', 1, Action::STAY}, {'e', 1, Action::DROP_ITEM}, {'E', 1, Action::DROP_ITEM},

    {'i', 2, Action::MOVE_UP},
    {'I', 2, Action::MOVE_UP},
    {'m', 2, Action::MOVE_DOWN},
    {'M', 2, Action::MOVE_DOWN},
    {'j', 2, Action::MOVE_LEFT},
    {'J', 2, Action::MOVE_LEFT},
    {'l', 2, Action::MOVE_RIGHT},
    {'L', 2, Action::MOVE_RIGHT},
    {'k', 2, Action::STAY},
    {'K', 2, Action::STAY},
    {'o', 2, Action::DROP_ITEM},
    {'O', 2, Action::DROP_ITEM}};

static const int NUM_KEY_BINDINGS = sizeof(keyBindings) / sizeof(keyBindings[0]);

//////////////////////////////////////////          Player            //////////////////////////////////////////

// Represents a player character
class Player
{
public:
    Point pos;
    GameObject *inventory; // Polymorphic pointer to held item
    int playerId;
    char sprite;
    char prevChar; // Character at position before player moved there
    bool atDoor;
    int doorId;
    bool alive;
    int keyCount; // Number of keys collected

public:
    // Constructors & Destructor
    Player();
    Player(int id, int startX, int startY, char playerSprite);
    ~Player();
    Player(const Player &other);
    Player &operator=(const Player &other);

    // Position getters
    int getX() const { return pos.x; }
    int getY() const { return pos.y; }
    Point getPosition() const { return pos; }
    char getSprite() const { return sprite; }

    // State getters
    bool isAtDoor() const { return atDoor; }
    int getDoorId() const { return doorId; }
    bool isAlive() const { return alive; }
    bool hasItem() const { return inventory != nullptr && inventory->isActive(); }
    bool hasTorch() const { return inventory != nullptr && inventory->getType() == ObjectType::TORCH; }
    bool hasKey() const { return inventory != nullptr && inventory->getType() == ObjectType::KEY; }
    bool hasBomb() const { return inventory != nullptr && inventory->getType() == ObjectType::BOMB; }
    int getKeyCount() const { return keyCount; }
    GameObject *getInventory() { return inventory; }
    const GameObject *getInventory() const { return inventory; }
    ObjectType getInventoryType() const { return inventory ? inventory->getType() : ObjectType::AIR; }

    // Setters
    void setPosition(int x, int y)
    {
        pos.x = x;
        pos.y = y;
    }
    void setDirection(Direction dir) { pos.setDirection(dir); }
    void kill() { alive = false; }
    void addKey() { keyCount++; }
    bool useKey();

    void draw()
    {
        gotoxy(pos.x, pos.y);
        std::cout << sprite << std::flush;
    }

    void erase()
    {
        gotoxy(pos.x, pos.y);
        std::cout << prevChar << std::flush;
    }

    void updateInventoryDisplay();

    // Movement & interaction
    bool move(Room *room);
    bool pickupItem(GameObject *item);
    Point dropItem(Room *room);
    void performAction(Action action);

private:
    void clearInventory();
    void copyInventoryFrom(const Player &other);
};
