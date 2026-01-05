
#pragma once

//////////////////////////////////////////       INCLUDES & FORWARDS
/////////////////////////////////////////////

#include "Constants.h"
#include "GameObject.h"
#include "Screen.h"
#include <unordered_map>
#include <vector>

class Player;
class Spring;
class Obstacle;
class ObstacleBlock;

//////////////////////////////////////////        Modification
/////////////////////////////////////////////

struct Modification {
  int x;
  int y;
  char newChar;

  Modification() : x(-1), y(-1), newChar(' ') {}
  Modification(int _x, int _y, char _c) : x(_x), y(_y), newChar(_c) {}
};

//////////////////////////////////////////         DarkZone
/////////////////////////////////////////////

struct DarkZone {
  int x1, y1; // Top-left corner
  int x2, y2; // Bottom-right corner

  DarkZone() : x1(-1), y1(-1), x2(-1), y2(-1) {}
  DarkZone(int _x1, int _y1, int _x2, int _y2)
      : x1(_x1), y1(_y1), x2(_x2), y2(_y2) {}

  bool contains(int x, int y) const {
    return x >= x1 && x <= x2 && y >= y1 && y <= y2;
  }
};

//////////////////////////////////////////      ExplosionResult
/////////////////////////////////////////////

struct ExplosionResult {
  bool keyDestroyed;
  bool player1Hit;
  bool player2Hit;
  int objectsDestroyed;

  ExplosionResult()
      : keyDestroyed(false), player1Hit(false), player2Hit(false),
        objectsDestroyed(0) {}
};

//////////////////////////////////////////     DoorRequirements
/////////////////////////////////////////////

// Requirements to open a specific door
struct DoorRequirements {
  int doorId;
  int requiredKeys;
  int requiredSwitches;
  bool isUnlocked;

  DoorRequirements()
      : doorId(-1), requiredKeys(1), requiredSwitches(0), isUnlocked(false) {}
  DoorRequirements(int id, int keys, int switches = 0)
      : doorId(id), requiredKeys(keys), requiredSwitches(switches),
        isUnlocked(false) {}
};

//////////////////////////////////////////           Room
/////////////////////////////////////////////

// Represents a single game room/level
class Room {
public:
  // Room identity
  int roomId;
  Point legendTopLeft;
  bool active;
  bool completed;

  // Layout
  const Screen *baseLayout;
  std::vector<Modification> mods;

  // Objects
  std::vector<GameObject *> objects;
  std::vector<Spring *> springs;     // Spring managers (not GameObjects)
  std::vector<Obstacle *> obstacles; // Obstacles in the room

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
  void initFromLayout(const Screen *layout, int *riddleCounter = nullptr);
  void loadObjects(int *riddleCounter = nullptr);
  void setDoorRequirements(int doorId, int keys, int switches = 0);
  void setLegendPoint(int x, int y) { legendTopLeft = Point(x, y); };

  // Drawing
  void draw();
  void drawDarkness();
  void drawVisibleObjects();

  // Legend
  void drawLegend(Player *p1, Player *p2);
  void drawEmptyLegend();
  void drawLegendInfo(Player *p1, Player *p2);
  void drawPlayerStats(Player* p);
  void DrawLives(Player* p);

  // Character access (encapsulated - use query methods below)
  void setCharAt(int x, int y, char c);
  void resetMods();

  // Query methods
  ObjectType getObjectTypeAt(int x, int y) const;
  bool isWallAt(int x, int y) const;

  // Object management
  GameObject *getObjectAt(int x, int y);
  const GameObject *getObjectAt(int x, int y) const;
  bool addObject(GameObject *obj);
  void removeObject(int index);
  void removeObjectAt(int x, int y);
  std::vector<Door *> getDoors();
  std::vector<Switch *> getSwitches();
  ExplosionResult updateAllObjects(Player *p1, Player *p2);
  void addObstacle(Obstacle *obs);

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

private:
  void copyObjectsFrom(const Room &other);
  void deleteAllObjects();
  void initVisibility();

  // Low-level character access (private - use public query methods instead)
  char getCharAt(int x, int y) const;

  // Spring creation helpers
  struct WallCheckResult {
    bool valid;
    Direction projectionDirection;
    Point anchorPosition;

    WallCheckResult()
        : valid(false), projectionDirection(Direction::STAY),
          anchorPosition(-1, -1) {}
  };

  Direction detectOrientation(const std::vector<Point> &positions);
  std::vector<Point> sortPositions(const std::vector<Point> &positions,
                                   Direction orientation);
  WallCheckResult checkWallAdjacency(const std::vector<Point> &sorted,
                                     Direction orientation);
  void scanAndCreateSprings();
  void createMultiCellObject(const std::vector<Point> &allObjCells);
  void createSpringFromGroup(const std::vector<Point> &group);
  void createObstacleFromGroup(
      const std::vector<Point> &group,
      std::unordered_map<Point, std::vector<Point>> &neighbors);
};
