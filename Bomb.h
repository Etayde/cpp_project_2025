#pragma once

////////////////////////////////////////      INCLUDES & FORWARDS       //////////////////////////////////////////

#include "PickableObject.h"

// Forward declarations
class Room;
class Player;

// Forward declare ExplosionResult (defined in Room.h)
struct ExplosionResult;

//////////////////////////////////////////         BombState          //////////////////////////////////////////

// Tracks the lifecycle state of a bomb
enum class BombState
{
    PLACED,       // In world, can be picked up
    IN_INVENTORY, // Held by player (not rendered)
    TICKING,      // Dropped, counting down
    EXPLODED      // Detonated, ready for cleanup
};

//////////////////////////////////////////           Bomb            //////////////////////////////////////////

// An explosive that can be picked up by players (@)
// Now fully self-contained with its own state management and explosion logic
class Bomb : public PickableObject
{
private:
    // State management
    BombState state;
    int fuseTimer;
    int blinkCounter;
    Room *currentRoom; // Reference to room for explosion processing
    ExplosionResult *lastExplosion; // Pointer to avoid incomplete type issues

    // Constants
    static const int FUSE_TIME = 50;
    static const int EXPLOSION_RADIUS = 5;
    static const int BLINK_RATE = 10;

    // Private methods
    void explode(Player *p1, Player *p2);

public:
    // Constructors
    Bomb() : PickableObject(), state(BombState::PLACED), fuseTimer(0),
             blinkCounter(0), currentRoom(nullptr), lastExplosion(nullptr)
    {
        sprite = '@';
        type = ObjectType::BOMB;
    }

    Bomb(const Point &pos) : PickableObject(pos, '@', ObjectType::BOMB),
                             state(BombState::PLACED), fuseTimer(0),
                             blinkCounter(0), currentRoom(nullptr), lastExplosion(nullptr) {}

    // Destructor
    ~Bomb() { delete lastExplosion; }

    // GameObject overrides
    GameObject *clone() const override { return new Bomb(*this); }
    const char *getName() const override { return "Bomb"; }
    void draw() const override;
    bool isPickable() const override;

    // Bomb-specific interface
    void activate(Room *room);                  // Start countdown when dropped
    void update(Player *p1, Player *p2);        // Overload to accept players
    BombState getState() const { return state; }
    ExplosionResult getExplosionResult() const;
};