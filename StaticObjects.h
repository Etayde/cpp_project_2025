#pragma once

////////////////////////////////////////      INCLUDES & FORWARDS       //////////////////////////////////////////

#include "GameObject.h"

//////////////////////////////////////////       StaticObject         //////////////////////////////////////////

// Base class for static objects
class StaticObject : public GameObject
{
public:
    StaticObject() : GameObject() {}
    StaticObject(const Point &pos, char spr, ObjectType t) : GameObject(pos, spr, t) {}

    bool isPickable() const override { return false; }
    bool isInteractable() const override { return false; }
};

//////////////////////////////////////////           Wall             //////////////////////////////////////////

// Indestructible wall (W)
class Wall : public StaticObject
{
public:
    Wall() : StaticObject()
    {
        sprite = 'W';
        type = ObjectType::WALL;
    }

    Wall(const Point &pos) : StaticObject(pos, 'W', ObjectType::WALL) {}

    GameObject *clone() const override { return new Wall(*this); }
    const char *getName() const override { return "Wall"; }

    bool isBlocking() const override { return true; }
    bool onExplosion() override { return false; }
};

//////////////////////////////////////////      BreakableWall         //////////////////////////////////////////

// Destructible wall (w)
class BreakableWall : public StaticObject
{
public:
    BreakableWall() : StaticObject()
    {
        sprite = 'w';
        type = ObjectType::BREAKABLE_WALL;
    }

    BreakableWall(const Point &pos) : StaticObject(pos, 'w', ObjectType::BREAKABLE_WALL) {}

    GameObject *clone() const override { return new BreakableWall(*this); }
    const char *getName() const override { return "Breakable Wall"; }

    bool isBlocking() const override { return true; }
    bool onExplosion() override { return true; }
};

//////////////////////////////////////////         SwitchWall            //////////////////////////////////////////

// A wall that can be removed by activating switches (Z)
class SwitchWall : public StaticObject
{
    bool removedBySwitch;

public:
    SwitchWall() : StaticObject(), removedBySwitch(true)
    {
        sprite = 'Z';
        type = ObjectType::SWITCH_WALL;
    }

    SwitchWall(const Point &pos, bool removable = true)
        : StaticObject(pos, 'Z', ObjectType::SWITCH_WALL), removedBySwitch(removable) {}

    GameObject *clone() const override { return new SwitchWall(*this); }
    const char *getName() const override { return "Switch Wall"; }

    bool isBlocking() const override { return true; }
    bool onExplosion() override { return true; }

    bool isRemovedBySwitch() const { return removedBySwitch; }
    void setRemovedBySwitch(bool removable) { removedBySwitch = removable; }
};

class Air : public StaticObject
{
public:
    Air() : StaticObject()
    {
        sprite = ' ';
        type = ObjectType::AIR;
    }

    Air(const Point &pos) : StaticObject(pos, ' ', ObjectType::AIR) {}

    GameObject *clone() const override { return new Air(*this); }
    const char *getName() const override { return "Air"; }

    bool isBlocking() const override { return false; }
    bool onExplosion() override { return false; }
};