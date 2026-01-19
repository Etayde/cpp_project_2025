
#pragma once
#include <string_view>
//////////////////////////////////////////      SCREEN DIMENSIONS       /////////////////////////////////////////////

enum ScreenSize
{
  MAX_X = 80,
  MAX_Y = 25,
};

//////////////////////////////////////////         GAME STATES       /////////////////////////////////////////////

enum class GameState
{
  mainMenu,
  instructions,
  quit,
  inGame,
  paused,
  gameOver,
  victory,
  error
};

//////////////////////////////////////////          DIRECTIONS       /////////////////////////////////////////////

enum class Direction
{
  UP,
  DOWN,
  LEFT,
  RIGHT,
  STAY,
  HORIZONTAL,
  VERTICAL
};

//////////////////////////////////////////        PLAYER ACTIONS       /////////////////////////////////////////////

enum class Action
{
  MOVE_UP,
  MOVE_DOWN,
  MOVE_LEFT,
  MOVE_RIGHT,
  STAY,
  DROP_ITEM,
  ANSWER_RIDDLE,
  ESC = 27
};

//////////////////////////////////////////        RIDDLE RESULTS       /////////////////////////////////////////////

enum class RiddleResult
{
  SOLVED,
  FAILED,
  ESCAPED,
  NO_RIDDLE
};

//////////////////////////////////////////        OBJECT TYPES       /////////////////////////////////////////////

enum class ObjectType
{
  AIR = ' ',
  WALL = 'W',
  BREAKABLE_WALL = 'w',
  SPRING = '#',
  SPRING_LINK = '#',
  OBSTACLE_BLOCK = '*',
  TORCH = '!',
  BOMB = '@',
  KEY = 'K',
  SWITCH_OFF = '\\',
  SWITCH_ON = '/',
  RIDDLE = '?',
  DOOR = 'D',
  SWITCH_WALL = 'Z'
};

inline bool operator==(char c, ObjectType t)
{
  return c == static_cast<char>(t);
}

inline bool operator==(ObjectType t, char c) { return c == t; }

inline bool operator!=(char c, ObjectType t) { return !(c == t); }

inline bool operator!=(ObjectType t, char c) { return !(c == t); }

//////////////////////////////////////////       DOOR DEFAULTS       /////////////////////////////////////////////

namespace DoorConfig
{
  constexpr int DEFAULT_REQUIRED_KEYS = 1;
  constexpr int DEFAULT_REQUIRED_SWITCHES = 0;
}

//////////////////////////////////////////      PLAYER SPRITES       /////////////////////////////////////////////

namespace PlayerSprites
{
  constexpr char PLAYER1 = '$';
  constexpr char PLAYER2 = '&';
}

//////////////////////////////////////////       INVENTORY UI       /////////////////////////////////////////////

namespace InventoryUI
{
  constexpr int WIDTH = 20;
  constexpr int HEIGHT = 3;
}

//////////////////////////////////////////      BLOCKING CHARS       /////////////////////////////////////////////

namespace BlockingChars
{
  inline constexpr char WALL = 'W';
  inline constexpr char LEGEND_CORNER = '+';
  inline constexpr char LEGEND_HORIZONTAL = '-';
  inline constexpr char LEGEND_VERTICAL = '|';

  // Implementation of std::string_view suggested by AI
  // Originally was implemented with switch case, but this is much cleaner
  inline std::string_view BLOCKING_CHARS = "W+|-";

  inline bool isBlockingChar(char c)
  {
    return BLOCKING_CHARS.find(c) != std::string_view::npos;
  }

}

//////////////////////////////////////////      PLAYER CONSTANTS       /////////////////////////////////////////////

namespace PlayerConstants
{
  constexpr int RESPAWN_DURATION_FRAMES = 50;
}

//////////////////////////////////////////      ERROR CODES       /////////////////////////////////////////////

enum class ErrorCode
{
  NONE,
  FILE_NOT_FOUND,
  INVALID_FORMAT,
  READ_ERROR,
  WRITE_ERROR,
  L_NOT_FOUND,
  MULTIPLE_L,
  L_OUT_OF_BOUNDS,
  LEGEND_OBSCURES_OBJECTS,
  LEGEND_OBSCURES_SPAWN,
  NO_SCREENS_FOUND,
  MISSING_RANDOM_SEED,
  SCREEN_MISMATCH
};
