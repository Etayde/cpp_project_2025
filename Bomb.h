#pragma once

////////////////////////////////////////      INCLUDES & FORWARDS       //////////////////////////////////////////

#include "PickableObject.h"

class Room;
class Player;

struct ExplosionResult;

//////////////////////////////////////////         BombState          //////////////////////////////////////////

// Tracks the lifecycle state of a bomb
enum class BombState
{
    PLACED,
    IN_INVENTORY,
    TICKING,
    EXPLODED
};

//////////////////////////////////////////           Bomb            //////////////////////////////////////////

// An explosive that can be picked up by players (@)
class Bomb : public PickableObject
{
private:
    BombState state;
    int fuseTimer;
    int blinkCounter;
    Room *currentRoom;

    static const int FUSE_TIME = 50;
    static const int EXPLOSION_RADIUS = 5;
    static const int BLINK_RATE = 10;

    ExplosionResult explode(Player *p1, Player *p2);

public:
    Bomb() : PickableObject(), state(BombState::PLACED), fuseTimer(0),
             blinkCounter(0), currentRoom(nullptr)
    {
        sprite = '@';
        type = ObjectType::BOMB;
    }

    Bomb(const Point &pos) : PickableObject(pos, '@', ObjectType::BOMB),
                             state(BombState::PLACED), fuseTimer(0),
                             blinkCounter(0), currentRoom(nullptr) {}

    GameObject *clone() const override { return new Bomb(*this); }
    const char *getName() const override { return "Bomb"; }
    void draw() const override;
    bool isPickable() const override;
    bool isAlwaysVisible() const override { return state == BombState::TICKING; }

    void activate(Room *room);
    using GameObject::update;
    ExplosionResult update(Player *p1, Player *p2);
    BombState getState() const { return state; }
};