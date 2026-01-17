//////////////////////////////////////       INCLUDES & FORWARDS       //////////////////////////////////////////

#include "Items.h"
#include "Room.h"

//////////////////////////////////////////    Torch::illuminate       //////////////////////////////////////////

void Torch::illuminate(Room *room, int playerX, int playerY) const
{
    if (room == nullptr)
        return;

    if (room->isInDarkZone(playerX, playerY))
    {
        room->lightRadius(playerX, playerY, LIGHT_RADIUS);
    }
}
