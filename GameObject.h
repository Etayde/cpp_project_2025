#pragma once

//////////////////////////////////////////       INCLUDES & FORWARDS       /////////////////////////////////////////////

#include "Console.h"
#include "Constants.h"
#include "Point.h"
#include "Renderer.h"
#include <iostream>

class Player;
class Room;
class Key;
class Bomb;
class Torch;
class Switch;
class Door;
class Wall;
class BreakableWall;
class SwitchWall;
class Spring;
class Riddle;

//////////////////////////////////////////         GameObject       /////////////////////////////////////////////

// Abstract base class for all game objects
// Uses virtual methods for polymorphism (with help of AI)
class GameObject
{
protected:
  Point position;
  char sprite;
  ObjectType type;
  bool active;

public:
  GameObject()
      : position(Point(-1, -1)), sprite(' '), type(ObjectType::AIR),
        active(false) {}

  GameObject(const Point &pos, char spr, ObjectType t)
      : position(pos), sprite(spr), type(t), active(true) {}

  virtual ~GameObject() = default;

  //////////////////////////////////////////    Pure Virtual Methods    /////////////////////////////////////////////

  // Must be implemented by derived classes
  virtual bool isBlocking() const = 0;
  virtual bool isPickable() const = 0;
  virtual bool isInteractable() const = 0;
  virtual GameObject *clone() const = 0;
  virtual const char *getName() const = 0;

  //////////////////////////////////////////      Virtual Methods      /////////////////////////////////////////////

  virtual bool onInteract(Player * /*player*/, Room * /*room*/)
  {
    return false;
  }
  virtual bool onExplosion() { return false; }
  virtual bool isAlwaysVisible() const { return false; }

  virtual void draw() const
  {
    if (active && position.x >= 0 && position.y >= 0)
    {
      Renderer::printAt(position.x, position.y, sprite);
    }
  }

  virtual void update() {}

  //////////////////////////////////////////         Getters         /////////////////////////////////////////////

  Point getPosition() const { return position; }
  int getX() const { return position.x; }
  int getY() const { return position.y; }
  char getSprite() const { return sprite; }
  ObjectType getType() const { return type; }
  bool isActive() const { return active; }

  //////////////////////////////////////////         Setters           /////////////////////////////////////////////

  void setPosition(const Point &pos) { position = pos; }
  void setPosition(int x, int y)
  {
    position.x = x;
    position.y = y;
  }
  void setSprite(char spr) { sprite = spr; }
  void setType(ObjectType t) { type = t; }
  void setActive(bool a) { active = a; }
};

//////////////////////////////////////////    createObjectFromChar       /////////////////////////////////////////////

GameObject *createObjectFromChar(char ch, int x, int y, int riddleId = -1);
