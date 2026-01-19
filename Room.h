
#pragma once

//////////////////////////////////////////       INCLUDES & FORWARDS       /////////////////////////////////////////////

#include "Constants.h"
#include "GameObject.h"
#include "Screen.h"
#include <unordered_map>
#include <vector>

class Player;
class Spring;
class Obstacle;
class ObstacleBlock;

//////////////////////////////////////////        Modification       /////////////////////////////////////////////

struct Modification
{
  int x;
  int y;
  char newChar;

  Modification() : x(-1), y(-1), newChar(' ') {}
  Modification(int _x, int _y, char _c) : x(_x), y(_y), newChar(_c) {}
};

//////////////////////////////////////////         DarkZone       /////////////////////////////////////////////

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

//////////////////////////////////////////      ExplosionResult       /////////////////////////////////////////////

struct ExplosionResult
{
  bool keyDestroyed;
  bool player1Hit;
  bool player2Hit;
  int switchesDestroyed;
  int objectsDestroyed;

  ExplosionResult()
      : keyDestroyed(false), player1Hit(false), player2Hit(false),
        switchesDestroyed(0), objectsDestroyed(0) {}
};

//////////////////////////////////////////     DoorRequirements       /////////////////////////////////////////////
struct DoorRequirements
{
  int doorId;
  int requiredKeys;
  int requiredSwitches;
  int targetRoomId;
  bool isUnlocked;

  DoorRequirements()
      : doorId(-1), requiredKeys(1), requiredSwitches(0), targetRoomId(-1), isUnlocked(false) {}
  DoorRequirements(int id, int keys, int switches = 0, int target = -1)
      : doorId(id), requiredKeys(keys), requiredSwitches(switches), targetRoomId(target),
        isUnlocked(false) {}
};

//////////////////////////////////////////           Room       /////////////////////////////////////////////

// Represents a single game room
class Room
{
  struct WallCheckResult
  {
    bool valid;
    Direction projectionDirection;
    Point anchorPosition;

    WallCheckResult()
        : valid(false), projectionDirection(Direction::STAY),
          anchorPosition(-1, -1) {}
  };

  int roomId;
  Point legendTopLeft;
  bool active;
  bool completed;
  const Screen *baseLayout;
  std::vector<Modification> mods;
  std::vector<GameObject *> objects;
  std::vector<Spring *> springs;
  std::vector<Obstacle *> obstacles;
  int totalKeysInRoom;
  int keysCollected;
  int activeSwitches;
  int totalSwitches;
  std::vector<DoorRequirements> doorReqs;
  int nextRoomId;
  int prevRoomId;
  Point spawnPoint;
  Point spawnPointFromNext;
  std::vector<DarkZone> darkZones;
  bool visibilityMap[MAX_Y][MAX_X];

  void copyObjectsFrom(const Room &other);
  void deleteAllObjects();
  void initVisibility();

  char getCharAt(int x, int y) const;

  Point findSmartSpawn(Point base);

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


public:
  Room();
  explicit Room(int id);
  ~Room();
  Room(const Room &other);
  Room &operator=(const Room &other);

  // Getters
  int getRoomId() const { return roomId; }
  bool isActive() const { return active; }
  bool isCompleted() const { return completed; }
  const Screen* getBaseLayout() const { return baseLayout; }
  int getNextRoomId() const { return nextRoomId; }
  int getPrevRoomId() const { return prevRoomId; }
  int getTotalKeysInRoom() const { return totalKeysInRoom; }
  int getKeysCollected() const { return keysCollected; }
  const Point& getLegendTopLeft() const { return legendTopLeft; }

  // Door requirements access
  size_t getDoorReqsCount() const { return doorReqs.size(); }
  int getDoorTargetRoomId(int doorId) const {
    if (doorId >= 0 && doorId < static_cast<int>(doorReqs.size()))
      return doorReqs[doorId].targetRoomId;
    return -1;
  }
  int getDoorRequiredKeys(int doorId) const {
    if (doorId >= 0 && doorId < static_cast<int>(doorReqs.size()))
      return doorReqs[doorId].requiredKeys;
    return 0;
  }

  // Setters
  void setActive(bool value) { active = value; }
  void setSpawnPoint(const Point& p) { spawnPoint = p; }
  void setSpawnPointFromNext(const Point& p) { spawnPointFromNext = p; }
  void setNextRoomId(int id) { nextRoomId = id; }
  void setPrevRoomId(int id) { prevRoomId = id; }
  
  // Obstacle access (for iteration)
  const std::vector<Obstacle*>& getObstacles() const { return obstacles; }
  void resetAllObstaclePushStates();

  // Initialization
  void initFromLayout(const Screen *layout, const std::vector<int> *riddleIds = nullptr, int *riddleIndex = nullptr);
  void loadObjects(const std::vector<int> *riddleIds = nullptr, int *riddleIndex = nullptr);
  void setDoorRequirements(int doorId, int keys, int switches = 0, int targetRoomId = -1);
  void setLegendPoint(int x, int y) { legendTopLeft = Point(x, y); };

  // Drawing
  void draw();
  void drawDarkness(Player *p1 = nullptr, Player *p2 = nullptr);
  void drawVisibleObjects();

  // Legend
  void drawLegend(Player *p1, Player *p2);
  void drawEmptyLegend();
  void drawLegendInfo(Player *p1, Player *p2);
  void drawPlayerStats(Player *p);
  void DrawLives(Player *p);

  // Character access
  void setCharAt(int x, int y, char c);
  void resetMods();

  // Query methods
  ObjectType getObjectTypeAt(int x, int y) const;
  bool isWallAt(int x, int y) const;

  // Object management
  bool isVacantSpot(int x, int y);
  GameObject *getObjectAt(int x, int y);
  const GameObject *getObjectAt(int x, int y) const;
  bool addObject(GameObject *obj);
  void removeObject(int index);
  void removeObjectAt(int x, int y);
  std::vector<Door *> getDoors();
  std::vector<Switch *> getSwitches();
  ExplosionResult updateAllObjects(Player *p1, Player *p2);
  void addObstacle(Obstacle *obs);

  bool enoughSwitchesLeft() const { return activeSwitches >= totalSwitches; }
  int getTotalSwitches() const { return totalSwitches; }
  int getDoorReqSwitches(int doorId) const
  {
    if (doorId < 0 || doorId >= static_cast<int>(doorReqs.size()))
      return 0;
    return doorReqs[doorId].requiredSwitches;
  }
  
  Point getSpawnPoint(int playerId);
  Point getSpawnPointFromNext(int playerId);

  // Collision & movement
  bool isBlocked(int x, int y);
  bool hasLineOfSight(int x1, int y1, int x2, int y2);

  // Puzzle & door system
  void updatePuzzleState();
  int countActiveSwitches() const;
  bool canOpenDoor(int doorId, int player1Keys, int player2Keys) const;
  int getDoorIdAt(int x, int y) const;
  bool unlockDoor(int doorId);
  bool isDoorUnlocked(int doorId) const;

  // Dark zones
  void addDarkZone(int x1, int y1, int x2, int y2);
  void clearDarkZones();
  bool isInDarkZone(int x, int y) const;
  void updateVisibility(Player *p1, Player *p2);
  void lightRadius(int centerX, int centerY, int radius);
  bool isVisible(int x, int y) const;
};
