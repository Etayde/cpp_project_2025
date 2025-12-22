#pragma once

////////////////////////////////////////       INCLUDES & FORWARDS       //////////////////////////////////////////

#include "GameObject.h"

////////////////////////////////////////       InteractableObject      //////////////////////////////////////////

// Base class for objects that can be interacted with (doors, switches, riddles)

class InteractableObject : public GameObject
{
protected:
    int linkedDoorId;

public:
    InteractableObject() : GameObject(), linkedDoorId(-1) {}
    InteractableObject(const Point &pos, char spr, ObjectType t)
        : GameObject(pos, spr, t), linkedDoorId(-1) {}

    bool isPickable() const override { return false; }
    bool isInteractable() const override { return true; }

    int getLinkedDoorId() const { return linkedDoorId; }
    void setLinkedDoorId(int id) { linkedDoorId = id; }
};