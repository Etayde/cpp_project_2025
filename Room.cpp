#include "Room.h"
#include "Player.h"
#include <cmath>

void Room::initFromLayout(const Screen* layout) {
    baseLayout = layout;
    modCount = 0;  // Reset modifications
    loadObjects();
}

char Room::getCharAt(int x, int y) const {
    // First check if there's a modification at this position
    for (int i = 0; i < modCount; i++) {
        if (mods[i].x == x && mods[i].y == y) {
            return mods[i].newChar;
        }
    }
    // If no modification, return from base layout
    if (baseLayout != nullptr) {
        return baseLayout->getCharAt(x, y);
    }
    return 'W';  // Default to wall if no layout
}

void Room::setCharAt(int x, int y, char c) {
    // Check if we already have a modification at this position
    for (int i = 0; i < modCount; i++) {
        if (mods[i].x == x && mods[i].y == y) {
            mods[i].newChar = c;
            return;
        }
    }
    // Add new modification
    if (modCount < MAX_MODS) {
        mods[modCount] = Modification(x, y, c);
        modCount++;
    }
}

void Room::resetMods() {
    modCount = 0;
}

void Room::draw() {
    // First draw the base layout
    if (baseLayout != nullptr) {
        baseLayout->draw();
    }
    
    // Then draw all modifications on top
    for (int i = 0; i < modCount; i++) {
        gotoxy(mods[i].x, mods[i].y);
        std::cout << mods[i].newChar;
    }
    
    // Finally draw darkness overlay
    drawDarkness();
    
    std::cout.flush();
}

void Room::drawDarkness() {
    if (darkZoneCount == 0) return;  // No dark zones, nothing to do
    
    for (int y = 0; y < MAX_Y_INGAME; y++) {
        for (int x = 0; x < MAX_X; x++) {
            // Only process cells in dark zones
            if (!isInDarkZone(x, y)) continue;
            
            gotoxy(x, y);
            if (visibilityMap[y][x]) {
                // Visible - draw the actual content
                std::cout << getCharAt(x, y);
            } else {
                // Not visible - draw darkness
                std::cout << ' ';
            }
        }
    }
    std::cout.flush();
}

// Dark zones management
void Room::addDarkZone(int x1, int y1, int x2, int y2) {
    if (darkZoneCount >= MAX_DARK_ZONES) return;
    darkZones[darkZoneCount] = DarkZone(x1, y1, x2, y2);
    darkZoneCount++;
}

void Room::clearDarkZones() {
    darkZoneCount = 0;
    // Reset visibility to all visible
    for (int y = 0; y < MAX_Y; y++)
        for (int x = 0; x < MAX_X; x++)
            visibilityMap[y][x] = true;
}

bool Room::isInDarkZone(int x, int y) const {
    for (int i = 0; i < darkZoneCount; i++) {
        if (x >= darkZones[i].x1 && x <= darkZones[i].x2 &&
            y >= darkZones[i].y1 && y <= darkZones[i].y2) {
            return true;
        }
    }
    return false;
}

// Visibility system
void Room::updateVisibility(Player* p1, Player* p2) {
    if (darkZoneCount == 0) return;  // No dark zones, everything visible
    
    // Step 1: Everything is visible by default
    for (int y = 0; y < MAX_Y; y++)
        for (int x = 0; x < MAX_X; x++)
            visibilityMap[y][x] = true;
    
    // Step 2: Dark zones make areas invisible
    for (int i = 0; i < darkZoneCount; i++) {
        for (int y = darkZones[i].y1; y <= darkZones[i].y2; y++) {
            for (int x = darkZones[i].x1; x <= darkZones[i].x2; x++) {
                if (x >= 0 && x < MAX_X && y >= 0 && y < MAX_Y) {
                    visibilityMap[y][x] = false;
                }
            }
        }
    }
    
    // Step 3: If player has torch and is inside dark zone, light around them
    if (p1 != nullptr && p1->hasTorch() && isInDarkZone(p1->getX(), p1->getY())) {
        lightRadius(p1->getX(), p1->getY(), TORCH_RADIUS);
    }
    if (p2 != nullptr && p2->hasTorch() && isInDarkZone(p2->getX(), p2->getY())) {
        lightRadius(p2->getX(), p2->getY(), TORCH_RADIUS);
    }
}

void Room::lightRadius(int centerX, int centerY, int radius) {
    for (int dy = -radius; dy <= radius; dy++) {
        for (int dx = -radius; dx <= radius; dx++) {
            int x = centerX + dx;
            int y = centerY + dy;
            
            // Check bounds
            if (x < 0 || x >= MAX_X || y < 0 || y >= MAX_Y_INGAME) continue;
            
            // Check if within circular radius
            double distance = sqrt(dx * dx + dy * dy);
            if (distance > radius) continue;
            
            // Light this cell
            visibilityMap[y][x] = true;
        }
    }
}

bool Room::isVisible(int x, int y) const {
    if (x < 0 || x >= MAX_X || y < 0 || y >= MAX_Y) return false;
    return visibilityMap[y][x];
}

void Room::loadObjects() {
    objectCount = 0;
    activeSwitches = 0;
    requiredSwitches = 0;

    if (baseLayout == nullptr) return;

    for (int y = 0; y < MAX_Y_INGAME; y++) {
        for (int x = 0; x < MAX_X; x++) {
            if (objectCount >= MAX_OBJECTS) return;
            
            char ch = baseLayout->getCharAt(x, y);
            ObjectType type = ObjectType::AIR;
            bool shouldAdd = false;

            switch (ch) {
                case '\\':
                    type = ObjectType::SWITCH_OFF;
                    shouldAdd = true;
                    requiredSwitches++;
                    break;
                case '/':
                    type = ObjectType::SWITCH_ON;
                    shouldAdd = true;
                    activeSwitches++;
                    break;
                case 'K':
                    type = ObjectType::KEY;
                    shouldAdd = true;
                    break;
                case '@':
                    type = ObjectType::BOMB;
                    shouldAdd = true;
                    break;
                case '!':
                    type = ObjectType::TORCH;
                    shouldAdd = true;
                    break;
                case '#':
                    type = ObjectType::SPRING;
                    shouldAdd = true;
                    break;
                case '*':
                    type = ObjectType::OBSTACLE;
                    shouldAdd = true;
                    break;
                case '=':
                    type = ObjectType::BREAKABLE_WALL;
                    shouldAdd = true;
                    break;
                case '?':
                    type = ObjectType::RIDDLE;
                    shouldAdd = true;
                    break;
                case '0': case '1': case '2': case '3': case '4':
                case '5': case '6': case '7': case '8': case '9':
                    type = ObjectType::DOOR;
                    shouldAdd = true;
                    break;
                default:
                    break;
            }

            if (shouldAdd) {
                Point pos(x, y, 0, 0, ch);
                objects[objectCount] = Object(type, pos);
                objects[objectCount].setSprite(ch);
                
                if (type == ObjectType::SWITCH_OFF || type == ObjectType::SWITCH_ON) {
                    objects[objectCount].setLinkedDoor(0);
                }
                
                objectCount++;
            }
        }
    }
}

Object* Room::getObjectAt(int x, int y) {
    for (int i = 0; i < objectCount; i++) {
        if (objects[i].pos.x == x && objects[i].pos.y == y &&
            objects[i].type != ObjectType::AIR) {
            return &objects[i];
        }
    }
    return nullptr;
}

bool Room::addObject(const Object& obj) {
    if (objectCount >= MAX_OBJECTS) return false;
    objects[objectCount] = obj;
    objectCount++;
    // Update the mods (not the base layout)
    setCharAt(obj.pos.x, obj.pos.y, obj.sprite);
    return true;
}

void Room::removeObject(int index) {
    if (index < 0 || index >= objectCount) return;
    
    // Add modification to show empty space
    setCharAt(objects[index].pos.x, objects[index].pos.y, ' ');
    
    // Mark object as AIR
    objects[index].type = ObjectType::AIR;
    objects[index].sprite = ' ';
    objects[index].pos = Point(-1, -1);
}

void Room::removeObjectAt(int x, int y) {
    for (int i = 0; i < objectCount; i++) {
        if (objects[i].pos.x == x && objects[i].pos.y == y &&
            objects[i].type != ObjectType::AIR) {
            removeObject(i);
            return;
        }
    }
}

bool Room::isBlocked(int x, int y) {
    char c = getCharAt(x, y);
    
    // Walls always block
    if (c == 'W') return true;
    
    // Breakable walls also block
    if (c == '=') return true;
    
    // Check objects
    Object* obj = getObjectAt(x, y);
    if (obj && obj->isFilled()) {
        return true;
    }
    
    // Obstacles block
    if (c == '*') return true;
    
    return false;
}

void Room::updatePuzzleState() {
    // Count active switches
    activeSwitches = 0;
    for (int i = 0; i < objectCount; i++) {
        if (objects[i].type == ObjectType::SWITCH_ON) {
            activeSwitches++;
        }
    }
    
    // Check if puzzle is complete (all switches activated)
    if (activeSwitches >= requiredSwitches && requiredSwitches > 0 && !completed) {
        completed = true;
        
        // Remove obstacles when all switches are activated
        for (int i = 0; i < objectCount; i++) {
            if (objects[i].type == ObjectType::OBSTACLE) {
                // Add modification to clear the obstacle
                setCharAt(objects[i].pos.x, objects[i].pos.y, ' ');
                // Also update on screen immediately
                gotoxy(objects[i].pos.x, objects[i].pos.y);
                std::cout << ' ';
                
                objects[i].type = ObjectType::AIR;
                objects[i].sprite = ' ';
                objects[i].setFilled(false);
            }
        }
        std::cout.flush();
    }
}

// Check if door is unlocked (either via switches or no switches in room)
bool Room::isDoorUnlocked() const {
    // If there are switches, door is unlocked only when all are activated
    if (requiredSwitches > 0) {
        return completed;
    }
    // If no switches, door requires keys (handled in Game)
    return false;
}

int Room::getDoorIdAt(int x, int y) const {
    char c = getCharAt(x, y);
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    return -1;
}

bool Room::hasLineOfSight(int x1, int y1, int x2, int y2) {
    // Bresenham's line algorithm
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;
    
    int x = x1;
    int y = y1;
    
    while (x != x2 || y != y2) {
        if (x != x1 || y != y1) {
            char c = getCharAt(x, y);
            if (c == 'W' || (c >= '0' && c <= '9')) {
                return false;
            }
        }
        
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x += sx;
        }
        if (e2 < dx) {
            err += dx;
            y += sy;
        }
    }
    
    return true;
}

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
            
            // Skip indestructible walls (W) and doors (0-9)
            if (c == 'W' || (c >= '0' && c <= '9')) continue;
            
            // Check players
            if (p1 && p1->getX() == x && p1->getY() == y) {
                result.player1Hit = true;
            }
            if (p2 && p2->getX() == x && p2->getY() == y) {
                result.player2Hit = true;
            }
            
            // Destroy objects
            Object* obj = getObjectAt(x, y);
            if (obj != nullptr && obj->type != ObjectType::AIR) {
                if (obj->type == ObjectType::KEY) {
                    result.keyDestroyed = true;
                }
                
                // Destroy everything except doors and indestructible walls
                if (obj->type != ObjectType::DOOR && obj->type != ObjectType::WALL) {
                    setCharAt(x, y, ' ');
                    gotoxy(x, y);
                    std::cout << ' ';
                    obj->type = ObjectType::AIR;
                    obj->sprite = ' ';
                    result.objectsDestroyed++;
                }
            } else if (c == '=') {
                // Breakable wall (not in objects array, just a character)
                setCharAt(x, y, ' ');
                gotoxy(x, y);
                std::cout << ' ';
                result.objectsDestroyed++;
            } else if (c != ' ' && c != 'W' && !(c >= '0' && c <= '9')) {
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
