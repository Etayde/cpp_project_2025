#pragma once

//////////////////////////////////////////       INCLUDES & FORWARDS       //////////////////////////////////////////
#include "Console.h"
#include "Constants.h"

//////////////////////////////////////////          Momentum          //////////////////////////////////////////
// Momentum representation class
class Momentum
{
private:
    bool activeState;
    int dx;
    int dy;
    int launchFramesRemaining;
    Direction launchDir;

public:
    Momentum()
        : activeState(false), dx(0), dy(0), launchFramesRemaining(0), launchDir(Direction::STAY)
    {
    }

    Momentum &operator=(const Momentum &other);
    Momentum(const Momentum &other)
        : activeState(other.activeState), dx(other.dx), dy(other.dy),
          launchFramesRemaining(other.launchFramesRemaining), launchDir(other.launchDir) {}

    // Getters
    bool isActive() const { return activeState; }
    int getDX() const { return dx; }
    int getDY() const { return dy; }
    int getLaunchFramesRemaining() const { return launchFramesRemaining; }
    Direction getLaunchDir() const { return launchDir; }

    // Setters
    void setActive(bool state) { activeState = state; }
    void setDX(int deltaX) { dx = deltaX; }
    void setDY(int deltaY) { dy = deltaY; }
    void setLaunchFramesRemaining(int frames) { launchFramesRemaining = frames; }
    void setLaunchDir(Direction dir) { launchDir = dir; }
    void resetMomentum();
    void incrementDX(int deltaX) { dx += deltaX; }
    void incrementDY(int deltaY) { dy += deltaY; }
};
