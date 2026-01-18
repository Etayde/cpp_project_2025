#pragma once

//////////////////////////////////////////       INCLUDES & FORWARDS       /////////////////////////////////////////////

#include "Constants.h"
#include "Player.h"
#include "Room.h"
#include "Screen.h"
#include "Renderer.h"
#include <vector>

class Riddle;

//////////////////////////////////////////           Game       /////////////////////////////////////////////

enum class GameOverMessege
{
  NONE,
  PLAYER1_DIED,
  PLAYER2_DIED,
  VALUABLE_DESTROYED
};

// Base class for all games - abstract
class Game
{
protected:
  bool silentMode;
  bool consoleInitialized;
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

  ActiveRiddle aRiddle;                
  std::vector<Screen *> loadedScreens; 

  void setGameOverMessege(GameOverMessege messege) { gameOverMessege = messege; }

  void updateCycleCount() { if (currentState == GameState::inGame) cycleCount++; }

public:
  GameState currentState;
  std::vector<Room> rooms;
  int currentRoomId;
  int finalRoomId;
  Player player1;
  Player player2;
  bool gameInitialized;

  virtual ~Game();

  static Game* createFromArgs(int argc, char* argv[]);


  virtual void run() = 0;

  void setCurrentState(GameState newState) { currentState = newState; }

  // Menu handlers
  virtual void showMainMenu();
  void handleMainMenuInput();
  virtual void showInstructions();
  void handleInstructionsInput();
  virtual void showPauseMenu();
  virtual void handlePauseInput();
  void showVictory();
  void showGameOver();
  void showErrorScreen();

  // Game logic
  void initializeRooms(unsigned int seed = 0);
  virtual void startNewGame();
  virtual void gameLoop() = 0;
  virtual void handleInput() = 0;
  virtual void update();

  // Room management
  virtual void changeRoom(int newRoomId, bool goingForward);
  Room *getCurrentRoom();
  void checkRoomTransitions();
  void redrawCurrentRoom();
  bool canPassThroughDoor(Room *room, int doorId);
  ErrorCode validateLegendPlacement(Room &room);
  bool checkGameOver(const ExplosionResult &result);

  // Accessors for event recording
  unsigned long getCycleCount() const { return cycleCount; }
  int getCurrentRoomId() const { return currentRoomId; }

  // Event reporting hooks (pure virtual)
  virtual void reportScreenChange(int roomId) = 0;
  virtual void reportLifeLost(int playerId) = 0;
  virtual void onRiddleAttempt(const std::string& question, int answer, bool correct) = 0;
  virtual void reportQuit() = 0;

  // Riddle interaction hooks
  virtual int getRiddleInput(unsigned long cycle) = 0;
  virtual void reportRiddleAnswer(int answer) = 0;
};
