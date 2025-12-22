#pragma once

//////////////////////////////////////////       INCLUDES & FORWARDS       //////////////////////////////////////////

#include "Constants.h"
#include "Screen.h"
#include "Player.h"
#include "Room.h"
#include <vector>

//////////////////////////////////////////           Game             //////////////////////////////////////////

// Forward declarations
class Riddle;

// Main game controller
class Game
{
private:
    struct ActiveRiddle
    {
        Riddle* riddle;
        Player* player;

        ActiveRiddle() : riddle(nullptr), player(nullptr) {}

        void reset() {
            riddle = nullptr;
            player = nullptr;
        }

        bool isActive() const {
            return riddle != nullptr;
        }
    };

    ActiveRiddle aRiddle;  // Track currently active riddle

public:
    GameState currentState;
    std::vector<Room> rooms;
    int currentRoomId;
    int finalRoomId;
    Player player1;
    Player player2;
    bool gameInitialized;  // Track if game is already started

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
