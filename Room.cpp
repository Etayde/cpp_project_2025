
//////////////////////////////////////       INCLUDES & FORWARDS       //////////////////////////////////////////

#include "Room.h"
#include "Player.h"
#include "Door.h"
#include "Switch.h"
#include "StaticObjects.h"
#include "Spring.h"
#include "Bomb.h"
#include <cmath>

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
}

//////////////////////////////////////////      Room Copy Constructor   //////////////////////////////////////////

Room::Room(const Room &other)
    : roomId(other.roomId), active(other.active), completed(other.completed),
      baseLayout(other.baseLayout), mods(other.mods),
      totalKeysInRoom(other.totalKeysInRoom), keysCollected(other.keysCollected),
      activeSwitches(other.activeSwitches), totalSwitches(other.totalSwitches),
      doorReqs(other.doorReqs), nextRoomId(other.nextRoomId), prevRoomId(other.prevRoomId),
      spawnPoint(other.spawnPoint), spawnPointFromNext(other.spawnPointFromNext),
      bombs(other.bombs), darkZones(other.darkZones)
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
        bombs = other.bombs;
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

    // Track which spring cells have been processed
    bool processedSpringCells[MAX_Y_INGAME][MAX_X] = {{false}};

    for (int y = 0; y < MAX_Y_INGAME; y++)
    {
        for (int x = 0; x < MAX_X; x++)
        {
            char ch = baseLayout->getCharAt(x, y);

            // Special handling for springs
            if (ch == '#')
            {
                if (processedSpringCells[y][x])
                    continue; // Already part of a detected spring

                // Detect multi-cell spring
                std::vector<Point> springCells;
                Direction launchDir = Direction::STAY;
                bool validSpring = false;

                // Check for adjacent wall in each direction
                int wallCount = 0;
                Direction wallDirection = Direction::STAY;

                if (x > 0 && getCharAt(x - 1, y) == 'W')
                {
                    wallCount++;
                    wallDirection = Direction::LEFT;
                    launchDir = Direction::RIGHT;
                }
                if (x < MAX_X - 1 && getCharAt(x + 1, y) == 'W')
                {
                    wallCount++;
                    wallDirection = Direction::RIGHT;
                    launchDir = Direction::LEFT;
                }
                if (y > 0 && getCharAt(x, y - 1) == 'W')
                {
                    wallCount++;
                    wallDirection = Direction::UP;
                    launchDir = Direction::DOWN;
                }
                if (y < MAX_Y_INGAME - 1 && getCharAt(x, y + 1) == 'W')
                {
                    wallCount++;
                    wallDirection = Direction::DOWN;
                    launchDir = Direction::UP;
                }

                // Valid spring must have exactly one adjacent wall
                if (wallCount == 1)
                {
                    // Scan perpendicular to wall to find all spring cells
                    springCells.push_back(Point(x, y));
                    processedSpringCells[y][x] = true;

                    // Determine scan direction (perpendicular to wall)
                    int dx = 0, dy = 0;
                    if (wallDirection == Direction::LEFT || wallDirection == Direction::RIGHT)
                    {
                        // Wall is left/right (vertical wall), scan horizontally
                        dx = 1;
                    }
                    else
                    {
                        // Wall is up/down (horizontal wall), scan vertically
                        dy = 1;
                    }

                    // Scan in both directions perpendicular to wall
                    for (int dir = -1; dir <= 1; dir += 2)
                    {
                        int scanX = x + (dx * dir);
                        int scanY = y + (dy * dir);

                        while (scanX >= 0 && scanX < MAX_X && scanY >= 0 && scanY < MAX_Y_INGAME &&
                               getCharAt(scanX, scanY) == '#')
                        {
                            // Add this spring cell (all cells in line are part of spring)
                            springCells.push_back(Point(scanX, scanY));
                            processedSpringCells[scanY][scanX] = true;
                            scanX += (dx * dir);
                            scanY += (dy * dir);
                        }
                    }

                    validSpring = true;
                }

                if (validSpring && !springCells.empty())
                {
                    // Create multi-cell spring object
                    Spring *spring = new Spring(launchDir, springCells.size(), springCells);
                    if (addObject(spring))
                    {
                        // Ensure all spring cells remain as '#' in the display
                        for (const Point &cell : springCells)
                        {
                            setCharAt(cell.x, cell.y, '#');
                        }
                    }
                    else
                    {
                        delete spring;
                    }
                }
                else
                {
                    // Invalid spring - replace with air
                    for (const Point &cell : springCells)
                    {
                        setCharAt(cell.x, cell.y, ' ');
                    }
                    if (!validSpring)
                    {
                        setCharAt(x, y, ' ');
                    }
                }

                continue; // Skip normal object creation for springs
            }

            // Normal object creation for non-spring objects
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
        if (obj != nullptr && obj->isActive() &&
            obj->getX() == x && obj->getY() == y)
        {
            return obj;
        }
    }
    return nullptr;
}

const GameObject *Room::getObjectAt(int x, int y) const
{
    for (const GameObject* obj : objects)
    {
        if (obj != nullptr && obj->isActive() &&
            obj->getX() == x && obj->getY() == y)
        {
            return obj;
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
    if (c == 'W' || c == '=')
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

//////////////////////////////////////////        explodeBomb          //////////////////////////////////////////

// Process bomb explosion - destroy objects, check player hits - MADE WITH AI'S HELP
ExplosionResult Room::explodeBomb(int centerX, int centerY, int radius, Player *p1, Player *p2)
{
    ExplosionResult result;

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
            if (!hasLineOfSight(centerX, centerY, x, y))
                continue;

            char c = getCharAt(x, y);
            if (c == 'W' || (c >= '0' && c <= '9'))
                continue; // Skip walls and doors

            // Check player hits
            if (p1 && p1->getX() == x && p1->getY() == y)
                result.player1Hit = true;
            if (p2 && p2->getX() == x && p2->getY() == y)
                result.player2Hit = true;

            // Process objects using polymorphic onExplosion()
            GameObject *obj = getObjectAt(x, y);
            if (obj != nullptr && obj->isActive())
            {
                if (obj->getType() == ObjectType::KEY)
                    result.keyDestroyed = true;

                if (obj->onExplosion())
                {
                    setCharAt(x, y, ' ');
                    gotoxy(x, y);
                    std::cout << ' ';
                    obj->setActive(false);
                    result.objectsDestroyed++;
                }
            }
            else if (c == '=')
            {
                setCharAt(x, y, ' ');
                gotoxy(x, y);
                std::cout << ' ';
                result.objectsDestroyed++;
            }
            else if (c != ' ' && c != 'W' && !(c >= '0' && c <= '9'))
            {
                setCharAt(x, y, ' ');
                gotoxy(x, y);
                std::cout << ' ';
                result.objectsDestroyed++;
            }
        }
    }

    std::cout.flush();
    return result;
}

//////////////////////////////////////////       handleBombDrop       //////////////////////////////////////////

void Room::handleBombDrop(Player &player)
{
    if (!player.hasBomb())
    {
        player.dropItem(this);
        return;
    }

    // No limit check - allow unlimited bombs
    Point dropPos = player.dropItem(this);

    if (dropPos.x >= 0 && dropPos.y >= 0)
    {
        RoomBomb newBomb;
        newBomb.x = dropPos.x;
        newBomb.y = dropPos.y;
        newBomb.fuseTimer = BombConfig::FUSE_TIME;
        newBomb.active = true;
        bombs.push_back(newBomb);
    }
}

//////////////////////////////////////////        updateBomb          //////////////////////////////////////////

bool Room::updateBomb(Player *p1, Player *p2)
{
    if (bombs.empty())
    {
        return false;
    }

    bool anyPlayerHit = false;

    // Update all bombs (iterate backwards to safely remove)
    for (int i = bombs.size() - 1; i >= 0; i--)
    {
        RoomBomb &bomb = bombs[i];

        if (!bomb.active)
            continue;

        bomb.decrementFuse();

        // Blink effect
        gotoxy(bomb.x, bomb.y);
        if (bomb.fuseTimer % BombConfig::BLINK_RATE < 5)
            std::cout << '@';
        else
            std::cout << '*';
        std::cout.flush();

        // Explode
        if (bomb.isReadyToExplode())
        {
            removeObjectAt(bomb.x, bomb.y);
            setCharAt(bomb.x, bomb.y, ' ');
            gotoxy(bomb.x, bomb.y);
            std::cout << ' ' << std::flush;

            ExplosionResult result = explodeBomb(
                bomb.x, bomb.y, BombConfig::RADIUS,
                p1, p2);

            // Remove this bomb from vector
            bombs.erase(bombs.begin() + i);

            p1->draw();
            p2->draw();

            if (result.keyDestroyed || result.player1Hit || result.player2Hit)
            {
                anyPlayerHit = true;
            }
        }
    }

    return anyPlayerHit;
}

//////////////////////////////////////////      getSpringAt           //////////////////////////////////////////

// Returns Spring object containing the given cell, or nullptr if no spring there
Spring *Room::getSpringAt(int x, int y)
{
    for (GameObject *obj : objects)
    {
        if (obj != nullptr && obj->isActive() && obj->getType() == ObjectType::SPRING)
        {
            Spring *spring = static_cast<Spring *>(obj);
            if (spring->containsCell(x, y))
            {
                return spring;
            }
        }
    }
    return nullptr;
}

//////////////////////////////////////////   updateMovingObstacles    //////////////////////////////////////////

// Update all moving obstacles
void Room::updateMovingObstacles(Player *p1, Player *p2)
{
    for (GameObject *obj : objects)
    {
        if (obj != nullptr && obj->isActive() && obj->getType() == ObjectType::OBSTACLE)
        {
            Obstacle *obstacle = static_cast<Obstacle *>(obj);
            if (obstacle->getIsMoving())
            {
                obstacle->updateMovement(this);
            }
        }
    }
}

//////////////////////////////////////////  Obstacle::updateMovement  //////////////////////////////////////////

// Update obstacle position during movement (implemented here to access Room)
bool Obstacle::updateMovement(Room *room)
{
    if (!isMoving || duration <= 0 || room == nullptr)
    {
        isMoving = false;
        return true;
    }

    // Try to move in the velocity direction
    int nextX = position.x + velocityX;
    int nextY = position.y + velocityY;

    // Check if blocked
    if (room->isBlocked(nextX, nextY))
    {
        isMoving = false;
        return true;
    }

    // Erase from current position
    room->setCharAt(position.x, position.y, ' ');
    gotoxy(position.x, position.y);
    std::cout << ' ' << std::flush;

    // Move to new position
    position.x = nextX;
    position.y = nextY;
    room->setCharAt(nextX, nextY, sprite);

    // Draw at new position
    gotoxy(nextX, nextY);
    std::cout << sprite << std::flush;

    // Decrement duration
    duration--;
    if (duration <= 0)
    {
        isMoving = false;
        return true;
    }

    return false;
}
