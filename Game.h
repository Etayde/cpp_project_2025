#pragma once

//////////////////////////////////////////       INCLUDES & FORWARDS       //////////////////////////////////////////

#include "Constants.h"
#include "Screen.h"
#include "Player.h"
#include "Room.h"
#include <vector>

//////////////////////////////////////////           Game             //////////////////////////////////////////

// Main game controller
class Game
{
public:
    GameState currentState;
    std::vector<Room> rooms;
    int currentRoomId;
    int finalRoomId;
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
    bool canPassThroughDoor(Room *room, int doorId);
};
