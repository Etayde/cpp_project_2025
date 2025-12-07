#pragma once
#include "Screen.h"
#include "Object.h"
#include "Constants.h"

const int MAX_OBJECTS = 100;
const int MAX_MODS = 100;
const int MAX_DARK_ZONES = 10;
const int TORCH_RADIUS = 2;

// Forward declaration
class Player;

// Structure to track a single modification
struct Modification {
    int x;
    int y;
    char newChar;
    
    Modification() : x(-1), y(-1), newChar(' ') {}
    Modification(int _x, int _y, char _c) : x(_x), y(_y), newChar(_c) {}
};

// Structure to define a dark zone (rectangle)
struct DarkZone {
    int x1, y1;  // top-left corner
    int x2, y2;  // bottom-right corner
    
    DarkZone() : x1(-1), y1(-1), x2(-1), y2(-1) {}
    DarkZone(int _x1, int _y1, int _x2, int _y2) : x1(_x1), y1(_y1), x2(_x2), y2(_y2) {}
};

// Structure to track active bomb in a room
struct RoomBomb {
    int x;
    int y;
    int fuseTimer;  // Countdown in game ticks
    bool active;
    
    RoomBomb() : x(-1), y(-1), fuseTimer(0), active(false) {}
};

// Structure to track bomb explosion results
struct ExplosionResult {
    bool keyDestroyed;
    bool player1Hit;
    bool player2Hit;
    int objectsDestroyed;
    
    ExplosionResult() : keyDestroyed(false), player1Hit(false), player2Hit(false), objectsDestroyed(0) {}
};

class Room {
public:
    int roomId;
    bool active;                    // Is this room currently active?
    bool completed;                 // Has the room puzzle been solved?
    const Screen* baseLayout;       // מצביע ל-layout המקורי (לא משתנה)
    Modification mods[MAX_MODS];    // מערך השינויים
    int modCount;                   // כמה שינויים יש
    Object objects[MAX_OBJECTS];    // Objects in this room
    int objectCount;
    int requiredSwitches;           // How many switches need to be ON to complete
    int activeSwitches;             // Currently active switches
    int nextRoomId;                 // Which room to go to when door is opened (forward)
    int prevRoomId;                 // Which room to go back to (-1 if first room)
    bool doorToNextUnlocked;        // Is the door to next room unlocked?
    bool doorToPrevUnlocked;        // Is the door to previous room unlocked? (always true if came from there)
    Point spawnPoint;               // Where players spawn in this room
    Point spawnPointFromNext;       // Where players spawn when coming back from next room
    
    // Bomb state for this room
    RoomBomb bomb;
    
    // Dark zones system
    DarkZone darkZones[MAX_DARK_ZONES];
    int darkZoneCount;
    bool visibilityMap[MAX_Y][MAX_X];  // true = visible, false = dark

    // Constructor
    Room() : roomId(-1), active(false), completed(false), baseLayout(nullptr),
             modCount(0), objectCount(0), requiredSwitches(0), activeSwitches(0), 
             nextRoomId(-1), prevRoomId(-1), doorToNextUnlocked(false), doorToPrevUnlocked(false),
             darkZoneCount(0) {
        // Initialize visibility to all visible
        for (int y = 0; y < MAX_Y; y++)
            for (int x = 0; x < MAX_X; x++)
                visibilityMap[y][x] = true;
    }

    Room(int id) : roomId(id), active(false), completed(false), baseLayout(nullptr),
                   modCount(0), objectCount(0), requiredSwitches(0), activeSwitches(0), 
                   nextRoomId(-1), prevRoomId(-1), doorToNextUnlocked(false), doorToPrevUnlocked(false),
                   darkZoneCount(0) {
        for (int y = 0; y < MAX_Y; y++)
            for (int x = 0; x < MAX_X; x++)
                visibilityMap[y][x] = true;
    }

    // Initialize room from a screen layout
    void initFromLayout(const Screen* layout);
    
    // Load objects from the layout
    void loadObjects();
    
    // Draw the room (base + mods + darkness overlay)
    void draw();
    
    // Draw only the darkness overlay (for updates)
    void drawDarkness();
    
    // Update a single cell - adds to mods array
    void setCharAt(int x, int y, char c);
    
    // Get character at position - checks mods first, then base
    char getCharAt(int x, int y) const;
    
    // Reset all modifications
    void resetMods();
    
    // Dark zones management
    void addDarkZone(int x1, int y1, int x2, int y2);
    void clearDarkZones();
    bool isInDarkZone(int x, int y) const;
    
    // Visibility system
    void updateVisibility(Player* p1, Player* p2);
    void lightRadius(int centerX, int centerY, int radius);
    bool isVisible(int x, int y) const;
    
    // Get object at position (returns nullptr if none)
    Object* getObjectAt(int x, int y);
    
    // Add an object to the room
    bool addObject(const Object& obj);
    
    // Remove an object from the room
    void removeObject(int index);
    
    // Remove object at position
    void removeObjectAt(int x, int y);
    
    // Check if position is blocked
    bool isBlocked(int x, int y);
    
    // Check and update puzzle state
    void updatePuzzleState();
    
    // Check if door is unlocked (via switches)
    bool isDoorUnlocked() const;
    
    // Get door ID at position (returns -1 if no door)
    int getDoorIdAt(int x, int y) const;
    
    // Explode a bomb at given position with given radius
    ExplosionResult explodeBomb(int centerX, int centerY, int radius, Player* p1, Player* p2);
    
    // Check if there's a clear line of sight (not blocked by walls/doors)
    bool hasLineOfSight(int x1, int y1, int x2, int y2);
};
