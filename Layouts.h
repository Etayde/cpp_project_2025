#pragma once

//////////////////////////////////////       INCLUDES & FORWARDS       //////////////////////////////////////////

#include "Screen.h"

//////////////////////////////////////////        Layouts           //////////////////////////////////////////

// Main menu screen data
const char *mainMenuData[MAX_Y] = {
    //0123456789012345678901234567890123456789012345678901234567890123456789012345678
     "===============================================================================", // 0
     "|                                                                             |", // 1
     "|                                                                             |", // 2
     "|                                                                             |", // 3
     "|                            WELCOME TO THE GAME                              |", // 4
     "|                           ---------------------                             |", // 5
     "|                                                                             |", // 6
     "|                       TWO PLAYERS COOPERATIVE ADVENTURE                     |", // 7
     "|                                                                             |", // 8
     "|                            [1] Start New Game                               |", // 9
     "|                           [8] View Instructions                             |", // 10
     "|                                 [9] Exit                                    |", // 11
     "|                                                                             |", // 12
     "|                                                                             |", // 13
     "|                                                                             |", // 14
     "|                                                                             |", // 15
     "|                                                                             |", // 16
     "|                                                                             |", // 17
     "|                                                                             |", // 18
     "|                                                                             |", // 19
     "|                                                                             |", // 20
     "|                                                                             |", // 21
     "|                                                                             |", // 22
     "|                                                                             |", // 23
     "==============================================================================="  // 24
};
static Screen mainMenuScreen(mainMenuData);

// Instructions screen data
const char *instructionsData[MAX_Y] = {
    //0123456789012345678901234567890123456789012345678901234567890123456789012345678
     "===============================================================================", // 0
     "|                               INSTRUCTIONS                                  |", // 1
     "|                           --------------------                              |", // 2
     "|     GOAL: Both players must cooperate to reach the FINAL ROOM together.     |", // 3
     "|  -------------------------------- | --------------------------------------  |", // 4
     "|            CONTROLS (Auto - Move) | MAP LEGEND & ITEMS                      |", // 5
     "|  -------------------------------- | --------------------------------------  |", // 6
     "|     ACTION      P1($)    P2(&)    |   W    Wall 1     Unbreakable           |", // 7
     "|     UP            W        I      |   =    Wall 2     Breakable             |", // 8
     "|     DOWN          X        M      |   #    Spring     Boosts speed          |", // 9
     "|     LEFT          A        J      |   Z    SwitchWall Can be pushed         |", // 10
     "|     RIGHT         D        L      |   !    Torch      Lights dark rooms     |", // 11
     "|     STAY          S        K      |   @    Bomb       Explodes (danger!)    |", // 12
     "|     DISPOSE       E        O      |   K    Key        Unlocks Doors (0-9)   |", // 13
     "|                                   |   \\ /  Switch    Toggles obstacles     |", // 14
     "|    * Characters move until STOP   |   ?    Riddle     Must solve to pass    |", // 15
     "|      is pressed or hit a wall.    |   0-9  Doors      Go to next room       |", // 16
     "|  -------------------------------- | --------------------------------------  |", // 17
     "|     SYSTEM: [ESC] - Pause game                                              |", // 18
     "|             [H]   - From Pause menu, return to main menu                    |", // 19
     "|            * Exiting back to main menu clears progress.                     |", // 20
     "|     INVENTORY : Shown at bottom of screen for each player.                  |", // 21
     "|=============================================================================|", // 22
     "|                          Press [ESC] to return                              |", // 23
     "==============================================================================="  // 24
};
static Screen instructionsScreen(instructionsData);

// Pause screen data
const char *pauseData[MAX_Y] = {
    //0123456789012345678901234567890123456789012345678901234567890123456789012345678
     "===============================================================================", // 0
     "|                                                                             |", // 1
     "|                                                                             |", // 2
     "|                                                                             |", // 3
     "|                                                                             |", // 4
     "|                                                                             |", // 5
     "|                                                                             |", // 6
     "|                               GAME PAUSED                                   |", // 7
     "|                             ----------------                                |", // 8
     "|                                                                             |", // 9
     "|                           Press [ESC] to Resume                             |", // 10
     "|                          Press [H] for Main Menu                            |", // 11
     "|                                                                             |", // 12
     "|                                                                             |", // 13
     "|                                                                             |", // 14
     "|                                                                             |", // 15
     "|                                                                             |", // 16
     "|                                                                             |", // 17
     "|                                                                             |", // 18
     "|                                                                             |", // 19
     "|                                                                             |", // 20
     "|                                                                             |", // 21
     "|                                                                             |", // 22
     "|                                                                             |", // 23
     "==============================================================================="  // 24
};
static Screen pauseScreen(pauseData);

// Victory screen data
const char *victoryData[MAX_Y] = {
    //0123456789012345678901234567890123456789012345678901234567890123456789012345678
     "===============================================================================", // 0
     "|                                                                             |", // 1
     "|                                                                             |", // 2
     "|                                                                             |", // 3
     "|                                                                             |", // 4
     "|                           *****  VICTORY!  *****                            |", // 5
     "|                                                                             |", // 6
     "|                   Congratulations! You completed the game!                  |", // 7
     "|                                                                             |", // 8
     "|                     Both players reached the final room.                    |", // 9
     "|                                                                             |", // 10
     "|                                                                             |", // 11
     "|                         Press any key to continue                           |", // 12
     "|                                                                             |", // 13
     "|                                                                             |", // 14
     "|                                                                             |", // 15
     "|                                                                             |", // 16
     "|                                                                             |", // 17
     "|                                                                             |", // 18
     "|                                                                             |", // 19
     "|                                                                             |", // 20
     "|                                                                             |", // 21
     "|                                                                             |", // 22
     "|                                                                             |", // 23
     "==============================================================================="  // 24
};
static Screen victoryScreen(victoryData);

// Game Over screen data
const char *gameOverData[MAX_Y] = {
    //0123456789012345678901234567890123456789012345678901234567890123456789012345678
     "===============================================================================", // 0
     "|                                                                             |", // 1
     "|                                                                             |", // 2
     "|                                                                             |", // 3
     "|                                                                             |", // 4
     "|                              *** GAME OVER ***                              |", // 5
     "|                                                                             |", // 6
     "|                                                                             |", // 7
     "|                      The bomb destroyed something vital!                    |", // 8
     "|                                                                             |", // 9
     "|                                                                             |", // 10
     "|                                                                             |", // 11
     "|                          Press any key to continue                          |", // 12
     "|                                                                             |", // 13
     "|                                                                             |", // 14
     "|                                                                             |", // 15
     "|                                                                             |", // 16
     "|                                                                             |", // 17
     "|                                                                             |", // 18
     "|                                                                             |", // 19
     "|                                                                             |", // 20
     "|                                                                             |", // 21
     "|                                                                             |", // 22
     "|                                                                             |", // 23
     "==============================================================================="  // 24
};
static Screen gameOverScreen(gameOverData);

// Room 0 - First room with switches
const char *room0Data[MAX_Y] = {
    //0123456789012345678901234567890123456789012345678901234567890123456789012345678
     "WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW",  // 0
     "W                                                  #            W             W",  // 1
     "W                                                  #            WWWW    W   WWW",  // 2
     "W                                                  #            W      WW     W",  // 3
     "W                                                  #            W   WWWWWWW   W",  // 4
     "W                                                               W      W      W",  // 5
     "W                                                               WWWW \\ W      W", // 6
     "W                                                               WWWWWWWW      W",  // 7
     "W                                                                             W",  // 8
     "W                                                                        #####W",  // 9
     "W###                                                                          W",  // 10
     "W                                                                             W",  // 11
     "WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW                             W",  // 12
     "W                                               W                             W",  // 13
     "W        WWWWWWWWWWWWWWWWWWWWW                  W                             W",  // 14
     "W        W                   W                  W                             W",  // 15
     "W        W     WWWWWWWW      W                  W                             W",  // 16
     "W        W          \\ W      W                  W                             1", // 17
     "W        WWWWWWWWWWWWWW      W     WWWWWWWWWWWWWW             #               W",  // 18
     "W                            W                                #               W",  // 19
     "WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW",  // 20
     "W                               | INVENTORY |                                 W",  // 21
     "W       PLAYER 1 ($):            -----------  PLAYER 2 (&):                   W",  // 22
     "W                                     |                                       W",  // 23
     "WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW"   // 24
};
static Screen room0Layout(room0Data);

// Room 1 - Bomb puzzle room with dark zone
const char *room1Data[MAX_Y] = {
    //0123456789012345678901234567890123456789012345678901234567890123456789012345678
     "WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW",  // 0
     "W                                                                             W",  // 1
     "W                                                                             W",  // 2
     "W                                                                             W",  // 3
     "W                  WWWWWWWWWWWWWWWWWWWWWWWWWWWWW             WWWWWWWWWWWWWWWWWW",  // 4
     "0                  W         W                 W             W                W",  // 5
     "W                  W         W   K             W             W    WWWWWWWWW   W",  // 6
     "W                  W         WWWWWWWW          W             W    W   K   W   W",  // 7
     "W                  W                           W             W            W   W",  // 8
     "W                  W                           W             WWWWWWWWWWWWWW   W",  // 9
     "W                  WWWWWWW  WWWWWWWWWWWWWWWWWWWW                              W",  // 10
     "W                  W                           =                              W",  // 11
     "W                  W                           =                              W",  // 12
     "W                  W                           =                              W",  // 13
     "W                  W\\                          =                              W", // 14
     "W                  WWWWWWWWWWWWWWWWWWWWWWWWWWWWW                              W",  // 15
     "W                                                                    WZZZZZZZZW",  // 16
     "W                  !                                                 W        W",  // 17
     "W                                                                    W        2",  // 18
     "W                  @                                                 W        W",  // 19
     "WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW",  // 20
     "W                               | INVENTORY |                                 W",  // 21
     "W       PLAYER 1 ($):            -----------  PLAYER 2 (&):                   W",  // 22
     "W                                     |                                       W",  // 23
     "WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW"   // 24
};
static Screen room1Layout(room1Data);

// Function to get layout by ID
inline Screen *getLayoutById(int id)
{
    switch (id)
    {
    case -2:
        return &mainMenuScreen;
    case -1:
        return &instructionsScreen;
    case -3:
        return &pauseScreen;
    case -4:
        return &victoryScreen;
    case -5:
        return &gameOverScreen;
    case 0:
        return &room0Layout;
    case 1:
        return &room1Layout;
    default:
        return &room0Layout;
    }
}
