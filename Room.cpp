
//////////////////////////////////////       INCLUDES & FORWARDS       /////////////////////////////////////////////

#include "Room.h"
#include "Bomb.h"
#include "Console.h"
#include "Constants.h"

#include "Door.h"
#include "GameObject.h"
#include "Layouts.h"

#include "Items.h"
#include "Obstacle.h"
#include "Player.h"
#include "Spring.h"
#include "SpringLink.h"
#include "StaticObjects.h"
#include "Switch.h"
#include <cmath>
#include <unordered_map>
#include <vector>

//////////////////////////////////////////      Room Constructors       /////////////////////////////////////////////

Room::Room()
    : roomId(-1), legendTopLeft(), active(false), completed(false),
      baseLayout(nullptr), totalKeysInRoom(0), keysCollected(0),
      activeSwitches(0), totalSwitches(0), nextRoomId(-1), prevRoomId(-1)
{
  initVisibility();
}

Room::Room(int id)
    : roomId(id), legendTopLeft(), active(false), completed(false),
      baseLayout(nullptr), totalKeysInRoom(0), keysCollected(0),
      activeSwitches(0), totalSwitches(0), nextRoomId(-1), prevRoomId(-1)
{
  initVisibility();
}

//////////////////////////////////////////       Room Destructor       /////////////////////////////////////////////

Room::~Room()
{
  deleteAllObjects();

  for (Spring *spring : springs)
  {
    delete spring;
  }
  springs.clear();

  for (Obstacle *obstacle : obstacles)
  {
    delete obstacle;
  }
  obstacles.clear();
}

//////////////////////////////////////////      Room Copy Constructor       /////////////////////////////////////////////

Room::Room(const Room &other)
    : roomId(other.roomId), legendTopLeft(other.legendTopLeft),
      active(other.active), completed(other.completed),
      baseLayout(other.baseLayout), mods(other.mods),
      totalKeysInRoom(other.totalKeysInRoom),
      keysCollected(other.keysCollected), activeSwitches(other.activeSwitches),
      totalSwitches(other.totalSwitches), doorReqs(other.doorReqs),
      nextRoomId(other.nextRoomId), prevRoomId(other.prevRoomId),
      spawnPoint(other.spawnPoint),
      spawnPointFromNext(other.spawnPointFromNext), darkZones(other.darkZones)
{
  for (int y = 0; y < MAX_Y; y++)
    for (int x = 0; x < MAX_X; x++)
      visibilityMap[y][x] = other.visibilityMap[y][x];

  copyObjectsFrom(other);
}

//////////////////////////////////////////    Room Assignment Operator       /////////////////////////////////////////////

Room &Room::operator=(const Room &other)
{
  if (this != &other)
  {
    deleteAllObjects();

    for (Spring *spring : springs)
    {
      delete spring;
    }
    springs.clear();

    roomId = other.roomId;
    legendTopLeft = other.legendTopLeft;
    active = other.active;
    completed = other.completed;
    baseLayout = other.baseLayout;
    mods = other.mods;
    totalKeysInRoom = other.totalKeysInRoom;
    keysCollected = other.keysCollected;
    activeSwitches = other.activeSwitches;
    totalSwitches = other.totalSwitches;
    nextRoomId = other.nextRoomId;
    prevRoomId = other.prevRoomId;
    spawnPoint = other.spawnPoint;
    spawnPointFromNext = other.spawnPointFromNext;
    darkZones = other.darkZones;
    doorReqs = other.doorReqs;

    for (int y = 0; y < MAX_Y; y++)
      for (int x = 0; x < MAX_X; x++)
        visibilityMap[y][x] = other.visibilityMap[y][x];

    copyObjectsFrom(other);
  }
  return *this;
}

//////////////////////////////////////////       Private Helpers       /////////////////////////////////////////////

void Room::copyObjectsFrom(const Room &other)
{
  objects.clear();
  for (GameObject *obj : other.objects)
  {
    if (obj != nullptr)
    {
      objects.push_back(obj->clone());
    }
  }

  springs.clear();
  std::unordered_map<Spring *, Spring *> springMap;

  for (Spring *oldSpring : other.springs)
  {
    if (oldSpring != nullptr)
    {
      Spring *newSpring = oldSpring->clone();
      springs.push_back(newSpring);
      springMap[oldSpring] = newSpring;
    }
  }

  for (GameObject *obj : objects)
  {
    if (obj != nullptr && obj->getType() == ObjectType::SPRING_LINK)
    {
      SpringLink *link = static_cast<SpringLink *>(obj);
      Spring *oldParent = link->getParentSpring();

      if (oldParent != nullptr &&
          springMap.find(oldParent) != springMap.end())
      {
        link->setParentSpring(springMap[oldParent]);
      }
    }
  }
}

void Room::deleteAllObjects()
{
  for (GameObject *obj : objects)
  {
    delete obj;
  }
  objects.clear();
}

void Room::initVisibility()
{
  for (int y = 0; y < MAX_Y; y++)
    for (int x = 0; x < MAX_X; x++)
      visibilityMap[y][x] = true;
}

//////////////////////////////////////////       initFromLayout       /////////////////////////////////////////////

void Room::initFromLayout(const Screen *layout, int *riddleCounter)
{
  baseLayout = layout;
  mods.clear();
  deleteAllObjects();
  loadObjects(riddleCounter);
}

//////////////////////////////////////////        loadObjects       /////////////////////////////////////////////

void Room::loadObjects(int *riddleCounter)
{
  totalKeysInRoom = 0;
  keysCollected = 0;
  activeSwitches = 0;
  totalSwitches = 0;
  std::vector<Point> springPositions;
  std::vector<Point> obstaclePositions;

  if (baseLayout == nullptr)
    return;

  for (int y = 0; y < MAX_Y_INGAME; y++)
  {
    for (int x = 0; x < MAX_X; x++)
    {
      char ch = baseLayout->getCharAt(x, y);

      if (ch == '#')
      {
        springPositions.push_back(Point(x, y));
        continue;
      }
      if (ch == '*')
      {
        obstaclePositions.push_back(Point(x, y));
        continue;
      }

      int riddleId = -1;
      if (ch == '?')
      {
        if (riddleCounter != nullptr)
        {
          riddleId = (*riddleCounter)++;
        }
        else
          continue;
      }

      GameObject *obj = createObjectFromChar(ch, x, y, riddleId);

      if (obj != nullptr)
      {
        if (obj->getType() == ObjectType::KEY)
          totalKeysInRoom++;
        else if (obj->getType() == ObjectType::SWITCH_OFF)
          totalSwitches++;
        else if (obj->getType() == ObjectType::SWITCH_ON)
        {
          totalSwitches++;
          activeSwitches++;
        }

        if (!addObject(obj))
          delete obj;
      }
    }
  }

  createMultiCellObject(springPositions);
  createMultiCellObject(obstaclePositions);
}

//////////////////////////////////////////    setDoorRequirements       /////////////////////////////////////////////

void Room::setDoorRequirements(int doorId, int keys, int switches)
{
  if (doorId < 0)
    return;

  if (doorId >= static_cast<int>(doorReqs.size()))
  {
    doorReqs.resize(doorId + 1);
  }

  doorReqs[doorId].doorId = doorId;
  doorReqs[doorId].requiredKeys = keys;
  doorReqs[doorId].requiredSwitches = switches;
  doorReqs[doorId].isUnlocked = false;
}

//////////////////////////////////////////           draw       /////////////////////////////////////////////

void Room::draw()
{
  if (baseLayout != nullptr)
    baseLayout->draw();

  for (const Modification &mod : mods)
  {
    gotoxy(mod.x, mod.y);
    std::cout << mod.newChar;
  }

  drawDarkness();
}

//////////////////////////////////////////        drawDarkness       /////////////////////////////////////////////

void Room::drawDarkness()
{
  if (darkZones.empty())
    return;

  for (int y = 0; y < MAX_Y_INGAME; y++)
  {
    for (int x = 0; x < MAX_X; x++)
    {
      if (!isInDarkZone(x, y))
        continue;

      gotoxy(x, y);
      if (visibilityMap[y][x])
        std::cout << getCharAt(x, y);
      else
        std::cout << ' ';
    }
  }
}

//////////////////////////////////////////     drawVisibleObjects       /////////////////////////////////////////////

void Room::drawVisibleObjects()
{
  for (GameObject *obj : objects)
  {
    if (!obj || !obj->isActive())
      continue;

    int x = obj->getX();
    int y = obj->getY();

    if (isInDarkZone(x, y) && !visibilityMap[y][x] && !obj->isAlwaysVisible())
    {
      gotoxy(x, y);
      std::cout << ' ';
      continue;
    }

    obj->draw();
  }
}

//////////////////////////////////////////         getCharAt       /////////////////////////////////////////////

char Room::getCharAt(int x, int y) const
{
  for (const Modification &mod : mods)
  {
    if (mod.x == x && mod.y == y)
      return mod.newChar;
  }

  if (baseLayout != nullptr)
    return baseLayout->getCharAt(x, y);
  return 'W';
}

//////////////////////////////////////////         setCharAt       /////////////////////////////////////////////

void Room::setCharAt(int x, int y, char c)
{
  for (Modification &mod : mods)
  {
    if (mod.x == x && mod.y == y)
    {
      mod.newChar = c;
      return;
    }
  }

  mods.push_back(Modification(x, y, c));
}

//////////////////////////////////////////         resetMods       /////////////////////////////////////////////

void Room::resetMods() { mods.clear(); }

//////////////////////////////////////////     getObjectTypeAt       /////////////////////////////////////////////

ObjectType Room::getObjectTypeAt(int x, int y) const
{
  return static_cast<ObjectType>(getCharAt(x, y));
}

//////////////////////////////////////////        isWallAt       /////////////////////////////////////////////

bool Room::isWallAt(int x, int y) const
{
  if (legendTopLeft.x >= 0 && legendTopLeft.y >= 0)
  {
    int legX = legendTopLeft.x - 1;
    int legY = legendTopLeft.y - 1;
    if (x >= legX && x < legX + 22 && y >= legY && y < legY + 5)
    {
      return true;
    }
  }
  char c = getCharAt(x, y);
  return (BlockingChars::isBlockingChar(c));
}

//////////////////////////////////////////        getObjectAt       /////////////////////////////////////////////

GameObject *Room::getObjectAt(int x, int y)
{
  for (GameObject *obj : objects)
  {
    if (obj != nullptr && obj->isActive())
    {
      if (obj->getX() == x && obj->getY() == y)
      {
        return obj;
      }
    }
  }
  return nullptr;
}

const GameObject *Room::getObjectAt(int x, int y) const
{
  for (const GameObject *obj : objects)
  {
    if (obj != nullptr && obj->isActive())
    {
      if (obj->getX() == x && obj->getY() == y)
      {
        return obj;
      }
    }
  }
  return nullptr;
}

//////////////////////////////////////////         addObject       /////////////////////////////////////////////

bool Room::addObject(GameObject *obj)
{
  if (obj == nullptr)
    return false;

  objects.push_back(obj);
  setCharAt(obj->getX(), obj->getY(), obj->getSprite());

  return true;
}

//////////////////////////////////////////        removeObject       /////////////////////////////////////////////

void Room::removeObject(int index)
{
  if (index < 0 || index >= static_cast<int>(objects.size()))
    return;

  GameObject *obj = objects[index];
  if (obj != nullptr)
  {
    setCharAt(obj->getX(), obj->getY(), ' ');
    delete obj;
  }
  objects.erase(objects.begin() + index);
}

//////////////////////////////////////////       removeObjectAt       /////////////////////////////////////////////

void Room::removeObjectAt(int x, int y)
{
  for (size_t i = 0; i < objects.size(); i++)
  {
    if (objects[i] != nullptr && objects[i]->getX() == x &&
        objects[i]->getY() == y)
    {
      removeObject(i);
      return;
    }
  }
}

//////////////////////////////////////////        addObstacle       /////////////////////////////////////////////

void Room::addObstacle(Obstacle *obs)
{
  if (obs != nullptr)
  {
    obstacles.push_back(obs);
  }
}

//////////////////////////////////////////          getDoors       /////////////////////////////////////////////

std::vector<Door *> Room::getDoors()
{
  std::vector<Door *> doors;
  for (GameObject *obj : objects)
  {
    if (obj != nullptr && obj->getType() == ObjectType::DOOR)
    {
      doors.push_back(static_cast<Door *>(obj));
    }
  }
  return doors;
}

//////////////////////////////////////////        getSwitches       /////////////////////////////////////////////

std::vector<Switch *> Room::getSwitches()
{
  std::vector<Switch *> switches;
  for (GameObject *obj : objects)
  {
    if (obj != nullptr && (obj->getType() == ObjectType::SWITCH_ON ||
                           obj->getType() == ObjectType::SWITCH_OFF))
    {
      switches.push_back(static_cast<Switch *>(obj));
    }
  }
  return switches;
}

//////////////////////////////////////////         isBlocked       /////////////////////////////////////////////

bool Room::isBlocked(int x, int y)
{
  if (isWallAt(x, y))
    return true;

  GameObject *obj = getObjectAt(x, y);
  if (obj != nullptr)
  {
    if (obj->isBlocking())
      return true;
    if (obj->isPickable())
      return false;
  }

  return false;
}

//////////////////////////////////////////      hasLineOfSight       /////////////////////////////////////////////

// Check line of sight using Bresenham's algorithm - MADE WITH AI
bool Room::hasLineOfSight(int x1, int y1, int x2, int y2)
{
  int dx = abs(x2 - x1);
  int dy = abs(y2 - y1);
  int sx = (x1 < x2) ? 1 : -1;
  int sy = (y1 < y2) ? 1 : -1;
  int err = dx - dy;

  int x = x1, y = y1;

  while (x != x2 || y != y2)
  {
    if (x != x1 || y != y1)
    {
      if (isWallAt(x, y))
        return false;

      char c = getCharAt(x, y);
      if (c >= '0' && c <= '9')
        return false;
    }

    int e2 = 2 * err;
    if (e2 > -dy)
    {
      err -= dy;
      x += sx;
    }
    if (e2 < dx)
    {
      err += dx;
      y += sy;
    }
  }

  return true;
}

//////////////////////////////////////////     updatePuzzleState       /////////////////////////////////////////////

void Room::updatePuzzleState()
{
  activeSwitches = countActiveSwitches();

  if (activeSwitches >= totalSwitches && totalSwitches > 0 && !completed)
  {
    completed = true;

    for (GameObject *obj : objects)
    {
      if (obj != nullptr && obj->getType() == ObjectType::SWITCH_WALL)
      {
        SwitchWall *sww = dynamic_cast<SwitchWall *>(obj);
        if (sww != nullptr && sww->isRemovedBySwitch())
        {
          setCharAt(sww->getX(), sww->getY(), ' ');
          gotoxy(sww->getX(), sww->getY());
          std::cout << ' ';
          sww->setActive(false);
        }
      }
    }
  }
}

//////////////////////////////////////////    countActiveSwitches       /////////////////////////////////////////////

int Room::countActiveSwitches() const
{
  int count = 0;
  for (const GameObject *obj : objects)
  {
    if (obj != nullptr && obj->getType() == ObjectType::SWITCH_ON)
      count++;
  }
  return count;
}

//////////////////////////////////////////        canOpenDoor       /////////////////////////////////////////////

bool Room::canOpenDoor(int doorId, int player1Keys, int player2Keys) const
{
  if (doorId < 0 || doorId >= static_cast<int>(doorReqs.size()))
    return false;

  const DoorRequirements &req = doorReqs[doorId];

  if (req.isUnlocked)
    return true;
  if (req.requiredSwitches > 0 && activeSwitches < req.requiredSwitches)
    return false;

  int totalKeys = player1Keys + player2Keys;
  if (req.requiredKeys > 0 && totalKeys < req.requiredKeys)
    return false;

  return true;
}

//////////////////////////////////////////        getDoorIdAt       /////////////////////////////////////////////

int Room::getDoorIdAt(int x, int y) const
{
  char c = getCharAt(x, y);
  if (c >= '0' && c <= '9')
    return c - '0';
  return -1;
}

//////////////////////////////////////////        unlockDoor       /////////////////////////////////////////////

bool Room::unlockDoor(int doorId)
{
  if (doorId >= 0 && doorId < static_cast<int>(doorReqs.size()))
  {
    doorReqs[doorId].isUnlocked = true;
    return true;
  }
  return false;
}

//////////////////////////////////////////      isDoorUnlocked       /////////////////////////////////////////////

bool Room::isDoorUnlocked(int doorId) const
{
  if (doorId >= 0 && doorId < static_cast<int>(doorReqs.size()))
    return doorReqs[doorId].isUnlocked;
  return false;
}

//////////////////////////////////////////        addDarkZone       /////////////////////////////////////////////

void Room::addDarkZone(int x1, int y1, int x2, int y2)
{
  darkZones.push_back(DarkZone(x1, y1, x2, y2));
}

//////////////////////////////////////////       clearDarkZones       /////////////////////////////////////////////

void Room::clearDarkZones()
{
  darkZones.clear();
  initVisibility();
}

//////////////////////////////////////////       isInDarkZone       /////////////////////////////////////////////

bool Room::isInDarkZone(int x, int y) const
{
  for (const DarkZone &zone : darkZones)
  {
    if (zone.contains(x, y))
      return true;
  }
  return false;
}

//////////////////////////////////////////      updateVisibility       /////////////////////////////////////////////

void Room::updateVisibility(Player *p1, Player *p2)
{
  if (darkZones.empty())
    return;

  initVisibility();

  for (const DarkZone &zone : darkZones)
  {
    for (int y = zone.y1; y <= zone.y2; y++)
    {
      for (int x = zone.x1; x <= zone.x2; x++)
      {
        if (x >= 0 && x < MAX_X && y >= 0 && y < MAX_Y)
          visibilityMap[y][x] = false;
      }
    }
  }

  if (p1 != nullptr && p1->hasTorch())
  {
    Torch *torch = static_cast<Torch *>(p1->getInventory());
    torch->illuminate(this, p1->getX(), p1->getY());
  }
  if (p2 != nullptr && p2->hasTorch())
  {
    Torch *torch = static_cast<Torch *>(p2->getInventory());
    torch->illuminate(this, p2->getX(), p2->getY());
  }
}

//////////////////////////////////////////        lightRadius       /////////////////////////////////////////////

// Light circular area around center - MADE WITH AI
void Room::lightRadius(int centerX, int centerY, int radius)
{
  for (int dy = -radius; dy <= radius; dy++)
  {
    for (int dx = -radius; dx <= radius; dx++)
    {
      int x = centerX + dx;
      int y = centerY + dy;

      if (x < 0 || x >= MAX_X || y < 0 || y >= MAX_Y_INGAME)
        continue;

      double distance = sqrt(dx * dx + dy * dy);
      if (distance > radius)
        continue;

      visibilityMap[y][x] = true;
    }
  }
}

//////////////////////////////////////////         isVisible       /////////////////////////////////////////////

bool Room::isVisible(int x, int y) const
{
  if (x < 0 || x >= MAX_X || y < 0 || y >= MAX_Y)
    return false;
  return visibilityMap[y][x];
}

//////////////////////////////////////////     updateAllObjects       /////////////////////////////////////////////

ExplosionResult Room::updateAllObjects(Player *p1, Player *p2)
{
  ExplosionResult totalResult;

  for (GameObject *obj : objects)
  {
    if (obj && obj->isActive())
    {
      if (obj->getType() == ObjectType::BOMB)
      {
        Bomb *bomb = static_cast<Bomb *>(obj);
        ExplosionResult result = bomb->update(p1, p2);

        totalResult.keyDestroyed |= result.keyDestroyed;
        totalResult.player1Hit |= result.player1Hit;
        totalResult.player2Hit |= result.player2Hit;
        totalResult.switchesDestroyed += result.switchesDestroyed;
        totalResult.objectsDestroyed += result.objectsDestroyed;

        if (result.switchesDestroyed > 0)
        {
          totalSwitches -= result.switchesDestroyed;
          if (totalSwitches < 0)
            totalSwitches = 0;
        }
      }
      else
      {
        obj->update();
      }
    }
  }

  for (int i = static_cast<int>(objects.size()) - 1; i >= 0; i--)
  {
    if (objects[i] && !objects[i]->isActive())
    {
      delete objects[i];
      objects.erase(objects.begin() + i);
    }
  }

  for (int i = static_cast<int>(springs.size()) - 1; i >= 0; i--)
  {
    if (springs[i] && springs[i]->allLinksInactive())
    {
      delete springs[i];
      springs.erase(springs.begin() + i);
    }
  }

  for (Obstacle *obstacle : obstacles)
  {
    if (obstacle && obstacle->needsReconstruction())
    {
      obstacle->reconstruct(this);
    }
  }

  for (int i = static_cast<int>(obstacles.size()) - 1; i >= 0; i--)
  {
    Obstacle *obs = obstacles[i];
    if (obs && obs->getBlocks().empty())
    {
      delete obs;
      obstacles.erase(obstacles.begin() + i);
    }
  }

  return totalResult;
}

//////////////////////////////////////////   Multi-Cell Object Detection Helpers       /////////////////////////////////////////////

Direction Room::detectOrientation(const std::vector<Point> &positions)
{
  if (positions.size() < 2)
    return Direction::STAY;

  bool allSameX = true;
  bool allSameY = true;

  int firstX = positions[0].x;
  int firstY = positions[0].y;

  for (size_t i = 1; i < positions.size(); i++)
  {
    if (positions[i].x != firstX)
      allSameX = false;
    if (positions[i].y != firstY)
      allSameY = false;
  }

  if (allSameX)
    return Direction::VERTICAL;
  if (allSameY)
    return Direction::HORIZONTAL;
  return Direction::STAY;
}

std::vector<Point> Room::sortPositions(const std::vector<Point> &positions,
                                       Direction orientation)
{
  std::vector<Point> sorted = positions;

  if (orientation == Direction::HORIZONTAL)
  {
    std::sort(sorted.begin(), sorted.end(),
              [](const Point &a, const Point &b)
              { return a.x < b.x; });
  }
  else
  {
    std::sort(sorted.begin(), sorted.end(),
              [](const Point &a, const Point &b)
              { return a.y < b.y; });
  }

  for (size_t i = 1; i < sorted.size(); i++)
  {
    if (orientation == Direction::HORIZONTAL)
    {
      if (sorted[i].x != sorted[i - 1].x + 1)
      {
        return {};
      }
    }
    else
    {
      if (sorted[i].y != sorted[i - 1].y + 1)
      {
        return {};
      }
    }
  }

  return sorted;
}

Room::WallCheckResult Room::checkWallAdjacency(const std::vector<Point> &sorted,
                                               Direction orientation)
{
  WallCheckResult result;

  if (sorted.empty())
    return result;

  Point first = sorted[0];
  Point last = sorted[sorted.size() - 1];

  if (orientation == Direction::HORIZONTAL)
  {
    char leftChar = baseLayout->getCharAt(first.x - 1, first.y);
    if (leftChar == 'W')
    {
      result.valid = true;
      result.projectionDirection =
          Direction::LEFT;
      result.anchorPosition = first;
      return result;
    }

    char rightChar = baseLayout->getCharAt(last.x + 1, last.y);
    if (rightChar == 'W')
    {
      result.valid = true;
      result.projectionDirection =
          Direction::RIGHT;
      result.anchorPosition = last;
      return result;
    }
  }
  else
  {
    char topChar = baseLayout->getCharAt(first.x, first.y - 1);
    if (topChar == 'W')
    {
      result.valid = true;
      result.projectionDirection = Direction::UP;
      result.anchorPosition = first;
      return result;
    }

    char bottomChar = baseLayout->getCharAt(last.x, last.y + 1);
    if (bottomChar == 'W')
    {
      result.valid = true;
      result.projectionDirection =
          Direction::DOWN;
      result.anchorPosition = last;
      return result;
    }
  }

  return result;
}

//////////////////////////////////////////   scanAndCreateSprings       /////////////////////////////////////////////

void Room::scanAndCreateSprings()
{
  if (baseLayout == nullptr)
    return;

  std::vector<Point> allSpringCells;
  for (int y = 0; y < MAX_Y_INGAME; y++)
  {
    for (int x = 0; x < MAX_X; x++)
    {
      if (baseLayout->getCharAt(x, y) == '#')
      {
        allSpringCells.push_back(Point(x, y));
      }
    }
  }

  bool processed[MAX_Y_INGAME][MAX_X] = {{false}};

  for (const Point &p : allSpringCells)
  {
    if (processed[p.y][p.x])
      continue;

    std::vector<Point> group;
    group.push_back(p);
    processed[p.y][p.x] = true;

    for (size_t i = 0; i < group.size(); i++)
    {
      int x = group[i].x;
      int y = group[i].y;

      Point neighbors[] = {Point(x + 1, y), Point(x - 1, y), Point(x, y + 1),
                           Point(x, y - 1)};

      for (const Point &neighbor : neighbors)
      {
        if (neighbor.x >= 0 && neighbor.x < MAX_X && neighbor.y >= 0 &&
            neighbor.y < MAX_Y_INGAME && !processed[neighbor.y][neighbor.x] &&
            baseLayout->getCharAt(neighbor.x, neighbor.y) == '#')
        {
          group.push_back(neighbor);
          processed[neighbor.y][neighbor.x] = true;
        }
      }
    }

    if (group.size() > 1)
    {
      Direction orientation = detectOrientation(group);
      if (orientation != Direction::STAY)
      {
        std::vector<Point> sorted = sortPositions(group, orientation);
        if (!sorted.empty())
        {
          WallCheckResult wallCheck = checkWallAdjacency(sorted, orientation);
          if (wallCheck.valid)
          {

            bool anchorIsFirst = (wallCheck.anchorPosition == sorted[0]);

            if (anchorIsFirst)
            {
              std::reverse(sorted.begin(), sorted.end());
            }

            Spring *spring = new Spring();
            std::vector<SpringLink *> springLinks;

            bool addFailed = false;
            for (size_t i = 0; i < sorted.size(); i++)
            {
              SpringLink *link =
                  new SpringLink(sorted[i], spring, static_cast<int>(i));
              springLinks.push_back(link);

              if (!addObject(link))
              {
                delete link;
                addFailed = true;
                break;
              }
            }

            if (!addFailed)
            {
              spring->initialize(springLinks, wallCheck.anchorPosition,
                                 wallCheck.projectionDirection);
              springs.push_back(spring);
            }
            else
              delete spring;
          }
        }
      }
    }
  }
}

///////////////////////////////////////////     createMultiCellObject       /////////////////////////////////////////////

void Room::createMultiCellObject(const std::vector<Point> &allObjCells)
{
  if (baseLayout == nullptr)
    return;

  if (allObjCells.empty())
    return;

  char ch = baseLayout->getCharAt(allObjCells[0].x, allObjCells[0].y);

  bool processed[MAX_Y_INGAME][MAX_X] = {{false}};

  for (const Point &p : allObjCells)
  {
    if (processed[p.y][p.x])
      continue;

    std::vector<Point> group;
    std::unordered_map<Point, std::vector<Point>>
        edges;
    group.push_back(p);
    processed[p.y][p.x] = true;

    for (size_t i = 0; i < group.size(); i++)
    {
      int x = group[i].x;
      int y = group[i].y;

      Point neighbors[] = {Point(x + 1, y), Point(x - 1, y), Point(x, y + 1),
                           Point(x, y - 1)};

      for (const Point &neighbor : neighbors)
      {
        if (neighbor.x >= 0 && neighbor.x < MAX_X && neighbor.y >= 0 &&
            neighbor.y < MAX_Y_INGAME && !processed[neighbor.y][neighbor.x])
        {
          if (baseLayout->getCharAt(neighbor.x, neighbor.y) != ch)
          {
            if (ch == '*')
              edges[group[i]].push_back(neighbor);
          }
          else
          {
            group.push_back(neighbor);
            processed[neighbor.y][neighbor.x] = true;
          }
        }
      }
    }

    switch (ch)
    {
    case '#':
      createSpringFromGroup(group);
      break;
    case '*':
      createObstacleFromGroup(group, edges);
      break;
    default:
      break;
    }
  }
}

///////////////////////////////////////////    createSpringFromGroup       /////////////////////////////////////////////

void Room::createSpringFromGroup(const std::vector<Point> &group)
{
  if (group.size() > 1)
  {
    Direction orientation = detectOrientation(group);
    if (orientation != Direction::STAY)
    {
      std::vector<Point> sorted = sortPositions(group, orientation);
      if (!sorted.empty())
      {
        WallCheckResult wallCheck = checkWallAdjacency(sorted, orientation);
        if (wallCheck.valid)
        {

          bool anchorIsFirst = (wallCheck.anchorPosition == sorted[0]);

          if (anchorIsFirst)
          {
            std::reverse(sorted.begin(), sorted.end());
          }

          Spring *spring = new Spring();
          std::vector<SpringLink *> springLinks;

          bool addFailed = false;
          for (size_t i = 0; i < sorted.size(); i++)
          {
            SpringLink *link =
                new SpringLink(sorted[i], spring, static_cast<int>(i));
            springLinks.push_back(link);

            if (!addObject(link))
            {
              delete link;
              addFailed = true;
              break;
            }
          }

          if (!addFailed)
          {
            spring->initialize(springLinks, wallCheck.anchorPosition,
                               wallCheck.projectionDirection);
            springs.push_back(spring);
          }
          else
            delete spring;
        }
      }
    }
  }
}

//////////////////////////////////////////   createObstacleFromGroup       /////////////////////////////////////////////

void Room::createObstacleFromGroup(
    const std::vector<Point> &group,
    std::unordered_map<Point, std::vector<Point>> &neighbors)
{
  Obstacle *obstacle = new Obstacle();
  std::vector<ObstacleBlock *> blocks;

  bool addFailed = false;
  for (const Point &pos : group)
  {
    ObstacleBlock *block = new ObstacleBlock(pos, obstacle);
    blocks.push_back(block);

    if (!addObject(block))
    {
      delete block;
      addFailed = true;
      break;
    }
  }

  if (!addFailed)
  {
    obstacle->initialize(blocks, neighbors);
    obstacles.push_back(obstacle);
  }
  else
  {
    delete obstacle;
  }
}

//////////////////////////////////////////     Legend Drawing       /////////////////////////////////////////////

void Room::drawLegend(Player *p1, Player *p2)
{
  drawEmptyLegend();
  drawLegendInfo(p1, p2);
}

void Room::drawEmptyLegend()
{
  int startX = legendTopLeft.x - 1;
  int startY = legendTopLeft.y - 1;

  for (int i = 0; i < 5; i++)
  {
    gotoxy(startX, startY + i);
    std::cout << legendData[i];
  }
}

void Room::drawLegendInfo(Player *p1, Player *p2)
{
  drawPlayerStats(p1);
  drawPlayerStats(p2);
}

void Room::drawPlayerStats(Player *p)
{

  int lineY = legendTopLeft.y + p->playerId;
  int startX = legendTopLeft.x;

  gotoxy(startX + 3, lineY);
  std::cout << p->getScore();

  DrawLives(p);

  gotoxy(startX + 17, lineY);
  if (p->hasItem())
  {
    std::cout << p->inventory->getSprite();
  }
  else
  {
    std::cout << " ";
  }
}

void Room::DrawLives(Player *p)
{
  int lineY = legendTopLeft.y + p->playerId;
  int startX = legendTopLeft.x - 1;
  int offset = startX + 8;

  switch (p->lives)
  {
  case 3:
    gotoxy(offset, lineY);
    std::cout << "<3 <3 <3";
    break;
  case 2:
    gotoxy(offset + 1, lineY);
    std::cout << "<3 <3";
    break;
  case 1:
    gotoxy(offset + 3, lineY);
    std::cout << "<3";
    break;
  default:
    gotoxy(offset, lineY);
    std::cout << "        ";
    break;
  }
}

//////////////////////////////////////////       isVacantSpot       /////////////////////////////////////////////

bool Room::isVacantSpot(int x, int y)
{
  if (x < 0 || x >= MAX_X || y < 0 || y >= MAX_Y_INGAME)
  {
    return false;
  }

  if (isWallAt(x, y))
  {
    return false;
  }

  for (GameObject *obj : objects)
  {
    if (obj->getX() == x && obj->getY() == y)
    {
      return false;
    }
  }
  return true;
}
