
#include "GameObject.h"
#include "Room.h"
#include "Items.h"
#include "Bomb.h"
#include "Switch.h"
#include "Door.h"
#include "StaticObjects.h"
#include "Spring.h"

class Player;
//////////////////////////////////////////     Switch::onInteract     //////////////////////////////////////////

// Toggles switch state and updates room puzzle
bool Switch::onInteract(Player* player, Room* room) {

    if (player == nullptr || room == nullptr) return false;
    
    toggle();
    draw();
    room->updatePuzzleState();

    return true;
}

//////////////////////////////////////////    createObjectFromChar    //////////////////////////////////////////

// Factory function - creates appropriate object type from character
GameObject* createObjectFromChar(char ch, int x, int y) {
    Point pos(x, y, 0, 0, ch);

    switch (ch) {
        case 'K':
            return new Key(pos);
        case '@':
            return new Bomb(pos);
        case '!':
            return new Torch(pos);
        case '\\':
            return new Switch(pos, false);
        case '/':
            return new Switch(pos, true);
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            return new Door(pos, ch - '0');
        case 'W':
            return new Wall(pos);
        case '=':
            return new BreakableWall(pos);
        case '*':
            return new Obstacle(pos);
        case '#':
            return new Spring(pos);
        case '?':
            return new Riddle(pos);
        default:
            return nullptr;
    }
}
