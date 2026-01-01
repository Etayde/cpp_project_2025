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

void Bomb::update(Player *p1, Player *p2)
{
    // Only update when ticking
    if (state != BombState::TICKING)
        return;

    blinkCounter++;
    fuseTimer--;

    if (fuseTimer <= 0)
    {
        explode(p1, p2);
        state = BombState::EXPLODED;
        active = false;
    }
}

//////////////////////////////////////////         explode           //////////////////////////////////////////

void Bomb::explode(Player *p1, Player *p2)
{
    // Initialize explosion result
    delete lastExplosion;
    lastExplosion = new ExplosionResult();

    if (!currentRoom)
        return;

    int centerX = position.x;
    int centerY = position.y;

    // Remove bomb from room
    currentRoom->removeObjectAt(centerX, centerY);
    currentRoom->setCharAt(centerX, centerY, ' ');
    gotoxy(centerX, centerY);
    std::cout << ' ' << std::flush;

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

            // Get character at position
            char c = currentRoom->getCharAt(x, y);

            // Skip walls and doors
            if (c == 'W' || (c >= '0' && c <= '9'))
                continue;

            // Check player hits
            if (p1 && p1->getX() == x && p1->getY() == y)
                lastExplosion->player1Hit = true;
            if (p2 && p2->getX() == x && p2->getY() == y)
                lastExplosion->player2Hit = true;

            // Process objects using polymorphic onExplosion()
            GameObject *obj = currentRoom->getObjectAt(x, y);
            if (obj != nullptr && obj->isActive())
            {
                if (obj->getType() == ObjectType::KEY)
                    lastExplosion->keyDestroyed = true;

                if (obj->onExplosion())
                {
                    currentRoom->setCharAt(x, y, ' ');
                    gotoxy(x, y);
                    std::cout << ' ';
                    obj->setActive(false);
                    lastExplosion->objectsDestroyed++;
                }
            }
            else if (c == 'w')
            {
                // Destroy non-object destructibles (like spring segments)
                currentRoom->setCharAt(x, y, ' ');
                gotoxy(x, y);
                std::cout << ' ';
                lastExplosion->objectsDestroyed++;
            }
            else if (c != ' ' && c != 'W' && !(c >= '0' && c <= '9'))
            {
                // Destroy other destructible characters
                currentRoom->setCharAt(x, y, ' ');
                gotoxy(x, y);
                std::cout << ' ';
                lastExplosion->objectsDestroyed++;
            }
        }
    }

    std::cout.flush();
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

//////////////////////////////////////////   getExplosionResult      //////////////////////////////////////////

ExplosionResult Bomb::getExplosionResult() const
{
    if (lastExplosion)
        return *lastExplosion;
    return ExplosionResult(); // Return empty result if no explosion yet
}
