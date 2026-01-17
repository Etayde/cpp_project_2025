#pragma once

//////////////////////////////////////       INCLUDES & FORWARDS       /////////////////////////////////////////////

#include "StaticObjects.h"
#include "Point.h"
#include "Constants.h"

class Spring;
class Room;

//////////////////////////////////////////        SpringLink       /////////////////////////////////////////////

class SpringLink : public StaticObject
{
    Spring *parentSpring;
    int linkIndex;
    bool collapsed;

public:
    SpringLink(const Point &pos, Spring *parent, int index);

    GameObject *clone() const override;
    const char *getName() const override { return "SpringLink"; }
    bool isBlocking() const override { return false; }
    bool onExplosion() override;

    Spring *getParentSpring() const { return parentSpring; }
    void setParentSpring(Spring *parent) { parentSpring = parent; }
    int getLinkIndex() const { return linkIndex; }
    bool isCollapsed() const { return collapsed; }
    void collapse(Room *room);
    void reset(Room *room);
    bool isStartLink() const { return linkIndex == 0; }
};
