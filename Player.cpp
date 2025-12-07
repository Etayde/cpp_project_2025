#include "Player.h"
#include "Room.h"

bool Player::move(Room* room) {
    if (room == nullptr) return false;
    
    // If not moving, just redraw at current position
    if (pos.diff_x == 0 && pos.diff_y == 0) {
        draw();
        return false;
    }
    
    // Calculate next position
    int nextX = pos.x + pos.diff_x;
    int nextY = pos.y + pos.diff_y;
    
    // Check bounds
    if (nextX < 1 || nextX >= MAX_X - 1 || nextY < 1 || nextY >= MAX_Y_INGAME - 1) {
        pos.diff_x = 0;
        pos.diff_y = 0;
        draw();
        return false;
    }
    
    // Check for wall collision
    char nextChar = room->getCharAt(nextX, nextY);
    if (nextChar == 'W' || nextChar == '=') {
        pos.diff_x = 0;
        pos.diff_y = 0;
        draw();
        return false;
    }
    
    // Check for object collision/interaction
    Object* obj = room->getObjectAt(nextX, nextY);
    if (obj != nullptr) {
        // Handle different object types
        switch (obj->type) {
            case ObjectType::WALL:
            case ObjectType::BREAKABLE_WALL:
            case ObjectType::OBSTACLE:
                // Blocked, stop moving
                pos.diff_x = 0;
                pos.diff_y = 0;
                draw();
                return false;
                
            case ObjectType::SWITCH_OFF:
            case ObjectType::SWITCH_ON:
                // Toggle switch and stop
                obj->toggleSwitch();
                room->setCharAt(obj->pos.x, obj->pos.y, obj->sprite);
                gotoxy(obj->pos.x, obj->pos.y);
                std::cout << obj->sprite;
                std::cout.flush();
                room->updatePuzzleState();
                pos.diff_x = 0;
                pos.diff_y = 0;
                draw();
                return false;
                
            case ObjectType::KEY:
                // Pick up key if inventory is empty
                if (!hasItem()) {
                    pickupItem(obj);
                    room->setCharAt(nextX, nextY, ' ');
                    nextChar = ' ';  // Update nextChar so prevChar will be space
                }
                break;
            
            case ObjectType::BOMB:
                // Pick up bomb if inventory is empty
                if (!hasItem()) {
                    pickupItem(obj);
                    room->setCharAt(nextX, nextY, ' ');
                    nextChar = ' ';  // Update nextChar so prevChar will be space
                }
                break;
            
            case ObjectType::TORCH:
                // Pick up torch if inventory is empty
                if (!hasItem()) {
                    pickupItem(obj);
                    room->setCharAt(nextX, nextY, ' ');
                    nextChar = ' ';  // Update nextChar so prevChar will be space
                }
                break;
                
            case ObjectType::DOOR:
                // Mark that player is at door (game logic handles transition)
                atDoor = true;
                doorId = obj->sprite - '0';
                // Don't stop - let player stand on door
                break;
                
            case ObjectType::SPRING:
                // Boost speed (move extra space)
                break;
                
            default:
                break;
        }
    } else {
        // Not at a door anymore
        atDoor = false;
        doorId = -1;
    }
    
    // Erase from current position (restore what was there)
    gotoxy(pos.x, pos.y);
    std::cout << prevChar;
    std::cout.flush();
    
    // Store the character at the new position before we overwrite it
    if (obj != nullptr && obj->type == ObjectType::DOOR) {
        prevChar = obj->sprite;  // Remember door character
    } else {
        prevChar = (nextChar == ' ' || nextChar == sprite) ? ' ' : nextChar;
    }
    
    // Update position
    pos.x = nextX;
    pos.y = nextY;
    
    // Draw at new position
    draw();
    
    return true;
}

bool Player::pickupItem(Object* item) {
    if (item == nullptr || hasItem()) return false;
    if (!item->isPickable()) return false;
    
    // Copy item to inventory
    inventory = *item;
    
    // Clear the original item
    item->type = ObjectType::AIR;
    item->sprite = ' ';
    item->pos = Point(-1, -1);
    
    // Update inventory display
    updateInventoryDisplay();
    
    return true;
}

Point Player::dropItem(Room* room) {
    Point dropPos(-1, -1);
    if (!hasItem() || room == nullptr) return dropPos;
    
    // Find an empty spot around the player
    int checkOffsets[][2] = {{0, 1}, {0, -1}, {1, 0}, {-1, 0}, {1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
    int dropX = -1;
    int dropY = -1;
    bool found = false;
    
    for (int i = 0; i < 8 && !found; i++) {
        int testX = pos.x + checkOffsets[i][0];
        int testY = pos.y + checkOffsets[i][1];
        
        if (testX >= 1 && testX < MAX_X - 1 && testY >= 1 && testY < MAX_Y_INGAME - 1) {
            char c = room->getCharAt(testX, testY);
            Object* existing = room->getObjectAt(testX, testY);
            // Allow drop on empty space (not wall, not breakable wall, not obstacle, not door)
            if ((c == ' ' || c == '.' ) && existing == nullptr) {
                dropX = testX;
                dropY = testY;
                found = true;
            }
        }
    }
    
    if (!found) return dropPos;  // No valid drop position
    
    // Create new object at drop position
    Object droppedItem = inventory;
    droppedItem.pos = Point(dropX, dropY, 0, 0, inventory.sprite);
    
    // Add to room
    if (room->addObject(droppedItem)) {
        dropPos.x = dropX;
        dropPos.y = dropY;
        
        // Clear inventory
        inventory = Object();
        updateInventoryDisplay();
        
        // Draw dropped item
        gotoxy(dropX, dropY);
        std::cout << droppedItem.sprite;
        std::cout.flush();
    }
    
    return dropPos;
}

void Player::updateInventoryDisplay() {
    // Position for inventory display
    int invX = (playerId == 1) ? 18 : 58;
    int invY = 23;
    
    gotoxy(invX, invY);
    if (hasItem()) {
        std::cout << "[" << inventory.sprite << "]  ";
    } else {
        std::cout << "[   ]";
    }
    std::cout.flush();
}

void Player::performAction(Action action) {
    switch (action) {
        case Action::MOVE_UP:
            pos.setDirection(Direction::UP);
            break;
        case Action::MOVE_DOWN:
            pos.setDirection(Direction::DOWN);
            break;
        case Action::MOVE_LEFT:
            pos.setDirection(Direction::LEFT);
            break;
        case Action::MOVE_RIGHT:
            pos.setDirection(Direction::RIGHT);
            break;
        case Action::STAY:
            pos.setDirection(Direction::STAY);
            break;
        case Action::DROP_ITEM:
            // Will be handled by game loop
            break;
        default:
            pos.setDirection(Direction::STAY);
            break;
    }
}
