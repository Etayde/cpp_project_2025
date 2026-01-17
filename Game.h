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

  virtual void run();

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

  // Accessors for event recording
  unsigned long getCycleCount() const { return cycleCount; }
  int getCurrentRoomId() const { return currentRoomId; }

  // Event recording hooks - overridden by NormalGame (save) and LoadedGame (verify)
  virtual void recordScreenChange(int roomId) { (void)roomId; }
  virtual void recordLifeLost(int playerId) { (void)playerId; }
  virtual void recordRiddleAttempt(const std::string& question, int answer, bool correct) {
    (void)question; (void)answer; (void)correct;
  }
  virtual void recordQuit() {}

  // New virtual methods for recording/playback of riddle answers
  virtual int getRecordedRiddleAnswer(unsigned long cycle) { (void)cycle; return -1; }
  virtual void recordRiddleAnswer(int answer) { (void)answer; }
};
