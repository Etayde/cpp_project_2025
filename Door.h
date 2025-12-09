#pragma once

#include "InteractableObject.h"

//////////////////////////////////////////           Door             //////////////////////////////////////////

// A door that requires keys and/or switches to open
// requiredKeys = keys needed PER PLAYER
// requiredSwitches = number of switches that must be ON
class Door : public InteractableObject
{
private:
    int doorId;
    int requiredKeys;
    int requiredSwitches;
    bool isOpen;
    int targetRoomId;

public:
    Door() : InteractableObject(), doorId(0), requiredKeys(1),
             requiredSwitches(0), isOpen(false), targetRoomId(-1)
    {
        sprite = '0';
        type = ObjectType::DOOR;
    }

    Door(const Point &pos, int id, int keys = 1, int switches = 0, int target = -1)
        : InteractableObject(pos, '0' + id, ObjectType::DOOR),
          doorId(id), requiredKeys(keys), requiredSwitches(switches),
          isOpen(false), targetRoomId(target) {}

    GameObject *clone() const override { return new Door(*this); }
    const char *getName() const override { return "Door"; }

    bool isBlocking() const override { return false; }
    bool onExplosion() override { return false; } // Doors survive explosions

    // Getters
    int getDoorId() const { return doorId; }
    int getRequiredKeys() const { return requiredKeys; }
    int getRequiredSwitches() const { return requiredSwitches; }
    bool getIsOpen() const { return isOpen; }
    int getTargetRoomId() const { return targetRoomId; }

    // Setters
    void setDoorId(int id)
    {
        doorId = id;
        sprite = '0' + id;
    }
    void setRequiredKeys(int keys) { requiredKeys = keys; }
    void setRequiredSwitches(int switches) { requiredSwitches = switches; }
    void setIsOpen(bool open) { isOpen = open; }
    void setTargetRoomId(int target) { targetRoomId = target; }

    // Check if door can be opened (both players need keys)
    bool canOpen(int keysAvailable, int switchesOn) const
    {
        bool keysOk = (keysAvailable >= requiredKeys * 2);
        bool switchesOk = (switchesOn >= requiredSwitches);
        return keysOk && switchesOk;
    }
};