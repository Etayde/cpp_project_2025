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

  if (player == nullptr || room == nullptr) return false;

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
// ... (existing code)

//////////////////////////////////////////          draw             //////////////////////////////////////////

void GameObject::draw() const
{
    if (!active || position.getX() < 0 || position.getY() < 0) return;

    switch (type)
    {
    case ObjectType::WALL:
        set_color(Color::BrightWhite);
        break;
    case ObjectType::BREAKABLE_WALL:
        set_color(Color::Gray);
        break;
    case ObjectType::SWITCH_WALL:
        set_color(Color::LightAqua);
        break;
    case ObjectType::TORCH:
        set_color(Color::LightYellow);
        break;
    case ObjectType::BOMB:
        set_color(Color::LightRed);
        break;
    case ObjectType::KEY:
        set_color(Color::Yellow);
        break;
    case ObjectType::DOOR:
        set_color(Color::LightBlue);
        break;
    case ObjectType::RIDDLE:
        set_color(Color::LightGreen);
        break;
    case ObjectType::SPRING:
    // case ObjectType::SPRING_LINK: // Identical value to SPRING
        set_color(Color::Green);
        break;
    case ObjectType::SWITCH_ON:
        set_color(Color::LightGreen);
        break;
    case ObjectType::SWITCH_OFF:
        set_color(Color::Gray); // Or Red?
        break;
    case ObjectType::OBSTACLE_BLOCK:
        set_color(Color::Gray);
        break;
    default:
        set_color(Color::White);
        break;
    }

    Renderer::printAt(position.getX(), position.getY(), sprite);
    reset_color();
}
