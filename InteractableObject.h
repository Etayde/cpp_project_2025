#pragma once

#include "GameObject.h"

//////////////////////////////////////////       InteractableObject      //////////////////////////////////////////

// Base class for objects that can be interacted with (doors, switches)

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

//////////////////////////////////////////          Riddle            //////////////////////////////////////////

// Puzzle marker placeholder
// Not used yet - reserved for future puzzle implementation
class Riddle : public InteractableObject
{
public:
    Riddle() : InteractableObject()
    {
        sprite = '?';
        type = ObjectType::RIDDLE;
    }

    Riddle(const Point &pos) : InteractableObject(pos, '?', ObjectType::RIDDLE) {}

    GameObject *clone() const override { return new Riddle(*this); }
    const char *getName() const override { return "Riddle"; }

    bool isBlocking() const override { return false; }
    bool onExplosion() override { return true; }
};