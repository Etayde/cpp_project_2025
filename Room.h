
#pragma once

//////////////////////////////////////////       INCLUDES & FORWARDS       //////////////////////////////////////////

#include "Screen.h"
#include "GameObject.h"
#include "Constants.h"
#include <vector>

class Player;

//////////////////////////////////////////        Modification        //////////////////////////////////////////

struct Modification
{
    int x;
    int y;
    char newChar;

    Modification() : x(-1), y(-1), newChar(' ') {}
    Modification(int _x, int _y, char _c) : x(_x), y(_y), newChar(_c) {}
};

//////////////////////////////////////////         DarkZone           //////////////////////////////////////////

struct DarkZone
{
    int x1, y1; // Top-left corner
    int x2, y2; // Bottom-right corner

    DarkZone() : x1(-1), y1(-1), x2(-1), y2(-1) {}
    DarkZone(int _x1, int _y1, int _x2, int _y2)
        : x1(_x1), y1(_y1), x2(_x2), y2(_y2) {}

    bool contains(int x, int y) const
    {
        return x >= x1 && x <= x2 && y >= y1 && y <= y2;
    }
};

//////////////////////////////////////////         RoomBomb           //////////////////////////////////////////

// Tracks an active bomb in the room
struct RoomBomb
{
    int x;
    int y;
    int fuseTimer;
    bool active;

    RoomBomb() : x(-1), y(-1), fuseTimer(0), active(false) {}

    void reset()
    {
        x = -1;
        y = -1;
        fuseTimer = 0;
        active = false;
    }

    void decrementFuse()
    {
        if (fuseTimer > 0)
            fuseTimer--;
    }

    bool isReadyToExplode() const
    {
        return active && fuseTimer <= 0;
    }
};

//////////////////////////////////////////      ExplosionResult       //////////////////////////////////////////

struct ExplosionResult
{
    bool keyDestroyed;
    bool player1Hit;
    bool player2Hit;
    int objectsDestroyed;

    ExplosionResult()
        : keyDestroyed(false), player1Hit(false), player2Hit(false), objectsDestroyed(0) {}
};

//////////////////////////////////////////     DoorRequirements       //////////////////////////////////////////

// Requirements to open a specific door
struct DoorRequirements
{
    int doorId;
    int requiredKeys;
    int requiredSwitches;
    bool isUnlocked;

    DoorRequirements()
        : doorId(-1), requiredKeys(1), requiredSwitches(0), isUnlocked(false) {}
    DoorRequirements(int id, int keys, int switches = 0)
        : doorId(id), requiredKeys(keys), requiredSwitches(switches), isUnlocked(false) {}
};

//////////////////////////////////////////           Room             //////////////////////////////////////////

// Represents a single game room/level
class Room
{
public:
    // Room identity
    int roomId;
    bool active;
    bool completed;

    // Layout
    const Screen *baseLayout;
    std::vector<Modification> mods;

    // Objects
    std::vector<GameObject*> objects;

    // Key counter system
    int totalKeysInRoom;
    int keysCollected;
    int activeSwitches;
    int totalSwitches;

    // Door requirements
    std::vector<DoorRequirements> doorReqs;

    // Room navigation
    int nextRoomId;
    int prevRoomId;
    Point spawnPoint;
    Point spawnPointFromNext;

    // Bomb system
    RoomBomb bomb;

    // Dark zones
    std::vector<DarkZone> darkZones;
    bool visibilityMap[MAX_Y][MAX_X];

public:
    // Constructors & Destructor
    Room();
    explicit Room(int id);
    ~Room();
    Room(const Room &other);
    Room &operator=(const Room &other);

    // Initialization
    void initFromLayout(const Screen *layout);
    void loadObjects();
    void setDoorRequirements(int doorId, int keys, int switches = 0);

    // Drawing
    void draw();
    void drawDarkness();

    // Character access
    char getCharAt(int x, int y) const;
    void setCharAt(int x, int y, char c);
    void resetMods();

    // Object management
    GameObject *getObjectAt(int x, int y);
    const GameObject *getObjectAt(int x, int y) const;
    bool addObject(GameObject *obj);
    void removeObject(int index);
    void removeObjectAt(int x, int y);
    std::vector<Door *> getDoors();
    std::vector<Switch *> getSwitches();
    bool updateBomb(Player *p1, Player *p2);
    void handleBombDrop(Player &player);

    // Collision & movement
    bool isBlocked(int x, int y);
    bool hasLineOfSight(int x1, int y1, int x2, int y2);

    // Puzzle & door system
    void updatePuzzleState();
    int countActiveSwitches() const;
    bool canOpenDoor(int doorId, int player1Keys, int player2Keys) const;
    int getDoorIdAt(int x, int y) const;
    void unlockDoor(int doorId);
    bool isDoorUnlocked(int doorId) const;

    // Dark zones
    void addDarkZone(int x1, int y1, int x2, int y2);
    void clearDarkZones();
    bool isInDarkZone(int x, int y) const;
    void updateVisibility(Player *p1, Player *p2);
    void lightRadius(int centerX, int centerY, int radius);
    bool isVisible(int x, int y) const;

    // Bomb system
    ExplosionResult explodeBomb(int centerX, int centerY, int radius,
                                Player *p1, Player *p2);

private:
    void copyObjectsFrom(const Room &other);
    void deleteAllObjects();
    void initVisibility();
};
