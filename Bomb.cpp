//////////////////////////////////////       INCLUDES & FORWARDS       /////////////////////////////////////////////

#include "Bomb.h"
#include "Room.h"
#include "Player.h"
#include "Renderer.h"
#include <cmath>

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
    ExplosionResult result;

    // Handle animation state per-tick
    if (state == BombState::ANIMATING)
    {
        animationTimer--;

        // Check player collision during animation each tick
        for (const Point& cell : explodedCells)
        {
            if (p1 && p1->getX() == cell.getX() && p1->getY() == cell.getY())
                result.player1Hit = true;
            if (p2 && p2->getX() == cell.getX() && p2->getY() == cell.getY())
                result.player2Hit = true;
        }

        // Animation finished
        if (animationTimer <= 0)
        {
            // Clear all cells in room
            for (const Point& cell : explodedCells)
            {
                if (currentRoom)
                    currentRoom->setCharAt(cell.getX(), cell.getY(), ' ');
            }
            explodedCells.clear();
            state = BombState::EXPLODED;
            active = false;
        }

        return result;
    }

    if (state != BombState::TICKING)
        return result;

    blinkCounter++;
    fuseTimer--;

    if (fuseTimer <= 0)
    {
        result = explode(p1, p2);
        return result;
    }

    return result;
}

//////////////////////////////////////////         explode           //////////////////////////////////////////

// Most of the bomb's logic methods, including this one,
// were made with AI assistance in the previous excercise but were in different classes.
// For better OOP oriented design, I moved the explode logic into the Bomb class itself
// to support better encapsulation and self containment.
ExplosionResult Bomb::explode(Player *p1, Player *p2)
{
    ExplosionResult result;
    explodedCells.clear();

    if (!currentRoom)
        return result;

    int centerX = getX();
    int centerY = getY();

    // Add center cell
    currentRoom->setCharAt(centerX, centerY, ' ');
    explodedCells.push_back(Point(centerX, centerY));

    for (int dy = -EXPLOSION_RADIUS; dy <= EXPLOSION_RADIUS; dy++)
    {
        for (int dx = -EXPLOSION_RADIUS; dx <= EXPLOSION_RADIUS; dx++)
        {
            int x = centerX + dx;
            int y = centerY + dy;

            if (x < 0 || x >= MAX_X || y < 0 || y >= MAX_Y)
                continue;

            double distance = sqrt(dx * dx + dy * dy);
            if (distance > EXPLOSION_RADIUS)
                continue;

            // Line of sight check - cells behind walls are sheltered
            if (!currentRoom->hasLineOfSight(centerX, centerY, x, y))
                continue;

            ObjectType type = currentRoom->getObjectTypeAt(x, y);
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
                    explodedCells.push_back(Point(x, y));
                    obj->setActive(false);
                    result.objectsDestroyed++;
                }
            }
            else if (type == ObjectType::BREAKABLE_WALL)
            {
                currentRoom->setCharAt(x, y, ' ');
                explodedCells.push_back(Point(x, y));
                result.objectsDestroyed++;
            }
            else if (type == ObjectType::AIR)
            {
                // Add air cells that are within line of sight (not sheltered)
                explodedCells.push_back(Point(x, y));
            }
            else if (type != ObjectType::WALL)
            {
                // Other destroyable non-wall cells
                currentRoom->setCharAt(x, y, ' ');
                explodedCells.push_back(Point(x, y));
                result.objectsDestroyed++;
            }
        }
    }

    // Start animation
    state = BombState::ANIMATING;
    animationTimer = ANIMATION_TICKS;

    return result;
}

//////////////////////////////////////////          draw             //////////////////////////////////////////

void Bomb::draw() const
{
    if (!active || state == BombState::IN_INVENTORY)
        return;

    if (state == BombState::PLACED && currentRoom && !currentRoom->isVisible(getX(), getY()))
        return;

    // Draw explosion animation
    if (state == BombState::ANIMATING)
    {
        // Blink: show '~' for 5 ticks, hide for 5 ticks
        bool showWave = (animationTimer / 5) % 2 == 0;
        
        if (showWave)
        {
            set_color(Color::Yellow);
            for (const Point& cell : explodedCells)
            {
                Renderer::printAt(cell.getX(), cell.getY(), '~');
            }
            reset_color();
        }
        else
        {
            for (const Point& cell : explodedCells)
            {
                Renderer::printAt(cell.getX(), cell.getY(), ' ');
            }
        }
        Renderer::flush();
        return;
    }

    Renderer::gotoxy(getX(), getY());

    if (state == BombState::TICKING)
    {
        if ((blinkCounter % BLINK_RATE) < 5) 
        {
            set_color(Color::Green);
            Renderer::print('@');
        }
        else 
        {
            set_color(Color::LightGreen);
            Renderer::print('*');
        }
    }
    else 
    {
        set_color(Color::Green);
        Renderer::print(sprite);
    }
    reset_color();

    Renderer::flush();
}

//////////////////////////////////////////       isPickable          //////////////////////////////////////////

bool Bomb::isPickable() const
{
    return (state == BombState::PLACED);
}
