#include "Momentum.h"

//////////////////////////////////////////      Assignment Operator   //////////////////////////////////////////
Momentum &Momentum::operator=(const Momentum &other)
{
    if (this != &other)
    {
        activeState = other.activeState;
        dx = other.dx;
        dy = other.dy;
        launchFramesRemaining = other.launchFramesRemaining;
        launchDir = other.launchDir;
    }
    return *this;
}

/////////////////////////////////////////////      resetMomentum       //////////////////////////////////////////
void Momentum::resetMomentum()
    {
        activeState = false;
        dx = 0;
        dy = 0;
        launchFramesRemaining = 0;
        launchDir = Direction::STAY;
    }