#pragma once

// Screen dimensions
enum ScreenSize { MAX_X = 80, MAX_Y = 25, MAX_Y_INGAME = 21 };
enum RoomCount { TOTAL_ROOMS = 3 };

// Game states
enum class GameState {
    mainMenu,
    instructions,
    quit,
    inGame,
    paused,
    gameOver,
    victory
};

// Movement directions
enum class Direction {
    UP,
    DOWN,
    LEFT,
    RIGHT,
    STAY
};

// Player actions
enum class Action {
    MOVE_UP,
    MOVE_DOWN,
    MOVE_LEFT,
    MOVE_RIGHT,
    STAY,
    DROP_ITEM,
    ESC = 27
};

// Object types - using char values for easy sprite mapping
enum class ObjectType {
    AIR = ' ',
    WALL = 'W',
    BREAKABLE_WALL = '=',  // Can be destroyed by bomb
    SPRING = '#',
    OBSTACLE = '*',
    TORCH = '!',
    BOMB = '@',
    KEY = 'K',
    SWITCH_OFF = '\\',
    SWITCH_ON = '/',
    RIDDLE = '?',
    DOOR = 'D'
};
