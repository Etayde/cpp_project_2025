#pragma once
#include "Constants.h"
#include "Screen.h"
#include "Player.h"
#include "Room.h"

const int BOMB_FUSE_TIME = 50;  // ~5 seconds at 100ms per tick
const int BOMB_RADIUS = 5;

class Game {
public:
    GameState currentState;
    Room rooms[TOTAL_ROOMS];
    int currentRoomId;
    Player player1;
    Player player2;
    bool player1DropRequested;
    bool player2DropRequested;

    // Constructor
    Game();
    
    // Destructor
    ~Game();

    // State management
    void setState(GameState newState) { currentState = newState; }
    GameState getState() const { return currentState; }

    // Game flow
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
    void render();
    
    // Room management
    void changeRoom(int newRoomId, bool goingForward);
    Room* getCurrentRoom();
    
    // Check if both players are at a door and handle transition
    // Returns: 0 = not at door, positive = door to next room, negative = door to previous room
    int checkPlayersAtDoor();
    
    // Handle bomb drop
    void handleBombDrop(Player& player);
    
    // Update bomb timer for current room and explode if needed
    void updateBomb();
};
