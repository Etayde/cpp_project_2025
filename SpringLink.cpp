#include "SpringLink.h"
#include "Spring.h"
#include "Room.h"
#include "Console.h"
#include <iostream>

// Constructor
SpringLink::SpringLink(const Point& pos, Spring* parent, int index)
    : StaticObject(pos, '#', ObjectType::SPRING_LINK),
      parentSpring(parent),
      linkIndex(index),
      collapsed(false)
{
}

// Clone
GameObject* SpringLink::clone() const
{
    return new SpringLink(*this);
}

// Handle explosion - destroy entire spring
bool SpringLink::onExplosion()
{
    if (parentSpring)
    {
        parentSpring->destroyAllLinks();
    }
    return true;
}

// Collapse this link and update visual
void SpringLink::collapse(Room* room)
{
    if (collapsed)
    {
        return;
    }

    collapsed = true;
    sprite = ' ';  // Change sprite to space (or '~' for visual feedback)

    // Update room's character map and display
    if (room != nullptr)
    {
        room->setCharAt(position.x, position.y, sprite);
        gotoxy(position.x, position.y);
        std::cout << sprite << std::flush;
    }
}

// Reset to uncompressed state
void SpringLink::reset(Room* room)
{
    if (!collapsed)
    {
        return;
    }

    collapsed = false;
    sprite = '#';  // Back to normal spring visual

    // Update room's character map and display
    if (room != nullptr)
    {
        room->setCharAt(position.x, position.y, sprite);
        gotoxy(position.x, position.y);
        std::cout << sprite << std::flush;
    }
}
