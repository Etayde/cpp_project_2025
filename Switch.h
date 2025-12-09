#pragma once

//////////////////////////////////////       INCLUDES & FORWARDS       //////////////////////////////////////////

#include "InteractableObject.h"

//////////////////////////////////////////          Switch            //////////////////////////////////////////

// A switch that can be toggled on/off to control doors
class Switch : public InteractableObject
{
private:
    bool isOn;

public:
    Switch() : InteractableObject(), isOn(false)
    {
        sprite = '\\';
        type = ObjectType::SWITCH_OFF;
    }

    Switch(const Point &pos, bool startOn = false)
        : InteractableObject(pos, startOn ? '/' : '\\',
                             startOn ? ObjectType::SWITCH_ON : ObjectType::SWITCH_OFF),
          isOn(startOn) {}

    GameObject *clone() const override { return new Switch(*this); }
    const char *getName() const override { return isOn ? "Switch (ON)" : "Switch (OFF)"; }

    bool isBlocking() const override { return true; }
    bool onExplosion() override { return true; }

    // Toggle the switch state
    void toggle()
    {
        isOn = !isOn;
        if (isOn)
        {
            sprite = '/';
            type = ObjectType::SWITCH_ON;
        }
        else
        {
            sprite = '\\';
            type = ObjectType::SWITCH_OFF;
        }
    }

    bool getIsOn() const { return isOn; }
    void setIsOn(bool on)
    {
        isOn = on;
        sprite = isOn ? '/' : '\\';
        type = isOn ? ObjectType::SWITCH_ON : ObjectType::SWITCH_OFF;
    }

    // Handle player interaction
    bool onInteract(Player *player, Room *room) override;
};
