#pragma once
#include "Console.h"
#include "Constants.h"
#include "Point.h"
#include "Object.h"

// Forward declarations
class Room;

// Key binding structure
struct PlayerKeyBinding {
    char key;
    int playerID;
    Action action;
};

// Key bindings for both players
static const PlayerKeyBinding keyBindings[] = {
    {'w', 1, Action::MOVE_UP},
    {'W', 1, Action::MOVE_UP},
    {'x', 1, Action::MOVE_DOWN},
    {'X', 1, Action::MOVE_DOWN},
    {'a', 1, Action::MOVE_LEFT},
    {'A', 1, Action::MOVE_LEFT},
    {'d', 1, Action::MOVE_RIGHT},
    {'D', 1, Action::MOVE_RIGHT},
    {'s', 1, Action::STAY},
    {'S', 1, Action::STAY},
    {'e', 1, Action::DROP_ITEM},
    {'E', 1, Action::DROP_ITEM},
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
    {'O', 2, Action::DROP_ITEM}
};

static const int NUM_KEY_BINDINGS = sizeof(keyBindings) / sizeof(keyBindings[0]);

class Player {
public:
    Point pos;
    Object inventory;
    int playerId;
    char sprite;
    char prevChar;  // Character that was at the position before player moved there
    bool atDoor;    // Is player currently standing on a door?
    int doorId;     // Which door (-1 if not at door)
    bool alive;     // Is player alive?

    // Constructor
    Player() : playerId(0), sprite(' '), prevChar(' '), atDoor(false), doorId(-1), alive(true) {}

    Player(int id, int startX, int startY, char playerSprite)
        : playerId(id), sprite(playerSprite), prevChar(' '), atDoor(false), doorId(-1), alive(true) {
        pos = Point(startX, startY, 0, 0, playerSprite);
        inventory = Object();  // Empty inventory (AIR)
    }

    // Getters
    int getX() const { return pos.x; }
    int getY() const { return pos.y; }
    Point getPosition() const { return pos; }
    char getSprite() const { return sprite; }
    Object* getInventory() { return &inventory; }
    bool hasItem() const { return inventory.type != ObjectType::AIR; }
    bool isAtDoor() const { return atDoor; }
    int getDoorId() const { return doorId; }
    bool isAlive() const { return alive; }
    bool hasTorch() const { return inventory.type == ObjectType::TORCH; }

    // Setters
    void setPosition(int x, int y) {
        pos.x = x;
        pos.y = y;
    }

    void setDirection(Direction dir) {
        pos.setDirection(dir);
    }
    
    void kill() { alive = false; }

    // Draw player at current position
    void draw() {
        gotoxy(pos.x, pos.y);
        std::cout << sprite;
        std::cout.flush();
    }

    // Erase player (restore previous character)
    void erase() {
        gotoxy(pos.x, pos.y);
        std::cout << prevChar;
        std::cout.flush();
    }

    // Move player in current direction, checking for collisions
    bool move(Room* room);

    // Pick up an item
    bool pickupItem(Object* item);

    // Drop current item (returns the dropped item position, or {-1,-1} if failed)
    Point dropItem(Room* room);

    // Update inventory display
    void updateInventoryDisplay();

    // Perform action based on input
    void performAction(Action action);
    
    // Check if player has a key
    bool hasKey() const { return inventory.type == ObjectType::KEY; }
    
    // Use the key (consume it)
    void useKey() {
        if (hasKey()) {
            inventory = Object();
            updateInventoryDisplay();
        }
    }
};
