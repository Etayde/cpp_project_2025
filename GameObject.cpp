//////////////////////////////////////       INCLUDES & FORWARDS       /////////////////////////////////////////////

#include "GameObject.h"
#include "Bomb.h"
#include "Door.h"
#include "Items.h"
#include "Riddle.h"
#include "Room.h"
#include "Spring.h"
#include "StaticObjects.h"
#include "Switch.h"

class Player;

//////////////////////////////////////////     Switch::onInteract    /////////////////////////////////////////////

bool Switch::onInteract(Player *player, Room *room)
{

  if (player == nullptr || room == nullptr)
    return false;

  toggle();
  draw();
  room->updatePuzzleState();

  return true;
}

//////////////////////////////////////////    createObjectFromChar       /////////////////////////////////////////////

GameObject *createObjectFromChar(char ch, int x, int y, int riddleId)
{
  Point pos(x, y, 0, 0, ch);

  static int nextRiddleId = 0;

  switch (ch)
  {
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
  case '0':
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
  case '8':
  case '9':
    return new Door(pos, ch - '0');
  case 'w':
    return new BreakableWall(pos);
  case 'Z':
    return new SwitchWall(pos);
  case '?':
  {
    int id = (riddleId >= 0) ? riddleId : nextRiddleId++;
    return new Riddle(pos, id);
  }
  default:
    return nullptr;
  }
}
