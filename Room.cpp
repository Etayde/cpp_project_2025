
//////////////////////////////////////       INCLUDES & FORWARDS       //////////////////////////////////////////

#include "Room.h"
#include "Player.h"
#include "Door.h"
#include "Switch.h"
#include "StaticObjects.h"
#include "Bomb.h"
#include "Spring.h"
#include "SpringLink.h"
#include "Obstacle.h"
#include "DebugLog.h"
#include <vector>
#include <unordered_map>
#include <cmath>
#include <algorithm>

//////////////////////////////////////////      Room Constructors      //////////////////////////////////////////

Room::Room()
    : roomId(-1), active(false), completed(false), baseLayout(nullptr),
      totalKeysInRoom(0), keysCollected(0),
      activeSwitches(0), totalSwitches(0), nextRoomId(-1), prevRoomId(-1)
{
    initVisibility();
}

Room::Room(int id)
    : roomId(id), active(false), completed(false), baseLayout(nullptr),
      totalKeysInRoom(0), keysCollected(0),
      activeSwitches(0), totalSwitches(0), nextRoomId(-1), prevRoomId(-1)
{
    initVisibility();
}

//////////////////////////////////////////       Room Destructor       //////////////////////////////////////////

Room::~Room()
{
    deleteAllObjects();

    // Delete springs (separate from objects)
    for (Spring* spring : springs)
    {
        delete spring;
    }
    springs.clear();
}

//////////////////////////////////////////      Room Copy Constructor   //////////////////////////////////////////

Room::Room(const Room &other)
    : roomId(other.roomId), active(other.active), completed(other.completed),
      baseLayout(other.baseLayout), mods(other.mods),
      totalKeysInRoom(other.totalKeysInRoom), keysCollected(other.keysCollected),
      activeSwitches(other.activeSwitches), totalSwitches(other.totalSwitches),
      doorReqs(other.doorReqs), nextRoomId(other.nextRoomId), prevRoomId(other.prevRoomId),
      spawnPoint(other.spawnPoint), spawnPointFromNext(other.spawnPointFromNext),
      darkZones(other.darkZones)
{
    for (int y = 0; y < MAX_Y; y++)
        for (int x = 0; x < MAX_X; x++)
            visibilityMap[y][x] = other.visibilityMap[y][x];

    copyObjectsFrom(other);
}

//////////////////////////////////////////    Room Assignment Operator  //////////////////////////////////////////

Room &Room::operator=(const Room &other)
{
    if (this != &other)
    {
        deleteAllObjects();

        roomId = other.roomId;
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

//////////////////////////////////////////       Private Helpers       //////////////////////////////////////////

// Deep copy objects
void Room::copyObjectsFrom(const Room &other)
{
    objects.clear();
    for (GameObject* obj : other.objects)
    {
        if (obj != nullptr)
        {
            objects.push_back(obj->clone());
        }
    }

    // Copy springs (note: this is a shallow copy of spring managers)
    // Spring pointers will reference the original springs
    // For deep copy, would need Spring::clone() method
    springs = other.springs;
}

// Delete all allocated objects
void Room::deleteAllObjects()
{
    for (GameObject* obj : objects)
    {
        delete obj;
    }
    objects.clear();
}

// Set all visibility to true
void Room::initVisibility()
{
    for (int y = 0; y < MAX_Y; y++)
        for (int x = 0; x < MAX_X; x++)
            visibilityMap[y][x] = true;
}

//////////////////////////////////////////       initFromLayout        //////////////////////////////////////////

// Initialize room from a screen layout
void Room::initFromLayout(const Screen *layout)
{
    baseLayout = layout;
    mods.clear();
    deleteAllObjects();
    loadObjects();
}

//////////////////////////////////////////        loadObjects          //////////////////////////////////////////

// Scan layout and create objects
void Room::loadObjects()
{
    totalKeysInRoom = 0;
    keysCollected = 0;
    activeSwitches = 0;
    totalSwitches = 0;

    if (baseLayout == nullptr)
        return;

    for (int y = 0; y < MAX_Y_INGAME; y++)
    {
        for (int x = 0; x < MAX_X; x++)
        {
            char ch = baseLayout->getCharAt(x, y);
            GameObject *obj = createObjectFromChar(ch, x, y);

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

    // Auto-scan and create springs
    scanAndCreateSprings();
}

//////////////////////////////////////////    setDoorRequirements      //////////////////////////////////////////

// Set key and switch requirements for a door
void Room::setDoorRequirements(int doorId, int keys, int switches)
{
    if (doorId < 0)
        return;

    // Ensure vector is large enough
    if (doorId >= static_cast<int>(doorReqs.size()))
    {
        doorReqs.resize(doorId + 1);
    }

    doorReqs[doorId].doorId = doorId;
    doorReqs[doorId].requiredKeys = keys;
    doorReqs[doorId].requiredSwitches = switches;
    doorReqs[doorId].isUnlocked = false;
}

//////////////////////////////////////////           draw              //////////////////////////////////////////

// Draw room: base layout -> modifications -> darkness
void Room::draw()
{
    if (baseLayout != nullptr)
        baseLayout->draw();

    for (const Modification& mod : mods)
    {
        gotoxy(mod.x, mod.y);
        std::cout << mod.newChar;
    }

    drawDarkness();
    std::cout.flush();
}

//////////////////////////////////////////        drawDarkness         //////////////////////////////////////////

// Draw darkness overlay for dark zones
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
    std::cout.flush();
}

//////////////////////////////////////////         getCharAt           //////////////////////////////////////////

// Get character at position (checks mods first, then base)
char Room::getCharAt(int x, int y) const
{
    for (const Modification& mod : mods)
    {
        if (mod.x == x && mod.y == y)
            return mod.newChar;
    }

    if (baseLayout != nullptr)
        return baseLayout->getCharAt(x, y);
    return 'W';
}

//////////////////////////////////////////         setCharAt           //////////////////////////////////////////

// Set/modify character at position
void Room::setCharAt(int x, int y, char c)
{
    for (Modification& mod : mods)
    {
        if (mod.x == x && mod.y == y)
        {
            mod.newChar = c;
            return;
        }
    }

    mods.push_back(Modification(x, y, c));
}

//////////////////////////////////////////         resetMods           //////////////////////////////////////////

void Room::resetMods()
{
    mods.clear();
}

//////////////////////////////////////////        getObjectAt          //////////////////////////////////////////

// Get object at position
GameObject *Room::getObjectAt(int x, int y)
{
    for (GameObject* obj : objects)
    {
        if (obj != nullptr && obj->isActive())
        {
            // All objects (including SpringLinks) are single-position
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
    for (const GameObject* obj : objects)
    {
        if (obj != nullptr && obj->isActive())
        {
            // All objects (including SpringLinks) are single-position
            if (obj->getX() == x && obj->getY() == y)
            {
                return obj;
            }
        }
    }
    return nullptr;
}

//////////////////////////////////////////         addObject           //////////////////////////////////////////

// Add object to room
bool Room::addObject(GameObject *obj)
{
    if (obj == nullptr)
        return false;

    objects.push_back(obj);
    setCharAt(obj->getX(), obj->getY(), obj->getSprite());

    return true;
}

//////////////////////////////////////////        removeObject         //////////////////////////////////////////

// Remove object at given index
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

//////////////////////////////////////////       removeObjectAt        //////////////////////////////////////////

// Remove object at position
void Room::removeObjectAt(int x, int y)
{
    for (size_t i = 0; i < objects.size(); i++)
    {
        if (objects[i] != nullptr && objects[i]->getX() == x && objects[i]->getY() == y)
        {
            removeObject(i);
            return;
        }
    }
}

//////////////////////////////////////////          getDoors           //////////////////////////////////////////

// Get all doors in the room
std::vector<Door *> Room::getDoors()
{
    std::vector<Door *> doors;
    for (GameObject* obj : objects)
    {
        if (obj != nullptr && obj->getType() == ObjectType::DOOR)
        {
            doors.push_back(static_cast<Door *>(obj));
        }
    }
    return doors;
}

//////////////////////////////////////////        getSwitches          //////////////////////////////////////////

// Get all switches in the room
std::vector<Switch *> Room::getSwitches()
{
    std::vector<Switch *> switches;
    for (GameObject* obj : objects)
    {
        if (obj != nullptr &&
            (obj->getType() == ObjectType::SWITCH_ON ||
             obj->getType() == ObjectType::SWITCH_OFF))
        {
            switches.push_back(static_cast<Switch *>(obj));
        }
    }
    return switches;
}

//////////////////////////////////////////         isBlocked           //////////////////////////////////////////

// Check if position is blocked
bool Room::isBlocked(int x, int y)
{
    char c = getCharAt(x, y);
    if (c == 'W' || c == 'w')
        return true;

    GameObject *obj = getObjectAt(x, y);
    if (obj != nullptr)
        return obj->isBlocking();

    if (c == '*')
        return true;
    return false;
}

//////////////////////////////////////////      hasLineOfSight         //////////////////////////////////////////

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
            char c = getCharAt(x, y);
            if (c == 'W' || (c >= '0' && c <= '9'))
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

//////////////////////////////////////////     updatePuzzleState       //////////////////////////////////////////

// Check switches and remove switch walls if complete
void Room::updatePuzzleState()
{
    activeSwitches = countActiveSwitches();

    if (activeSwitches >= totalSwitches && totalSwitches > 0 && !completed)
    {
        completed = true;

        for (GameObject* obj : objects)
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
        std::cout.flush();
    }
}

//////////////////////////////////////////    countActiveSwitches      //////////////////////////////////////////

int Room::countActiveSwitches() const
{
    int count = 0;
    for (const GameObject* obj : objects)
    {
        if (obj != nullptr && obj->getType() == ObjectType::SWITCH_ON)
            count++;
    }
    return count;
}

//////////////////////////////////////////        canOpenDoor          //////////////////////////////////////////

// Check if door requirements are met
bool Room::canOpenDoor(int doorId, int player1Keys, int player2Keys) const
{
    if (doorId < 0 || doorId >= static_cast<int>(doorReqs.size()))
        return false;

    const DoorRequirements &req = doorReqs[doorId];

    if (req.isUnlocked)
        return true;
    if (req.requiredSwitches > 0 && activeSwitches < req.requiredSwitches)
        return false;

    // Check total keys from both players
    int totalKeys = player1Keys + player2Keys;
    if (req.requiredKeys > 0 && totalKeys < req.requiredKeys)
        return false;

    return true;
}

//////////////////////////////////////////        getDoorIdAt          //////////////////////////////////////////

int Room::getDoorIdAt(int x, int y) const
{
    char c = getCharAt(x, y);
    if (c >= '0' && c <= '9')
        return c - '0';
    return -1;
}

//////////////////////////////////////////        unlockDoor           //////////////////////////////////////////

void Room::unlockDoor(int doorId)
{
    if (doorId >= 0 && doorId < static_cast<int>(doorReqs.size()))
        doorReqs[doorId].isUnlocked = true;
}

//////////////////////////////////////////      isDoorUnlocked         //////////////////////////////////////////

bool Room::isDoorUnlocked(int doorId) const
{
    if (doorId >= 0 && doorId < static_cast<int>(doorReqs.size()))
        return doorReqs[doorId].isUnlocked;
    return false;
}

//////////////////////////////////////////        addDarkZone          //////////////////////////////////////////

void Room::addDarkZone(int x1, int y1, int x2, int y2)
{
    darkZones.push_back(DarkZone(x1, y1, x2, y2));
}

//////////////////////////////////////////       clearDarkZones        //////////////////////////////////////////

void Room::clearDarkZones()
{
    darkZones.clear();
    initVisibility();
}

//////////////////////////////////////////       isInDarkZone          //////////////////////////////////////////

bool Room::isInDarkZone(int x, int y) const
{
    for (const DarkZone& zone : darkZones)
    {
        if (zone.contains(x, y))
            return true;
    }
    return false;
}

//////////////////////////////////////////      updateVisibility       //////////////////////////////////////////

// Update visibility based on player torches
void Room::updateVisibility(Player *p1, Player *p2)
{
    if (darkZones.empty())
        return;

    initVisibility();

    for (const DarkZone& zone : darkZones)
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

    // Players with torches light up area
    if (p1 != nullptr && p1->hasTorch() && isInDarkZone(p1->getX(), p1->getY()))
        lightRadius(p1->getX(), p1->getY(), LightConfig::TORCH_RADIUS);
    if (p2 != nullptr && p2->hasTorch() && isInDarkZone(p2->getX(), p2->getY()))
        lightRadius(p2->getX(), p2->getY(), LightConfig::TORCH_RADIUS);
}

//////////////////////////////////////////        lightRadius          //////////////////////////////////////////

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

//////////////////////////////////////////         isVisible           //////////////////////////////////////////

bool Room::isVisible(int x, int y) const
{
    if (x < 0 || x >= MAX_X || y < 0 || y >= MAX_Y)
        return false;
    return visibilityMap[y][x];
}

//////////////////////////////////////////     updateAllObjects       //////////////////////////////////////////

ExplosionResult Room::updateAllObjects(Player *p1, Player *p2)
{
    ExplosionResult totalResult;

    for (GameObject *obj : objects)
    {
        if (obj && obj->isActive())
        {
            if (obj->getType() == ObjectType::BOMB)
            {
                // Bombs need player references and return explosion results
                Bomb *bomb = static_cast<Bomb *>(obj);
                ExplosionResult result = bomb->update(p1, p2);

                // Accumulate bomb explosion results
                totalResult.keyDestroyed |= result.keyDestroyed;
                totalResult.player1Hit |= result.player1Hit;
                totalResult.player2Hit |= result.player2Hit;
                totalResult.objectsDestroyed += result.objectsDestroyed;
            }
            else
            {
                obj->update(); // Standard update
            }
        }
    }

    return totalResult;
}

Direction Room::detectOrientation(const std::vector<Point>& positions)
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
        return Direction::VERTICAL;   // Same column
    if (allSameY)
        return Direction::HORIZONTAL; // Same row
    return Direction::STAY; // Not aligned
}

std::vector<Point> Room::sortPositions(const std::vector<Point>& positions, Direction orientation)
{
    std::vector<Point> sorted = positions;

    if (orientation == Direction::HORIZONTAL)
    {
        // Sort by x coordinate (left to right)
        std::sort(sorted.begin(), sorted.end(),
                  [](const Point& a, const Point& b) { return a.x < b.x; });
    }
    else
    {
        // Sort by y coordinate (top to bottom)
        std::sort(sorted.begin(), sorted.end(),
                  [](const Point& a, const Point& b) { return a.y < b.y; });
    }

    // Verify consecutive
    for (size_t i = 1; i < sorted.size(); i++)
    {
        if (orientation == Direction::HORIZONTAL)
        {
            if (sorted[i].x != sorted[i-1].x + 1)
            {
                return {}; // Not consecutive
            }
        }
        else
        {
            if (sorted[i].y != sorted[i-1].y + 1)
            {
                return {}; // Not consecutive
            }
        }
    }

    return sorted;
}

Room::WallCheckResult Room::checkWallAdjacency(const std::vector<Point>& sorted, Direction orientation)
{
    WallCheckResult result;

    if (sorted.empty())
        return result;

    // Check first position (index 0) for wall adjacency
    Point first = sorted[0];
    Point last = sorted[sorted.size() - 1];

    if (orientation == Direction::HORIZONTAL)
    {
        // Check left of first position
        char leftChar = baseLayout->getCharAt(first.x - 1, first.y);
        if (leftChar == 'W' || leftChar == 'w')
        {
            result.valid = true;
            result.projectionDirection = Direction::LEFT;  // Compress TOWARD wall (left)
            result.anchorPosition = first;
            return result;
        }

        // Check right of last position
        char rightChar = baseLayout->getCharAt(last.x + 1, last.y);
        if (rightChar == 'W' || rightChar == 'w')
        {
            result.valid = true;
            result.projectionDirection = Direction::RIGHT;  // Compress TOWARD wall (right)
            result.anchorPosition = last;
            return result;
        }
    }
    else // VERTICAL
    {
        // Check above first position
        char topChar = baseLayout->getCharAt(first.x, first.y - 1);
        if (topChar == 'W' || topChar == 'w')
        {
            result.valid = true;
            result.projectionDirection = Direction::UP;  // Compress TOWARD wall (up)
            result.anchorPosition = first;
            return result;
        }

        // Check below last position
        char bottomChar = baseLayout->getCharAt(last.x, last.y + 1);
        if (bottomChar == 'W' || bottomChar == 'w')
        {
            result.valid = true;
            result.projectionDirection = Direction::DOWN;  // Compress TOWARD wall (down)
            result.anchorPosition = last;
            return result;
        }
    }

    return result;
}

//////////////////////////////////////////   scanAndCreateSprings   //////////////////////////////////////////

void Room::scanAndCreateSprings()
{
    if (baseLayout == nullptr)
        return;

    // Step 1: Collect all '#' positions
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

    // Step 2: Group adjacent cells into springs
    bool processed[MAX_Y_INGAME][MAX_X] = {false};

    for (const Point& p : allSpringCells)
    {
        if (processed[p.y][p.x])
            continue;

        // Start new spring group
        std::vector<Point> group;
        group.push_back(p);
        processed[p.y][p.x] = true;

        // Collect all adjacent cells (horizontal or vertical)
        for (size_t i = 0; i < group.size(); i++)
        {
            int x = group[i].x;
            int y = group[i].y;

            // Check 4 neighbors
            Point neighbors[] = {
                Point(x + 1, y), Point(x - 1, y),
                Point(x, y + 1), Point(x, y - 1)
            };

            for (const Point& neighbor : neighbors)
            {
                if (neighbor.x >= 0 && neighbor.x < MAX_X &&
                    neighbor.y >= 0 && neighbor.y < MAX_Y_INGAME &&
                    !processed[neighbor.y][neighbor.x] &&
                    baseLayout->getCharAt(neighbor.x, neighbor.y) == '#')
                {
                    group.push_back(neighbor);
                    processed[neighbor.y][neighbor.x] = true;
                }
            }
        }

        // Step 3: Validate and create spring
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
                        // Ensure sorted array has START cell first, ANCHOR cell last
                        // If anchor is at the beginning (sorted[0]), reverse the array
                        bool anchorIsFirst = (wallCheck.anchorPosition == sorted[0]);

                        DebugLog::getStream() << "[ROOM_SPRING_SCAN] Found spring group with " << sorted.size()
                                              << " cells | Orientation: " << static_cast<int>(orientation)
                                              << " | Anchor at: (" << wallCheck.anchorPosition.x << "," << wallCheck.anchorPosition.y << ")"
                                              << " | Projection dir: " << static_cast<int>(wallCheck.projectionDirection) << std::endl;

                        if (anchorIsFirst)
                        {
                            DebugLog::getStream() << "[ROOM_SPRING_SCAN] Reversing array (anchor was first)" << std::endl;
                            std::reverse(sorted.begin(), sorted.end());
                        }

                        DebugLog::getStream() << "[ROOM_SPRING_SCAN] Start cell: (" << sorted[0].x << "," << sorted[0].y << ")"
                                              << " | End cell: (" << sorted[sorted.size()-1].x << "," << sorted[sorted.size()-1].y << ")" << std::endl;

                        // Create Spring manager
                        Spring* spring = new Spring();
                        std::vector<SpringLink*> springLinks;

                        // Create SpringLink for each cell
                        bool addFailed = false;
                        for (size_t i = 0; i < sorted.size(); i++)
                        {
                            SpringLink* link = new SpringLink(sorted[i], spring, static_cast<int>(i));
                            springLinks.push_back(link);

                            // Add to objects vector
                            if (!addObject(link))
                            {
                                DebugLog::getStream() << "[ROOM_SPRING_SCAN] ERROR: Failed to add SpringLink at index " << i << std::endl;
                                delete link;
                                addFailed = true;
                                break;
                            }
                        }

                        // If all links added successfully, initialize spring and add to springs vector
                        if (!addFailed)
                        {
                            spring->initialize(springLinks, wallCheck.anchorPosition, wallCheck.projectionDirection);
                            springs.push_back(spring);
                            DebugLog::getStream() << "[ROOM_SPRING_SCAN] Spring created successfully with " << springLinks.size() << " links" << std::endl;
                        }
                        else
                        {
                            // Cleanup on failure
                            delete spring;
                            DebugLog::getStream() << "[ROOM_SPRING_SCAN] Spring creation failed - cleaned up" << std::endl;
                            // Links already added to objects will be cleaned up by deleteAllObjects()
                        }
                    }
                }
            }
        }
    }
}

///////////////////////////////////////////     createMultiCellObject   //////////////////////////////////////////

void Room::createMultiCellObject(char ch)
{
    if (baseLayout == nullptr)
        return;

    // Step 1: Collect all ch positions
    std::vector<Point> allObjCells;
    for (int y = 0; y < MAX_Y_INGAME; y++)
    {
        for (int x = 0; x < MAX_X; x++)
        {
            if (baseLayout->getCharAt(x, y) == ch)
            {
                allObjCells.push_back(Point(x, y));
            }
        }
    }

    // Step 2: Group adjacent cells into springs
    bool processed[MAX_Y_INGAME][MAX_X] = {false};

    for (const Point& p : allObjCells)
    {
        if (processed[p.y][p.x])
            continue;

        // Start new spring group
        std::vector<Point> group;
        std::unordered_map<Point, std::vector<Point>> edges;  // To store edge positions for obstacles
        group.push_back(p);
        processed[p.y][p.x] = true;

        // Collect all adjacent cells (horizontal or vertical)
        for (size_t i = 0; i < group.size(); i++)
        {
            int x = group[i].x;
            int y = group[i].y;

            // Check 4 neighbors
            Point neighbors[] = {
                Point(x + 1, y), Point(x - 1, y),
                Point(x, y + 1), Point(x, y - 1)
            };

            for (const Point& neighbor : neighbors)
            {
                if (neighbor.x >= 0 && neighbor.x < MAX_X &&
                    neighbor.y >= 0 && neighbor.y < MAX_Y_INGAME &&
                    !processed[neighbor.y][neighbor.x])
                {
                    if (!baseLayout->getCharAt(neighbor.x, neighbor.y) == ch) 
                        switch (ch)
                        {
                            case '*':
                                edges[group[i]].push_back(neighbor);
                            default:
                                continue;
                        }
                
                    group.push_back(neighbor);
                    processed[neighbor.y][neighbor.x] = true;
                
                }
            }
        }
    
        // Step 3: Process the collected group based on character type
        switch (ch)
        {
            case '#':
                // Handle springs separately
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

///////////////////////////////////////////    createSpringFromGroup   //////////////////////////////////////////

void Room::createSpringFromGroup(const std::vector<Point>& group)
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
                    // Ensure sorted array has START cell first, ANCHOR cell last
                    // If anchor is at the beginning (sorted[0]), reverse the array
                    bool anchorIsFirst = (wallCheck.anchorPosition == sorted[0]);

                    DebugLog::getStream() << "[ROOM_SPRING_SCAN] Found spring group with " << sorted.size()
                                            << " cells | Orientation: " << static_cast<int>(orientation)
                                            << " | Anchor at: (" << wallCheck.anchorPosition.x << "," << wallCheck.anchorPosition.y << ")"
                                            << " | Projection dir: " << static_cast<int>(wallCheck.projectionDirection) << std::endl;

                    if (anchorIsFirst)
                    {
                        DebugLog::getStream() << "[ROOM_SPRING_SCAN] Reversing array (anchor was first)" << std::endl;
                        std::reverse(sorted.begin(), sorted.end());
                    }

                    DebugLog::getStream() << "[ROOM_SPRING_SCAN] Start cell: (" << sorted[0].x << "," << sorted[0].y << ")"
                                            << " | End cell: (" << sorted[sorted.size()-1].x << "," << sorted[sorted.size()-1].y << ")" << std::endl;

                    // Create Spring manager
                    Spring* spring = new Spring();
                    std::vector<SpringLink*> springLinks;

                    // Create SpringLink for each cell
                    bool addFailed = false;
                    for (size_t i = 0; i < sorted.size(); i++)
                    {
                        SpringLink* link = new SpringLink(sorted[i], spring, static_cast<int>(i));
                        springLinks.push_back(link);

                        // Add to objects vector
                        if (!addObject(link))
                        {
                            DebugLog::getStream() << "[ROOM_SPRING_SCAN] ERROR: Failed to add SpringLink at index " << i << std::endl;
                            delete link;
                            addFailed = true;
                            break;
                        }
                    }

                    // If all links added successfully, initialize spring and add to springs vector
                    if (!addFailed)
                    {
                        spring->initialize(springLinks, wallCheck.anchorPosition, wallCheck.projectionDirection);
                        springs.push_back(spring);
                        DebugLog::getStream() << "[ROOM_SPRING_SCAN] Spring created successfully with " << springLinks.size() << " links" << std::endl;
                    }
                    else
                    {
                        // Cleanup on failure
                        delete spring;
                        DebugLog::getStream() << "[ROOM_SPRING_SCAN] Spring creation failed - cleaned up" << std::endl;
                        // Links already added to objects will be cleaned up by deleteAllObjects()
                    }
                }
            }
        }
    }
}

//////////////////////////////////////////   createObstacleFromGroup  //////////////////////////////////////////

void Room::createObstacleFromGroup(const std::vector<Point>& group,std::unordered_map<Point, std::vector<Point>>& neighbors)
{
    Obstacle* obstacle = new Obstacle();
    std::vector<ObstacleBlock*> blocks;

    bool addFailed = false;
    for (const Point& pos : group)
    {
        ObstacleBlock* block = new ObstacleBlock(pos, obstacle);
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
        obstacle->initialize(blocks);
        obstacles.push_back(obstacle);
    }
    else
    {
        delete obstacle;
    }
}


void Room::neighborsToEdgeDirections(ObstacleBlock* block, 
    std::unordered_map<Point, std::vector<Point>>& neighbors) const
{
    int x = block->getX();
    int y = block->getY();

    std::vector<Direction> edgeDirections;
    std::vector<Point> neighborPositions = neighbors[Point(x, y)];

    if (neighborPositions.empty())
        return;

    for (const Point& np : neighborPositions)
    {
        if (np.x == x && np.y == y - 1)
            edgeDirections.push_back(Direction::UP);
        else if (np.x == x && np.y == y + 1)
            edgeDirections.push_back(Direction::DOWN);
        else if (np.x == x - 1 && np.y == y)
            edgeDirections.push_back(Direction::LEFT);
        else if (np.x == x + 1 && np.y == y)
            edgeDirections.push_back(Direction::RIGHT);
    }

    block->setEdgeDirections(edgeDirections);

    return;
}