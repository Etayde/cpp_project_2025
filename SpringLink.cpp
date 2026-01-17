//////////////////////////////////////       INCLUDES & FORWARDS       /////////////////////////////////////////////

#include "SpringLink.h"
#include "Spring.h"
#include "Room.h"
#include "Console.h"
#include "Renderer.h"
#include <iostream>

//////////////////////////////////////////       Constructor       /////////////////////////////////////////////

SpringLink::SpringLink(const Point &pos, Spring *parent, int index)
    : StaticObject(pos, '#', ObjectType::SPRING_LINK),
      parentSpring(parent), linkIndex(index), collapsed(false) {}

//////////////////////////////////////////          clone       /////////////////////////////////////////////

GameObject *SpringLink::clone() const { return new SpringLink(*this); }

//////////////////////////////////////////       onExplosion       /////////////////////////////////////////////

bool SpringLink::onExplosion() {
    if (parentSpring) parentSpring->destroyAllLinks();
    return true;
}

//////////////////////////////////////////        collapse       /////////////////////////////////////////////

void SpringLink::collapse(Room *room) {
    if (collapsed) return;

    collapsed = true;
    sprite = ' ';

    if (room != nullptr)
    {
        room->setCharAt(position.x, position.y, sprite);
        Renderer::printAt(position.x, position.y, sprite);
        Renderer::flush();
    }
}

//////////////////////////////////////////          reset       /////////////////////////////////////////////

void SpringLink::reset(Room *room)
{
    if (!collapsed) return;

    collapsed = false;
    sprite = '#';

    if (room != nullptr)
    {
        room->setCharAt(position.x, position.y, sprite);
        Renderer::printAt(position.x, position.y, sprite);
        Renderer::flush();
    }
}
