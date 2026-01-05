
#pragma once
#include <string_view>
//////////////////////////////////////////      SCREEN DIMENSIONS       /////////////////////////////////////////////

enum ScreenSize {
  MAX_X = 80,       // Screen width
  MAX_Y = 25,       // Total screen height
  MAX_Y_INGAME = 25 // Playable area (bottom 4 rows for UI)
};

//////////////////////////////////////////         GAME STATES       /////////////////////////////////////////////

enum class GameState {
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

enum class Direction {
  UP,
  DOWN,
  LEFT,
  RIGHT,
  STAY,
  HORIZONTAL, // For spring orientation (not movement direction)
  VERTICAL    // For spring orientation (not movement direction)
};

//////////////////////////////////////////        PLAYER ACTIONS       /////////////////////////////////////////////

enum class Action {
  MOVE_UP,
  MOVE_DOWN,
  MOVE_LEFT,
  MOVE_RIGHT,
  STAY,
  DROP_ITEM,
  ESC = 27
};

//////////////////////////////////////////        RIDDLE RESULTS       /////////////////////////////////////////////

enum class RiddleResult {
  SOLVED, // Correct answer - remove riddle
  FAILED, // Wrong answer - keep riddle
  ESCAPED // ESC pressed - pause game
};

//////////////////////////////////////////        OBJECT TYPES       /////////////////////////////////////////////

// Char values match sprites for easy mapping
enum class ObjectType {
  AIR = ' ',
  WALL = 'W',
  BREAKABLE_WALL = 'w',
  SPRING = '#',
  SPRING_LINK = '#', // Individual spring link (same sprite as SPRING)
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

// Bomb constants moved to Bomb class for better encapsulation
// Torch constants moved to Torch class for better encapsulation

// Operator overloads for convenient comparison with char
inline bool operator==(char c, ObjectType t) {
  return c == static_cast<char>(t);
}

inline bool operator==(ObjectType t, char c) { return c == t; }

inline bool operator!=(char c, ObjectType t) { return !(c == t); }

inline bool operator!=(ObjectType t, char c) { return !(c == t); }

//////////////////////////////////////////        ROOM LIMITS       /////////////////////////////////////////////

namespace RoomLimits {
constexpr int MAX_OBJECTS = 100;
constexpr int MAX_MODS = 100;
constexpr int MAX_DARK_ZONES = 10;
} // namespace RoomLimits

//////////////////////////////////////////       DOOR DEFAULTS       /////////////////////////////////////////////

namespace DoorConfig {
constexpr int DEFAULT_REQUIRED_KEYS = 1;
constexpr int DEFAULT_REQUIRED_SWITCHES = 0;
} // namespace DoorConfig

//////////////////////////////////////////      PLAYER SPRITES       /////////////////////////////////////////////

namespace PlayerSprites {
constexpr char PLAYER1 = '$';
constexpr char PLAYER2 = '&';
} // namespace PlayerSprites

//////////////////////////////////////////       INVENTORY UI       /////////////////////////////////////////////

namespace InventoryUI {
constexpr int WIDTH = 20;
constexpr int HEIGHT = 3;
} // namespace InventoryUI

namespace BlockingChars {
inline constexpr char WALL = 'W';
inline constexpr char LEGEND_CORNER = '+';
inline constexpr char LEGEND_HORIZONTAL = '-';
inline constexpr char LEGEND_VERTICAL = '|';

// Implementation of std::string_view suggested by AI
// Originally was implemented with switch case, but this is much cleaner
inline std::string_view BLOCKING_CHARS = "W+|-";

inline bool isBlockingChar(char c) {
  return BLOCKING_CHARS.find(c) != std::string_view::npos;
}

} // namespace BlockingChars

namespace PlayerConstants {
constexpr int RESPAWN_DURATION_FRAMES = 50;
} // namespace PlayerConstants
