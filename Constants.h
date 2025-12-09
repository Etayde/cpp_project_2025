
#pragma once
//////////////////////////////////////////      SCREEN DIMENSIONS      //////////////////////////////////////////

enum ScreenSize
{
    MAX_X = 80,       // Screen width
    MAX_Y = 25,       // Total screen height
    MAX_Y_INGAME = 21 // Playable area (bottom 4 rows for UI)
};

//////////////////////////////////////////      ROOM CONFIGURATION     //////////////////////////////////////////

enum RoomCount
{
    TOTAL_ROOMS = 3
};

//////////////////////////////////////////         GAME STATES         //////////////////////////////////////////

enum class GameState
{
    mainMenu,
    instructions,
    quit,
    inGame,
    paused,
    gameOver,
    victory
};

//////////////////////////////////////////          DIRECTIONS         //////////////////////////////////////////

enum class Direction
{
    UP,
    DOWN,
    LEFT,
    RIGHT,
    STAY
};

//////////////////////////////////////////        PLAYER ACTIONS       //////////////////////////////////////////

enum class Action
{
    MOVE_UP,
    MOVE_DOWN,
    MOVE_LEFT,
    MOVE_RIGHT,
    STAY,
    DROP_ITEM,
    ESC = 27
};

//////////////////////////////////////////        OBJECT TYPES         //////////////////////////////////////////

// Char values match sprites for easy mapping
enum class ObjectType
{
    AIR = ' ',
    WALL = 'W',
    BREAKABLE_WALL = '=',
    SPRING = '#',
    OBSTACLE = '*',
    TORCH = '!',
    BOMB = '@',
    KEY = 'K',
    SWITCH_OFF = '\\',
    SWITCH_ON = '/',
    RIDDLE = '?',
    DOOR = 'D',
    SWITCH_WALL = 'Z'
};

//////////////////////////////////////////       BOMB CONSTANTS        //////////////////////////////////////////

namespace BombConfig
{
    constexpr int FUSE_TIME = 50; // ~5 seconds at 10 FPS
    constexpr int RADIUS = 5;
    constexpr int BLINK_RATE = 10;
}

//////////////////////////////////////////       LIGHT CONSTANTS       //////////////////////////////////////////

namespace LightConfig
{
    constexpr int TORCH_RADIUS = 2;
}

//////////////////////////////////////////        ROOM LIMITS          //////////////////////////////////////////

namespace RoomLimits
{
    constexpr int MAX_OBJECTS = 100;
    constexpr int MAX_MODS = 100;
    constexpr int MAX_DARK_ZONES = 10;
}

//////////////////////////////////////////       DOOR DEFAULTS         //////////////////////////////////////////

namespace DoorConfig
{
    constexpr int DEFAULT_REQUIRED_KEYS = 1;
    constexpr int DEFAULT_REQUIRED_SWITCHES = 0;
}

//////////////////////////////////////////      PLAYER SPRITES         //////////////////////////////////////////

namespace PlayerSprites
{
    constexpr char PLAYER1 = '$';
    constexpr char PLAYER2 = '&';
}

//////////////////////////////////////////       INVENTORY UI          //////////////////////////////////////////

namespace InventoryUI
{
    constexpr int PLAYER1_X = 18;
    constexpr int PLAYER2_X = 58;
    constexpr int Y_POS = 23;
}
