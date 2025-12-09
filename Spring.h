#pragma once

//////////////////////////////////////       INCLUDES & FORWARDS       //////////////////////////////////////////

#include "StaticObjects.h"

//////////////////////////////////////////          Spring            //////////////////////////////////////////

// Boosts player movement
class Spring : public StaticObject
{
private:
    int boostStrength;

public:
    Spring() : StaticObject(), boostStrength(1)
    {
        sprite = '#';
        type = ObjectType::SPRING;
    }

    Spring(const Point &pos, int boost = 1)
        : StaticObject(pos, '#', ObjectType::SPRING), boostStrength(boost) {}

    GameObject *clone() const override { return new Spring(*this); }
    const char *getName() const override { return "Spring"; }

    bool isBlocking() const override { return false; }
    bool onExplosion() override { return true; }

    int getBoostStrength() const { return boostStrength; }
};