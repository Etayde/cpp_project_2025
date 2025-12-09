
#pragma once
//////////////////////////////////////////       INCLUDES & FORWARDS       //////////////////////////////////////////

#include "Constants.h"
#include "Screen.h"
#include "Player.h"
#include "Room.h"

//////////////////////////////////////////           Game             //////////////////////////////////////////

// Main game controller - manages state, rooms, and gameplay
class Game
{
public:
    GameState currentState;
    Room rooms[TOTAL_ROOMS];
    int currentRoomId;
    Player player1;
    Player player2;

public:
    Game();
    ~Game();

    // Main game loop
    void run();

    // Menu handlers
    void showMainMenu();
    void handleMainMenuInput();
    void showInstructions();
    void handleInstructionsInput();
    void showPauseMenu();
    void handlePauseInput();
    void showVictory();
    void showGameOver();

    // Game logic
    void initializeRooms();
    void startNewGame();
    void gameLoop();
    void handleInput();
    void update();

    // Room management
    void changeRoom(int newRoomId, bool goingForward);
    Room *getCurrentRoom();
    void checkRoomTransitions();
    void redrawCurrentRoom();
};
