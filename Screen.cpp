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

    for (int i = 0; i < MAX_Y - 1; ++i)
    {
      if (screen[i] != nullptr)
      {
        for (int j = 0; screen[i][j] != '\0'; ++j)
        {
            char c = screen[i][j];
            
            // Color walls and borders white
            if (c == 'W' || c == 'w' || c == 'Z' || c == '|' || c == '-' || c == '=')
                set_color(Color::White);
            else if (c == '!') set_color(Color::LightYellow);
            else if (c == 'K' || c == '/' || c == '\\') set_color(Color::LightPurple);
            else if (c == '@') set_color(Color::Green);
            else if (c >= '0' && c <= '9') set_color(Color::Purple);
            else if (c == '#' || c == '*') set_color(Color::Gray);
            else if (c == '?') set_color(Color::LightBlue);
            
            Renderer::print(c);
            reset_color();
        }
        Renderer::print('\n');
      }
    }

    // Print last line without trailing newline
    if (screen[MAX_Y - 1] != nullptr)
    {
        for (int j = 0; screen[MAX_Y - 1][j] != '\0'; ++j)
        {
            char c = screen[MAX_Y - 1][j];
            
            if (c == 'W' || c == 'w' || c == 'Z' || c == '|' || c == '-' || c == '=')
                set_color(Color::White);
            else if (c == '!') set_color(Color::LightYellow);
            else if (c == 'K' || c == '/' || c == '\\') set_color(Color::LightPurple);
            else if (c == '@') set_color(Color::Green);
            else if (c >= '0' && c <= '9') set_color(Color::Purple);
            else if (c == '#' || c == '*') set_color(Color::Gray);
            else if (c == '?') set_color(Color::LightBlue);
            
            Renderer::print(c);
            reset_color();
        }
    }
    Renderer::flush();
}
