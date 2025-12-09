#pragma once

#include "PickableObject.h"

//////////////////////////////////////////           Key              //////////////////////////////////////////

// A key that unlocks doors
class Key : public PickableObject
{
public:
    Key() : PickableObject()
    {
        sprite = 'K';
        type = ObjectType::KEY;
    }

    Key(const Point &pos) : PickableObject(pos, 'K', ObjectType::KEY) {}

    GameObject *clone() const override { return new Key(*this); }
    const char *getName() const override { return "Key"; }
};

//////////////////////////////////////////          Torch             //////////////////////////////////////////

// A light source that illuminates dark areas
class Torch : public PickableObject
{
private:
    int lightRadius;

public:
    static const int DEFAULT_LIGHT_RADIUS = 4;

    Torch() : PickableObject(), lightRadius(DEFAULT_LIGHT_RADIUS)
    {
        sprite = '!';
        type = ObjectType::TORCH;
    }

    Torch(const Point &pos) : PickableObject(pos, '!', ObjectType::TORCH),
                              lightRadius(DEFAULT_LIGHT_RADIUS) {}

    GameObject *clone() const override { return new Torch(*this); }
    const char *getName() const override { return "Torch"; }

    int getLightRadius() const { return lightRadius; }
    void setLightRadius(int radius) { lightRadius = radius; }
};