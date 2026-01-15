#include "LoadedGame.h"
#include "Recorder.h"
#include <string>
#include <fstream>

using namespace std;

LoadedGame::LoadedGame(const string& filename, bool silent) : Game(), steps()
{
    silentMode = silent;  // Set the flag inherited from Game
    Renderer::setSilentMode(silentMode);  // Update renderer mode

    initErrorMessage = loadActions(filename);
    if (initErrorMessage != ErrorCode::NONE) 
        currentState = GameState::error;
}

LoadedGame::LoadedGame(int argc, char* argv[]) : Game(), steps()
{
    // Parse arguments specific to LoadedGame
    bool silent = false;

    for (int i = 1; i < argc; i++)
    {
        std::string arg(argv[i]);

        if (arg == "-silent")
        {
            silent = true;
        }
        // -load flag doesn't need to be checked (factory already determined this is LoadedGame)
    }

    // Set silent mode (before loading actions)
    silentMode = silent;
    Renderer::setSilentMode(silentMode);

    // Load actions from hardcoded filename: "adv-world.steps.txt"
    initErrorMessage = loadActions("adv-world.steps.txt");
    
    if (initErrorMessage != ErrorCode::NONE)
        currentState = GameState::error;
    //     // File doesn't exist or couldn't be loaded
        
    //     // Error message will be displayed by Game::run()
    
}

void LoadedGame::handleInput() 
{
    ActionRecord curr = steps.getActionAt(steps.getCurrIndex());

    if (curr.action == Action::ESC)
    {
        currentState = GameState::paused;
        return;
    }

    Player &player = (curr.playerId == 1) ? player1 : player2;

    steps.advanceToNextAction();

    player.performAction(curr.action, getCurrentRoom());
}
    

void LoadedGame::run()
{
  bool running = true;

  while (running)
  {
    switch (currentState)
    {

    case GameState::inGame:
      startNewGame();
      gameLoop();
      break;

    case GameState::victory:
      showVictory();
      Renderer::sleep_ms(2000);
      gameInitialized = false;
      currentState = GameState::quit;
      break;

    case GameState::gameOver:
      showGameOver();
      Renderer::sleep_ms(2000);
      gameInitialized = false;
      currentState = GameState::quit;
      break;

    case GameState::error:
      showErrorScreen();
      Renderer::sleep_ms(2000);
      gameInitialized = false;
      currentState = GameState::quit;
      break;

    case GameState::quit:
      running = false;
      break;

    default:
      break;
    }
  }
}
