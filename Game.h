#pragma once

//////////////////////////////////////////       INCLUDES & FORWARDS       /////////////////////////////////////////////

#include "Constants.h"
#include "Player.h"
#include "Room.h"
#include "Screen.h"
#include <vector>

//////////////////////////////////////////           Game       /////////////////////////////////////////////

// Forward declarations
class Riddle;

enum class GameOverMessege
{
  NONE,
  PLAYER1_DIED,
  PLAYER2_DIED,
  VALUABLE_DESTROYED
};

// Main game controller
class Game
{
protected:
  int initErrorMessage;
  int initErrorRoomId;
  GameOverMessege gameOverMessege;
  
  Game();

  struct ActiveRiddle
  {
    Riddle *riddle;
    Player *player;

    ActiveRiddle() : riddle(nullptr), player(nullptr) {}

    void reset()
    {
      riddle = nullptr;
      player = nullptr;
    }

    bool isActive() const { return riddle != nullptr; }
  };

  ActiveRiddle aRiddle;                // Track currently active riddle
  std::vector<Screen *> loadedScreens; // Screens loaded from files

  void setGameOverMessege(GameOverMessege messege) { gameOverMessege = messege; }

public:
  GameState currentState;
  std::vector<Room> rooms;
  int currentRoomId;
  int finalRoomId;
  Player player1;
  Player player2;
  bool gameInitialized; // Track if game is already started

  virtual ~Game();

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
  void showErrorScreen();

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
  int validateLegendPlacement(Room &room);
  bool checkGameOver(const ExplosionResult &result);
};
