#pragma once

#include "GameObject.h"

// Base class for items that can be picked up (keys, bombs, torches)
class PickableObject : public GameObject
{
public:
    PickableObject() : GameObject() {}
    PickableObject(const Point &pos, char spr, ObjectType t) : GameObject(pos, spr, t) {}

    bool isBlocking() const override { return false; }
    bool isPickable() const override { return true; }
    bool isInteractable() const override { return false; }
    bool onExplosion() override { return true; } // Destroyed by bombs
};