#include "Bomb.h"
#include "Room.h"
#include "Player.h"
#include "Console.h"
#include <cmath>
#include <iostream>

//////////////////////////////////////////        activate           //////////////////////////////////////////

void Bomb::activate(Room *room)
{
    currentRoom = room;
    state = BombState::TICKING;
    fuseTimer = FUSE_TIME;
    blinkCounter = 0;
}

//////////////////////////////////////////         update            //////////////////////////////////////////

ExplosionResult Bomb::update(Player *p1, Player *p2)
{
    // Only update when ticking
    if (state != BombState::TICKING)
        return ExplosionResult();

    blinkCounter++;
    fuseTimer--;

    if (fuseTimer <= 0)
    {
        ExplosionResult result = explode(p1, p2);
        state = BombState::EXPLODED;
        active = false;
        return result;
    }

    return ExplosionResult();
}

//////////////////////////////////////////         explode           //////////////////////////////////////////

// This method, along with other parts of the bomb's logic methods,
// were made with AI assistance in the previous excercise but were in different classes.
// For better OOP oriented design, I moved the explode logic into the Bomb class itself
// to support better encapsulation and self containment.
ExplosionResult Bomb::explode(Player *p1, Player *p2)
{
    // Initialize explosion result
    ExplosionResult result;

    if (!currentRoom)
        return result;

    int centerX = position.x;
    int centerY = position.y;

    // Remove bomb from room
    currentRoom->removeObjectAt(centerX, centerY);
    currentRoom->setCharAt(centerX, centerY, ' ');
    gotoxy(centerX, centerY);
    std::cout << ' ';


    // Process explosion in radius
    for (int dy = -EXPLOSION_RADIUS; dy <= EXPLOSION_RADIUS; dy++)
    {
        for (int dx = -EXPLOSION_RADIUS; dx <= EXPLOSION_RADIUS; dx++)
        {
            int x = centerX + dx;
            int y = centerY + dy;

            // Check bounds
            if (x < 0 || x >= MAX_X || y < 0 || y >= MAX_Y_INGAME)
                continue;

            // Check distance
            double distance = sqrt(dx * dx + dy * dy);
            if (distance > EXPLOSION_RADIUS)
                continue;

            // Check line of sight
            if (!currentRoom->hasLineOfSight(centerX, centerY, x, y))
                continue;

            // Get what's at this position
            ObjectType type = currentRoom->getObjectTypeAt(x, y);

            // Skip unbreakable walls
            if (type == ObjectType::WALL)
                continue;

            // Skip doors
            if (static_cast<char>(type) >= '0' && static_cast<char>(type) <= '9')
                continue;

            // Check player hits
            if (p1 && p1->getX() == x && p1->getY() == y)
                result.player1Hit = true;
            if (p2 && p2->getX() == x && p2->getY() == y)
                result.player2Hit = true;

            // Process objects using polymorphic onExplosion()
            GameObject *obj = currentRoom->getObjectAt(x, y);
            if (obj != nullptr && obj->isActive())
            {
                if (obj->getType() == ObjectType::KEY)
                    result.keyDestroyed = true;
                if (obj->getType() == ObjectType::SWITCH_OFF)
                    result.switchesDestroyed++;
                if (obj->getType() == ObjectType::SWITCH_ON)
                    result.switchesDestroyed++;

                if (obj->onExplosion())
                {
                    currentRoom->setCharAt(x, y, ' ');
                    gotoxy(x, y);
                    std::cout << ' ';
                    obj->setActive(false);
                    result.objectsDestroyed++;
                }
            }
            else if (type == ObjectType::BREAKABLE_WALL)
            {
                // Destroy breakable walls
                currentRoom->setCharAt(x, y, ' ');
                gotoxy(x, y);
                std::cout << ' ';
                result.objectsDestroyed++;
            }
            else if (type != ObjectType::AIR && type != ObjectType::WALL)
            {
                // Destroy other destructible characters (spring segments, etc.)
                currentRoom->setCharAt(x, y, ' ');
                gotoxy(x, y);
                std::cout << ' ';
                result.objectsDestroyed++;
            }
        }
    }

    return result;

}

//////////////////////////////////////////          draw             //////////////////////////////////////////

void Bomb::draw() const
{
    // Don't draw if inactive or in inventory
    if (!active || state == BombState::IN_INVENTORY)
        return;

    // Darkness handling: PLACED bombs invisible in dark, TICKING bombs always visible (glowing)
    if (state == BombState::PLACED && currentRoom && !currentRoom->isVisible(position.x, position.y))
        return;

    gotoxy(position.x, position.y);

    if (state == BombState::TICKING)
    {
        // Blinking animation
        if ((blinkCounter % BLINK_RATE) < 5)
            std::cout << '@';
        else
            std::cout << '*';
    }
    else
    {
        std::cout << sprite;
    }

    std::cout.flush();

}

//////////////////////////////////////////       isPickable          //////////////////////////////////////////

bool Bomb::isPickable() const
{
    // Can only pick up bombs in PLACED state
    // Once ticking, they cannot be picked up again
    return (state == BombState::PLACED);
}
