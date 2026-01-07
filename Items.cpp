//////////////////////////////////////       INCLUDES & FORWARDS       //////////////////////////////////////////

#include "Items.h"
#include "Room.h"

//////////////////////////////////////////    Torch::illuminate       //////////////////////////////////////////

// Apply torch lighting effect to room at player position
void Torch::illuminate(Room *room, int playerX, int playerY) const
{
    if (room == nullptr)
        return;

    // Only light up if player is in a dark zone
    if (room->isInDarkZone(playerX, playerY))
    {
        room->lightRadius(playerX, playerY, LIGHT_RADIUS);
    }
}
