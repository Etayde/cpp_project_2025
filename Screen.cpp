//////////////////////////////////////       INCLUDES & FORWARDS       //////////////////////////////////////////

#include "Screen.h"
#include "Point.h"

//////////////////////////////////////////         getCharAt          //////////////////////////////////////////

char Screen::getCharAt(const Point &p) const { return getCharAt(p.getX(), p.getY()); }

//////////////////////////////////////////          isWall            //////////////////////////////////////////

bool Screen::isWall(const Point &p) const { return getCharAt(p) == 'W'; }

//////////////////////////////////////////         isObject           //////////////////////////////////////////

bool Screen::isObject(const Point &p) const
{
    char c = getCharAt(p);
    return (c != ' ' && c != 'W');
}

//////////////////////////////////////////         objectIs           //////////////////////////////////////////

ObjectType Screen::objectIs(const Point &p) const
{
    char ch = getCharAt(p);

    switch (ch)
    {
    case '#':
        return ObjectType::SPRING;
    case '*':
        return ObjectType::OBSTACLE_BLOCK;
    case '!':
        return ObjectType::TORCH;
    case '@':
        return ObjectType::BOMB;
    case 'K':
        return ObjectType::KEY;
    case '\\':
        return ObjectType::SWITCH_OFF;
    case '/':
        return ObjectType::SWITCH_ON;
    case '?':
        return ObjectType::RIDDLE;
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
        return ObjectType::DOOR;
    default:
        return ObjectType::AIR;
    }
}

//////////////////////////////////////////          draw            //////////////////////////////////////////

void Screen::draw() const
{
    Renderer::clrscr();
    Renderer::gotoxy(0, 0);

    for (int i = 0; i < MAX_Y; ++i)
    {
      if (screen[i] != nullptr)
      {
        for (int j = 0; screen[i][j] != '\0'; ++j)
        {
            char c = screen[i][j];
            
            // Basic static map coloring
            if (c == 'W' || c == '|' || c == '-' || c == '=')
            {
                if (c == '=') set_color(Color::Gray);
                else set_color(Color::BrightWhite);
            }
            // For other static chars like floors or HUD text, keep default (White) or set specific
            
            Renderer::print(c);
            reset_color();
        }
        Renderer::print('\n');
      }
    }
    Renderer::flush();
}
