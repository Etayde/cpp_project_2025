#pragma once

//////////////////////////////////////////       INCLUDES & FORWARDS       /////////////////////////////////////////////

#include "Constants.h"
#include "Player.h"
#include "Room.h"
#include "Screen.h"
#include "Renderer.h"
#include <vector>

//////////////////////////////////////////           Game       /////////////////////////////////////////////

class Riddle;

enum class GameOverMessege
{
  NONE,
  PLAYER1_DIED,
  PLAYER2_DIED,
  VALUABLE_DESTROYED
};

class Game
{
protected:
  bool silentMode;
  bool consoleInitialized;  // Track whether we initialized console
  ErrorCode initErrorMessage;
  int initErrorRoomId;
  GameOverMessege gameOverMessege;
  unsigned long cycleCount;

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
  std::vector<Screen *> loadedScreens; 

  void setGameOverMessege(GameOverMessege messege) { gameOverMessege = messege; }

  void updateCycleCount() { cycleCount++; }

public:
  GameState currentState;
  std::vector<Room> rooms;
  int currentRoomId;
  int finalRoomId;
  Player player1;
  Player player2;
  bool gameInitialized;

  virtual ~Game();

  // Factory method to create appropriate game type from command-line args
  static Game* createFromArgs(int argc, char* argv[]);

  void run();

  // Menu handlers
  virtual void showMainMenu();
  void handleMainMenuInput();
  virtual void showInstructions();
  void handleInstructionsInput();
  virtual void showPauseMenu();
  void handlePauseInput();
  void showVictory();
  void showGameOver();
  void showErrorScreen();

  // Game logic
  void initializeRooms();
  virtual void startNewGame();
  virtual void gameLoop();
  virtual void handleInput() = 0;
  virtual void update();

  // Room management
  void changeRoom(int newRoomId, bool goingForward);
  Room *getCurrentRoom();
  void checkRoomTransitions();
  void redrawCurrentRoom();
  bool canPassThroughDoor(Room *room, int doorId);
  ErrorCode validateLegendPlacement(Room &room);
  bool checkGameOver(const ExplosionResult &result);
};
