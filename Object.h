#pragma once
#include "Point.h"
#include "Constants.h"

// Forward declarations
class Player;
class Room;

class Object {
public:
    char sprite;
    ObjectType type;
    Point pos;
    bool filled;      // Does it block the player?
    bool pickable;
    bool active;      // Is the object active (for switches, etc.)
    int linkedDoor;   // For switches: which door they control (-1 if none)

    // Default constructor - creates AIR
    Object() 
        : sprite(' '), type(ObjectType::AIR), pos(Point(-1, -1)), 
          filled(false), pickable(false), active(false), linkedDoor(-1) {}

    // Parameterized constructor
    Object(ObjectType _type, Point _pos)
        : type(_type), pos(_pos), filled(false), pickable(false), active(false), linkedDoor(-1) {
        sprite = static_cast<char>(_type);
        
        // Set properties based on type
        switch (_type) {
            case ObjectType::WALL:
                filled = true;
                break;
            case ObjectType::BREAKABLE_WALL:
                filled = true;
                break;
            case ObjectType::OBSTACLE:
                filled = true;
                break;
            case ObjectType::KEY:
                pickable = true;
                break;
            case ObjectType::TORCH:
                pickable = true;
                break;
            case ObjectType::BOMB:
                pickable = true;
                break;
            case ObjectType::DOOR:
                filled = true;
                break;
            default:
                break;
        }
    }

    // Getters
    ObjectType getType() const { return type; }
    Point getPos() const { return pos; }
    char getSprite() const { return sprite; }
    bool isFilled() const { return filled; }
    bool isPickable() const { return pickable; }
    bool isActive() const { return active; }
    int getLinkedDoor() const { return linkedDoor; }

    // Setters
    void setType(ObjectType _type) { type = _type; sprite = static_cast<char>(_type); }
    void setSprite(char c) { sprite = c; }
    void setPos(Point _pos) { pos = _pos; }
    void setFilled(bool f) { filled = f; }
    void setPickable(bool p) { pickable = p; }
    void setActive(bool a) { active = a; }
    void setLinkedDoor(int door) { linkedDoor = door; }

    // Draw the object at its position
    void draw() const {
        if (pos.x >= 0 && pos.y >= 0) {
            gotoxy(pos.x, pos.y);
            std::cout << sprite;
            std::cout.flush();
        }
    }

    // Toggle switch
    void toggleSwitch() {
        if (type == ObjectType::SWITCH_OFF) {
            type = ObjectType::SWITCH_ON;
            sprite = '/';
            active = true;
        } else if (type == ObjectType::SWITCH_ON) {
            type = ObjectType::SWITCH_OFF;
            sprite = '\\';
            active = false;
        }
    }
};
