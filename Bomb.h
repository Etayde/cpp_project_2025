#pragma once

////////////////////////////////////////      INCLUDES & FORWARDS       //////////////////////////////////////////

#include "PickableObject.h"

//////////////////////////////////////////           Bomb            //////////////////////////////////////////

// An explosive that can be picked up by players (@)
class Bomb : public PickableObject
{
public:
    static const int DEFAULT_FUSE_TIME = 50;
    static const int EXPLOSION_RADIUS = 5;

    Bomb() : PickableObject()
    {
        sprite = '@';
        type = ObjectType::BOMB;
    }

    Bomb(const Point &pos) : PickableObject(pos, '@', ObjectType::BOMB) {}

    GameObject *clone() const override { return new Bomb(*this); }
    const char *getName() const override { return "Bomb"; }
};