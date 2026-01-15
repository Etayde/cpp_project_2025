#include "LoadedGame.h"
#include "Recorder.h"
#include <string>
#include <fstream>
#include <iostream>

using namespace std;

LoadedGame::LoadedGame(const string& filename, bool silent) : Game(), steps(),
    expectedEventIndex(0), testPassed(true)
{
    silentMode = silent;  // Set the flag inherited from Game
    Renderer::setSilentMode(silentMode);  // Update renderer mode

    initErrorMessage = loadActions(filename);
    if (initErrorMessage != ErrorCode::NONE)
        currentState = GameState::error;
    else
        currentState = GameState::inGame;

    // Load expected results in silent mode
    if (silentMode && currentState == GameState::inGame)
    {
        loadExpectedResults("adv-world.result");
    }
}

LoadedGame::LoadedGame(int argc, char* argv[]) : Game(), steps(),
    expectedEventIndex(0), testPassed(true)
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
    else
        currentState = GameState::inGame;

    // Load expected results in silent mode
    if (silentMode && currentState == GameState::inGame)
    {
        loadExpectedResults("adv-world.result");
    }
}

void LoadedGame::handleInput()
{
    const ActionRecord* curr = steps.getCurrentAction();

    // Process all actions that match the current cycle
    while (curr != nullptr && curr->cycle == cycleCount)
    {
        Player& player = (curr->playerId == 1) ? player1 : player2;
        player.performAction(curr->action, getCurrentRoom());

        steps.advanceToNextAction();
        curr = steps.getCurrentAction();
    }
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
    case GameState::gameOver:
      if (silentMode)
      {
        // Check if all expected events were consumed
        if (testPassed && expectedEventIndex < expectedEvents.size())
        {
          testFailed("Expected more events (got " + std::to_string(expectedEventIndex) +
                     ", expected " + std::to_string(expectedEvents.size()) + ")");
        }

        // Output test result
        if (testPassed)
        {
          std::cout << "Test passed" << std::endl;
        }
        else
        {
          std::cout << "Test not passed" << std::endl;
          std::cout << testFailureDetails << std::endl;
        }
      }
      else
      {
        if (currentState == GameState::victory)
          showVictory();
        else
          showGameOver();
        Renderer::sleep_ms(2000);
      }
      gameInitialized = false;
      currentState = GameState::quit;
      break;

    case GameState::error:
      if (silentMode)
      {
        std::cout << "Test not passed" << std::endl;
        std::cout << "Error during initialization" << std::endl;
      }
      else
      {
        showErrorScreen();
        Renderer::sleep_ms(2000);
      }
      gameInitialized = false;
      currentState = GameState::quit;
      break;

    case GameState::quit:
      running = false;
      break;

    default:
      currentState = GameState::quit;
      break;
    }
  }
}

///////////////////////////////////////////    loadExpectedResults    /////////////////////////////////////////////

ErrorCode LoadedGame::loadExpectedResults(const string& filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        return ErrorCode::FILE_NOT_FOUND;
    }

    expectedEvents.clear();
    expectedEventIndex = 0;

    while (file >> std::ws && file.peek() != EOF)
    {
        GameEvent event;
        if (!event.read(file))
        {
            file.close();
            return ErrorCode::READ_ERROR;
        }
        expectedEvents.push_back(event);
    }

    file.close();
    return ErrorCode::NONE;
}

///////////////////////////////////////////    verifyEvent    /////////////////////////////////////////////

bool LoadedGame::verifyEvent(const GameEvent& actual)
{
    if (expectedEventIndex >= expectedEvents.size())
    {
        testFailed("Unexpected event at cycle " + std::to_string(actual.cycle));
        return false;
    }

    const GameEvent& expected = expectedEvents[expectedEventIndex];

    // Check cycle matches
    if (expected.cycle != actual.cycle)
    {
        testFailed("Event at wrong cycle: expected " + std::to_string(expected.cycle) +
                   ", got " + std::to_string(actual.cycle));
        return false;
    }

    // Check type matches
    if (expected.type != actual.type)
    {
        testFailed("Event type mismatch at cycle " + std::to_string(actual.cycle));
        return false;
    }

    // Check room matches
    if (expected.roomId != actual.roomId)
    {
        testFailed("Event room mismatch at cycle " + std::to_string(actual.cycle) +
                   ": expected room " + std::to_string(expected.roomId) +
                   ", got room " + std::to_string(actual.roomId));
        return false;
    }

    // Check type-specific data
    switch (actual.type)
    {
    case GameEventType::LIFE_LOST:
        if (expected.playerId != actual.playerId)
        {
            testFailed("Life lost player mismatch at cycle " + std::to_string(actual.cycle) +
                       ": expected player " + std::to_string(expected.playerId) +
                       ", got player " + std::to_string(actual.playerId));
            return false;
        }
        break;
    case GameEventType::RIDDLE_ANSWERED:
        if (expected.question != actual.question ||
            expected.answerGiven != actual.answerGiven ||
            expected.wasCorrect != actual.wasCorrect)
        {
            testFailed("Riddle mismatch at cycle " + std::to_string(actual.cycle));
            return false;
        }
        break;
    default:
        break;
    }

    expectedEventIndex++;
    return true;
}

///////////////////////////////////////////    testFailed    /////////////////////////////////////////////

void LoadedGame::testFailed(const std::string& details)
{
    if (testPassed)  // Only record first failure
    {
        testPassed = false;
        testFailureDetails = details;
    }
}

///////////////////////////////////////////    checkMissedEvents    /////////////////////////////////////////////

void LoadedGame::checkMissedEvents()
{
    // Check if any expected events at current cycle were missed
    while (expectedEventIndex < expectedEvents.size() &&
           expectedEvents[expectedEventIndex].cycle == cycleCount)
    {
        testFailed("Expected event at cycle " + std::to_string(cycleCount) + " did not occur");
        return;  // Stop checking after first missed event
    }
}

///////////////////////////////////////////    recordScreenChange    /////////////////////////////////////////////

void LoadedGame::recordScreenChange(int roomId)
{
    if (!silentMode)
        return;

    GameEvent actual(cycleCount, roomId);
    verifyEvent(actual);
}

///////////////////////////////////////////    recordLifeLost    /////////////////////////////////////////////

void LoadedGame::recordLifeLost(int playerId)
{
    if (!silentMode)
        return;

    GameEvent actual(cycleCount, currentRoomId, playerId);
    verifyEvent(actual);
}

///////////////////////////////////////////    recordRiddleAttempt    /////////////////////////////////////////////

void LoadedGame::recordRiddleAttempt(const std::string& question, int answer, bool correct)
{
    if (!silentMode)
        return;

    GameEvent actual(cycleCount, currentRoomId, question, answer, correct);
    verifyEvent(actual);
}
