#pragma once

#include "PickableObject.h"

//////////////////////////////////////////           Bomb            //////////////////////////////////////////

// An explosive that can be picked up by players
// Once dropped, the bomb's state is tracked by RoomBomb struct in Room
class Bomb : public PickableObject
{
public:
    static const int DEFAULT_FUSE_TIME = 50; // ~5 seconds at 10 FPS
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