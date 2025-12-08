
#include "Room.h"
#include "Player.h"
#include "Door.h"
#include "Switch.h"
#include "StaticObjects.h"
#include "Bomb.h"
#include <cmath>

//////////////////////////////////////////      Room Constructors      //////////////////////////////////////////

Room::Room() 
    : roomId(-1), active(false), completed(false), baseLayout(nullptr),
      modCount(0), objectCount(0), totalKeysInRoom(0), keysCollected(0),
      activeSwitches(0), totalSwitches(0), nextRoomId(-1), prevRoomId(-1),
      darkZoneCount(0) 
{
    for (int i = 0; i < RoomLimits::MAX_OBJECTS; i++) {
        objects[i] = nullptr;
    }
    
    for (int i = 0; i < MAX_DOORS; i++) {
        doorReqs[i] = DoorRequirements(i, DoorConfig::DEFAULT_REQUIRED_KEYS);
    }
    
    initVisibility();
}

Room::Room(int id) 
    : roomId(id), active(false), completed(false), baseLayout(nullptr),
      modCount(0), objectCount(0), totalKeysInRoom(0), keysCollected(0),
      activeSwitches(0), totalSwitches(0), nextRoomId(-1), prevRoomId(-1),
      darkZoneCount(0) 
{
    for (int i = 0; i < RoomLimits::MAX_OBJECTS; i++) {
        objects[i] = nullptr;
    }
    
    for (int i = 0; i < MAX_DOORS; i++) {
        doorReqs[i] = DoorRequirements(i, DoorConfig::DEFAULT_REQUIRED_KEYS);
    }
    
    initVisibility();
}

//////////////////////////////////////////       Room Destructor       //////////////////////////////////////////

Room::~Room() {
    deleteAllObjects();
}

//////////////////////////////////////////      Room Copy Constructor   //////////////////////////////////////////

Room::Room(const Room& other) 
    : roomId(other.roomId), active(other.active), completed(other.completed),
      baseLayout(other.baseLayout), modCount(other.modCount), objectCount(0),
      totalKeysInRoom(other.totalKeysInRoom), keysCollected(other.keysCollected),
      activeSwitches(other.activeSwitches), totalSwitches(other.totalSwitches),
      nextRoomId(other.nextRoomId), prevRoomId(other.prevRoomId),
      spawnPoint(other.spawnPoint), spawnPointFromNext(other.spawnPointFromNext),
      bomb(other.bomb), darkZoneCount(other.darkZoneCount)
{
    for (int i = 0; i < modCount; i++) mods[i] = other.mods[i];
    for (int i = 0; i < MAX_DOORS; i++) doorReqs[i] = other.doorReqs[i];
    for (int i = 0; i < darkZoneCount; i++) darkZones[i] = other.darkZones[i];
    
    for (int y = 0; y < MAX_Y; y++)
        for (int x = 0; x < MAX_X; x++)
            visibilityMap[y][x] = other.visibilityMap[y][x];
    
    for (int i = 0; i < RoomLimits::MAX_OBJECTS; i++) objects[i] = nullptr;
    
    copyObjectsFrom(other);
}

//////////////////////////////////////////    Room Assignment Operator  //////////////////////////////////////////

Room& Room::operator=(const Room& other) {
    if (this != &other) {
        deleteAllObjects();
        
        roomId = other.roomId;
        active = other.active;
        completed = other.completed;
        baseLayout = other.baseLayout;
        modCount = other.modCount;
        totalKeysInRoom = other.totalKeysInRoom;
        keysCollected = other.keysCollected;
        activeSwitches = other.activeSwitches;
        totalSwitches = other.totalSwitches;
        nextRoomId = other.nextRoomId;
        prevRoomId = other.prevRoomId;
        spawnPoint = other.spawnPoint;
        spawnPointFromNext = other.spawnPointFromNext;
        bomb = other.bomb;
        darkZoneCount = other.darkZoneCount;
        
        for (int i = 0; i < modCount; i++) mods[i] = other.mods[i];
        for (int i = 0; i < MAX_DOORS; i++) doorReqs[i] = other.doorReqs[i];
        for (int i = 0; i < darkZoneCount; i++) darkZones[i] = other.darkZones[i];
        
        for (int y = 0; y < MAX_Y; y++)
            for (int x = 0; x < MAX_X; x++)
                visibilityMap[y][x] = other.visibilityMap[y][x];
        
        copyObjectsFrom(other);
    }
    return *this;
}

//////////////////////////////////////////       Private Helpers       //////////////////////////////////////////

// Deep copy objects using polymorphic clone()
void Room::copyObjectsFrom(const Room& other) {
    objectCount = 0;
    for (int i = 0; i < other.objectCount; i++) {
        if (other.objects[i] != nullptr) {
            objects[objectCount] = other.objects[i]->clone();
            objectCount++;
        }
    }
}

// Delete all allocated objects
void Room::deleteAllObjects() {
    for (int i = 0; i < objectCount; i++) {
        delete objects[i];
        objects[i] = nullptr;
    }
    objectCount = 0;
}

// Set all visibility to true
void Room::initVisibility() {
    for (int y = 0; y < MAX_Y; y++)
        for (int x = 0; x < MAX_X; x++)
            visibilityMap[y][x] = true;
}

//////////////////////////////////////////       initFromLayout        //////////////////////////////////////////

// Initialize room from a screen layout
void Room::initFromLayout(const Screen* layout) {
    baseLayout = layout;
    modCount = 0;
    deleteAllObjects();
    loadObjects();
}

//////////////////////////////////////////        loadObjects          //////////////////////////////////////////

// Scan layout and create objects using factory function
void Room::loadObjects() {
    totalKeysInRoom = 0;
    keysCollected = 0;
    activeSwitches = 0;
    totalSwitches = 0;
    
    if (baseLayout == nullptr) return;
    
    for (int y = 0; y < MAX_Y_INGAME; y++) {
        for (int x = 0; x < MAX_X; x++) {
            char ch = baseLayout->getCharAt(x, y);
            GameObject* obj = createObjectFromChar(ch, x, y);
            
            if (obj != nullptr) {
                // Track keys and switches
                if (obj->getType() == ObjectType::KEY) totalKeysInRoom++;
                else if (obj->getType() == ObjectType::SWITCH_OFF) totalSwitches++;
                else if (obj->getType() == ObjectType::SWITCH_ON) {
                    totalSwitches++;
                    activeSwitches++;
                }
                
                if (!addObject(obj)) delete obj;
            }
        }
    }
}

//////////////////////////////////////////    setDoorRequirements      //////////////////////////////////////////

// Set key and switch requirements for a door
void Room::setDoorRequirements(int doorId, int keys, int switches) {
    if (doorId >= 0 && doorId < MAX_DOORS) {
        doorReqs[doorId].doorId = doorId;
        doorReqs[doorId].requiredKeys = keys;
        doorReqs[doorId].requiredSwitches = switches;
        doorReqs[doorId].isUnlocked = false;
    }
}

//////////////////////////////////////////           draw              //////////////////////////////////////////

// Draw room: base layout + modifications + darkness
void Room::draw() {
    if (baseLayout != nullptr) baseLayout->draw();
    
    for (int i = 0; i < modCount; i++) {
        gotoxy(mods[i].x, mods[i].y);
        std::cout << mods[i].newChar;
    }
    
    drawDarkness();
    std::cout.flush();
}

//////////////////////////////////////////        drawDarkness         //////////////////////////////////////////

// Draw darkness overlay for dark zones
void Room::drawDarkness() {
    if (darkZoneCount == 0) return;
    
    for (int y = 0; y < MAX_Y_INGAME; y++) {
        for (int x = 0; x < MAX_X; x++) {
            if (!isInDarkZone(x, y)) continue;
            
            gotoxy(x, y);
            if (visibilityMap[y][x]) std::cout << getCharAt(x, y);
            else std::cout << ' ';
        }
    }
    std::cout.flush();
}

//////////////////////////////////////////         getCharAt           //////////////////////////////////////////

// Get character at position (checks mods first, then base)
char Room::getCharAt(int x, int y) const {
    for (int i = 0; i < modCount; i++) {
        if (mods[i].x == x && mods[i].y == y) return mods[i].newChar;
    }
    
    if (baseLayout != nullptr) return baseLayout->getCharAt(x, y);
    return 'W';
}

//////////////////////////////////////////         setCharAt           //////////////////////////////////////////

// Set/modify character at position
void Room::setCharAt(int x, int y, char c) {
    for (int i = 0; i < modCount; i++) {
        if (mods[i].x == x && mods[i].y == y) {
            mods[i].newChar = c;
            return;
        }
    }
    
    if (modCount < RoomLimits::MAX_MODS) {
        mods[modCount] = Modification(x, y, c);
        modCount++;
    }
}

//////////////////////////////////////////         resetMods           //////////////////////////////////////////

void Room::resetMods() {
    modCount = 0;
}

//////////////////////////////////////////        getObjectAt          //////////////////////////////////////////

// Get object at position using polymorphism
GameObject* Room::getObjectAt(int x, int y) {
    for (int i = 0; i < objectCount; i++) {
        if (objects[i] != nullptr && objects[i]->isActive() &&
            objects[i]->getX() == x && objects[i]->getY() == y) {
            return objects[i];
        }
    }
    return nullptr;
}

const GameObject* Room::getObjectAt(int x, int y) const {
    for (int i = 0; i < objectCount; i++) {
        if (objects[i] != nullptr && objects[i]->isActive() &&
            objects[i]->getX() == x && objects[i]->getY() == y) {
            return objects[i];
        }
    }
    return nullptr;
}

//////////////////////////////////////////         addObject           //////////////////////////////////////////

// Add object to room (room takes ownership)
bool Room::addObject(GameObject* obj) {
    if (obj == nullptr || objectCount >= RoomLimits::MAX_OBJECTS) return false;
    
    objects[objectCount] = obj;
    objectCount++;
    setCharAt(obj->getX(), obj->getY(), obj->getSprite());
    
    return true;
}

//////////////////////////////////////////        removeObject         //////////////////////////////////////////

// Remove object at index
void Room::removeObject(int index) {
    if (index < 0 || index >= objectCount) return;
    
    GameObject* obj = objects[index];
    if (obj != nullptr) {
        setCharAt(obj->getX(), obj->getY(), ' ');
        delete obj;
        objects[index] = nullptr;
    }
}

//////////////////////////////////////////       removeObjectAt        //////////////////////////////////////////

// Remove object at position
void Room::removeObjectAt(int x, int y) {
    for (int i = 0; i < objectCount; i++) {
        if (objects[i] != nullptr && objects[i]->getX() == x && objects[i]->getY() == y) {
            removeObject(i);
            return;
        }
    }
}

//////////////////////////////////////////          getDoors           //////////////////////////////////////////

// Get all doors in the room
std::vector<Door*> Room::getDoors() {
    std::vector<Door*> doors;
    for (int i = 0; i < objectCount; i++) {
        if (objects[i] != nullptr && objects[i]->getType() == ObjectType::DOOR) {
            doors.push_back(static_cast<Door*>(objects[i]));
        }
    }
    return doors;
}

//////////////////////////////////////////        getSwitches          //////////////////////////////////////////

// Get all switches in the room
std::vector<Switch*> Room::getSwitches() {
    std::vector<Switch*> switches;
    for (int i = 0; i < objectCount; i++) {
        if (objects[i] != nullptr &&
            (objects[i]->getType() == ObjectType::SWITCH_ON ||
             objects[i]->getType() == ObjectType::SWITCH_OFF)) {
            switches.push_back(static_cast<Switch*>(objects[i]));
        }
    }
    return switches;
}

//////////////////////////////////////////         isBlocked           //////////////////////////////////////////

// Check if position is blocked
bool Room::isBlocked(int x, int y) {
    char c = getCharAt(x, y);
    if (c == 'W' || c == '=') return true;
    
    GameObject* obj = getObjectAt(x, y);
    if (obj != nullptr) return obj->isBlocking();
    
    if (c == '*') return true;
    return false;
}

//////////////////////////////////////////      hasLineOfSight         //////////////////////////////////////////

// Check line of sight using Bresenham's algorithm
bool Room::hasLineOfSight(int x1, int y1, int x2, int y2) {
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;
    
    int x = x1, y = y1;
    
    while (x != x2 || y != y2) {
        if (x != x1 || y != y1) {
            char c = getCharAt(x, y);
            if (c == 'W' || (c >= '0' && c <= '9')) return false;
        }
        
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x += sx; }
        if (e2 < dx) { err += dx; y += sy; }
    }
    
    return true;
}

//////////////////////////////////////////     updatePuzzleState       //////////////////////////////////////////

// Update puzzle state - check switches and remove obstacles if complete
void Room::updatePuzzleState() {
    activeSwitches = countActiveSwitches();
    
    if (activeSwitches >= totalSwitches && totalSwitches > 0 && !completed) {
        completed = true;
        
        // Remove obstacles linked to switches
        for (int i = 0; i < objectCount; i++) {
            if (objects[i] != nullptr && objects[i]->getType() == ObjectType::OBSTACLE) {
                Obstacle* obs = dynamic_cast<Obstacle*>(objects[i]);
                if (obs != nullptr && obs->isRemovedBySwitch()) {
                    setCharAt(obs->getX(), obs->getY(), ' ');
                    gotoxy(obs->getX(), obs->getY());
                    std::cout << ' ';
                    obs->setActive(false);
                }
            }
        }
        std::cout.flush();
    }
}

//////////////////////////////////////////    countActiveSwitches      //////////////////////////////////////////

int Room::countActiveSwitches() const {
    int count = 0;
    for (int i = 0; i < objectCount; i++) {
        if (objects[i] != nullptr && objects[i]->getType() == ObjectType::SWITCH_ON) count++;
    }
    return count;
}

//////////////////////////////////////////        canOpenDoor          //////////////////////////////////////////

// Check if door requirements are met
bool Room::canOpenDoor(int doorId, int player1Keys, int player2Keys) const {
    if (doorId < 0 || doorId >= MAX_DOORS) return false;
    
    const DoorRequirements& req = doorReqs[doorId];
    
    if (req.isUnlocked) return true;
    if (req.requiredSwitches > 0 && activeSwitches < req.requiredSwitches) return false;
    if (req.requiredKeys > 0 && (player1Keys < req.requiredKeys || player2Keys < req.requiredKeys)) return false;
    
    return true;
}

//////////////////////////////////////////        getDoorIdAt          //////////////////////////////////////////

int Room::getDoorIdAt(int x, int y) const {
    char c = getCharAt(x, y);
    if (c >= '0' && c <= '9') return c - '0';
    return -1;
}

//////////////////////////////////////////        unlockDoor           //////////////////////////////////////////

void Room::unlockDoor(int doorId) {
    if (doorId >= 0 && doorId < MAX_DOORS) doorReqs[doorId].isUnlocked = true;
}

//////////////////////////////////////////      isDoorUnlocked         //////////////////////////////////////////

bool Room::isDoorUnlocked(int doorId) const {
    if (doorId >= 0 && doorId < MAX_DOORS) return doorReqs[doorId].isUnlocked;
    return false;
}

//////////////////////////////////////////        addDarkZone          //////////////////////////////////////////

void Room::addDarkZone(int x1, int y1, int x2, int y2) {
    if (darkZoneCount >= RoomLimits::MAX_DARK_ZONES) return;
    darkZones[darkZoneCount] = DarkZone(x1, y1, x2, y2);
    darkZoneCount++;
}

//////////////////////////////////////////       clearDarkZones        //////////////////////////////////////////

void Room::clearDarkZones() {
    darkZoneCount = 0;
    initVisibility();
}

//////////////////////////////////////////       isInDarkZone          //////////////////////////////////////////

bool Room::isInDarkZone(int x, int y) const {
    for (int i = 0; i < darkZoneCount; i++) {
        if (darkZones[i].contains(x, y)) return true;
    }
    return false;
}

//////////////////////////////////////////      updateVisibility       //////////////////////////////////////////

// Update visibility based on player torches
void Room::updateVisibility(Player* p1, Player* p2) {
    if (darkZoneCount == 0) return;
    
    initVisibility();
    
    // Dark zones make areas invisible
    for (int i = 0; i < darkZoneCount; i++) {
        for (int y = darkZones[i].y1; y <= darkZones[i].y2; y++) {
            for (int x = darkZones[i].x1; x <= darkZones[i].x2; x++) {
                if (x >= 0 && x < MAX_X && y >= 0 && y < MAX_Y) visibilityMap[y][x] = false;
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

// Light circular area around center
void Room::lightRadius(int centerX, int centerY, int radius) {
    for (int dy = -radius; dy <= radius; dy++) {
        for (int dx = -radius; dx <= radius; dx++) {
            int x = centerX + dx;
            int y = centerY + dy;
            
            if (x < 0 || x >= MAX_X || y < 0 || y >= MAX_Y_INGAME) continue;
            
            double distance = sqrt(dx * dx + dy * dy);
            if (distance > radius) continue;
            
            visibilityMap[y][x] = true;
        }
    }
}

//////////////////////////////////////////         isVisible           //////////////////////////////////////////

bool Room::isVisible(int x, int y) const {
    if (x < 0 || x >= MAX_X || y < 0 || y >= MAX_Y) return false;
    return visibilityMap[y][x];
}

//////////////////////////////////////////        explodeBomb          //////////////////////////////////////////

// Process bomb explosion - destroy objects, check player hits
ExplosionResult Room::explodeBomb(int centerX, int centerY, int radius, Player* p1, Player* p2) {
    ExplosionResult result;
    
    for (int dy = -radius; dy <= radius; dy++) {
        for (int dx = -radius; dx <= radius; dx++) {
            int x = centerX + dx;
            int y = centerY + dy;
            
            if (x < 0 || x >= MAX_X || y < 0 || y >= MAX_Y_INGAME) continue;
            
            double distance = sqrt(dx * dx + dy * dy);
            if (distance > radius) continue;
            if (!hasLineOfSight(centerX, centerY, x, y)) continue;
            
            char c = getCharAt(x, y);
            if (c == 'W' || (c >= '0' && c <= '9')) continue;  // Skip walls and doors
            
            // Check player hits
            if (p1 && p1->getX() == x && p1->getY() == y) result.player1Hit = true;
            if (p2 && p2->getX() == x && p2->getY() == y) result.player2Hit = true;
            
            // Process objects using polymorphic onExplosion()
            GameObject* obj = getObjectAt(x, y);
            if (obj != nullptr && obj->isActive()) {
                if (obj->getType() == ObjectType::KEY) result.keyDestroyed = true;
                
                if (obj->onExplosion()) {
                    setCharAt(x, y, ' ');
                    gotoxy(x, y);
                    std::cout << ' ';
                    obj->setActive(false);
                    result.objectsDestroyed++;
                }
            }
            else if (c == '=') {
                setCharAt(x, y, ' ');
                gotoxy(x, y);
                std::cout << ' ';
                result.objectsDestroyed++;
            }
            else if (c != ' ' && c != 'W' && !(c >= '0' && c <= '9')) {
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

bool Room::handleBombDrop(Player& player) {
    if (!player.hasBomb()) {
        player.dropItem(this);
        return;
    }
    
    if (bomb.active) return;  // Only one bomb at a time
    
    Point dropPos = player.dropItem(this);
    
    if (dropPos.x >= 0 && dropPos.y >= 0) {
        bomb.x = dropPos.x;
        bomb.y = dropPos.y;
        bomb.fuseTimer = BombConfig::FUSE_TIME;
        bomb.active = true;
    }
}

//////////////////////////////////////////        updateBomb          //////////////////////////////////////////

bool Room::updateBomb(Player* p1, Player* p2) {
    
    if (!bomb.active) { return false; }
    
    bomb.decrementFuse();
    // Blink effect
    gotoxy(bomb.x, bomb.y);
    if (bomb.fuseTimer % BombConfig::BLINK_RATE < 5) std::cout << '@';
    else std::cout << '*';
    std::cout.flush();

    // Explode
    if (bomb.isReadyToExplode()) {
        removeObjectAt(bomb.x, bomb.y);
        setCharAt(bomb.x, bomb.y, ' ');
        gotoxy(bomb.x, bomb.y);
        std::cout << ' ' << std::flush;
        
        ExplosionResult result = explodeBomb(
            bomb.x, bomb.y, BombConfig::RADIUS,
            p1, p2
        );
        
        p1->draw();
        p2->draw();
        
        if (result.keyDestroyed || result.player1Hit || result.player2Hit) {
            return true;
        }
        
        return false;
    }
}
