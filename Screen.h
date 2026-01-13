#pragma once

//////////////////////////////////////////       INCLUDES & FORWARDS       /////////////////////////////////////////////

#include "Console.h"
#include "Constants.h"
#include "Renderer.h"
#include <iostream>
#include <string>

class Point;

//////////////////////////////////////////          Screen       /////////////////////////////////////////////

class Screen
{
private:
  const char *screen[MAX_Y];
  std::string ownedData[MAX_Y];

public:
  Screen(const char *layout[MAX_Y])
  {
    for (int i = 0; i < MAX_Y; i++)
      screen[i] = layout[i];
  }

  Screen()
  {
    for (int i = 0; i < MAX_Y; i++)
      screen[i] = nullptr;
  }

  Screen(const std::string layout[MAX_Y])
  {
    for (int i = 0; i < MAX_Y; i++)
    {
      ownedData[i] = layout[i];
      screen[i] = ownedData[i].c_str();
    }
  }

  char getCharAt(int x, int y) const
  {
    if (x >= 0 && x < MAX_X && y >= 0 && y < MAX_Y && screen[y] != nullptr)
    {
      return screen[y][x];
    }
    return 'W';
  }

  char getCharAt(const Point &p) const;

  bool isWall(int x, int y) const { return getCharAt(x, y) == 'W'; }
  bool isWall(const Point &p) const;

  bool isObject(const Point &p) const;

  ObjectType objectIs(const Point &p) const;

  void draw() const
  {
    Renderer::clrscr();
    Renderer::gotoxy(0, 0);

    for (int i = 0; i < MAX_Y - 1; ++i)
    {
      if (screen[i] != nullptr)
      {
        Renderer::print(screen[i]);
        Renderer::print('\n');
      }
    }

    if (screen[MAX_Y - 1] != nullptr)
      Renderer::print(screen[MAX_Y - 1]);
    Renderer::flush();
  }
};
