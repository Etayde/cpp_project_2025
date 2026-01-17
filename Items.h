#pragma once

//////////////////////////////////////       INCLUDES & FORWARDS       //////////////////////////////////////////

#include "PickableObject.h"

class Room;

//////////////////////////////////////////           Key              //////////////////////////////////////////

// A key that unlocks doors (K)
class Key : public PickableObject
{
public:
    Key() : PickableObject()
    {
        sprite = 'K';
        type = ObjectType::KEY;
    }

    Key(const Point &pos) : PickableObject(pos, 'K', ObjectType::KEY) {}

    GameObject *clone() const override { return new Key(*this); }
    const char *getName() const override { return "Key"; }
};

//////////////////////////////////////////          Torch             //////////////////////////////////////////

// A light source that illuminates dark areas (!)
class Torch : public PickableObject
{
public:
    static constexpr int LIGHT_RADIUS = 2;

    Torch() : PickableObject()
    {
        sprite = '!';
        type = ObjectType::TORCH;
    }

    Torch(const Point &pos) : PickableObject(pos, '!', ObjectType::TORCH) {}

    GameObject *clone() const override { return new Torch(*this); }
    const char *getName() const override { return "Torch"; }

    void illuminate(Room *room, int playerX, int playerY) const;
};