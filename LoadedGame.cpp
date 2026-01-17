#include "LoadedGame.h"
#include "Recorder.h"
#include "Layouts.h"
#include <string>
#include <fstream>
#include <iostream>

using namespace std;

LoadedGame::LoadedGame(const string& filename, bool silent) : Game(), steps(),
    expectedEventIndex(0), testPassed(true), quitCycle(-1)
{
    silentMode = silent;  // Set the flag inherited from Game
    Renderer::setSilentMode(silentMode);  // Update renderer mode

    initErrorMessage = loadActions(filename);
    if (initErrorMessage != ErrorCode::NONE)
        currentState = GameState::error;
    else
        currentState = GameState::inGame;

    // Load expected results (needed for quit detection in all modes, verification in silent mode)
    if (currentState == GameState::inGame)
    {
        ErrorCode resultError = loadExpectedResults("adv-world.result.txt");
        // In silent mode, missing results file is an error
        // In non-silent mode, just continue without quit detection (quitCycle stays -1)
        if (silentMode && resultError != ErrorCode::NONE)
        {
            initErrorMessage = resultError;
            currentState = GameState::error;
        }
    }
}

LoadedGame::LoadedGame(int argc, char* argv[]) : Game(), steps(),
    expectedEventIndex(0), testPassed(true), quitCycle(-1)
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
        {currentState = GameState::inGame;
        cout << "Loaded steps" << endl;             ////////////////////
        cout << " currentState = " << static_cast<int>(currentState) << endl; 
        }            ////////////////////
    // Load expected results (needed for quit detection in all modes, verification in silent mode)
    if (currentState == GameState::inGame)
    {
        cout << "Loading expected results" << endl;             ////////////////////
        ErrorCode resultError = loadExpectedResults("adv-world.result.txt");
        // In silent mode, missing results file is an error
        // In non-silent mode, just continue without quit detection (quitCycle stays -1)
        if (silentMode && resultError != ErrorCode::NONE)
        {
            initErrorMessage = resultError;
            currentState = GameState::error;
        }
    }
    else
    {
        cout << "skipping expected results load" << endl;             ////////////////////
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

void LoadedGame::gameLoop()
{
    Room *room = getCurrentRoom();

    // Normal game start - draw room and start game updates
    if (room)
    {
        room->draw();
    }
    player1.draw(room);
    player2.draw(room);
    if (room)
        room->drawLegend(&player1, &player2);

    while (currentState == GameState::inGame)
    {
        // Check for quit condition
        if (shouldQuit())
        {
            currentState = GameState::quit;
            return;
        }

        handleInput();
        update();
        Renderer::sleep_ms(100);
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

    case GameState::paused:

      break;
    case GameState::quit:
      if (silentMode)
      {
        recordQuit();  // Verify QUIT event against expected

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
        showQuitScreen();
        Renderer::sleep_ms(2000);
      }
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
        cout << "Could not open expected results file: " << filename << endl;
        return ErrorCode::FILE_NOT_FOUND;
    }

    expectedEvents.clear();
    expectedEventIndex = 0;
    quitCycle = -1;  // Reset quit cycle
    cout << "Expected events resetted" << endl;             ////////////////////

    while (file >> std::ws && file.peek() != EOF)
    {
        GameEvent event;
        if (!event.read(file))
        {
            cout << "Error reading expected event from file." << endl;
            file.close();
            return ErrorCode::READ_ERROR;
        }

        // Check for QUIT event and store the quit cycle
        if (event.type == GameEventType::QUIT)
        {
            quitCycle = static_cast<long>(event.cycle);
        }

        expectedEvents.push_back(event);
        cout << "Loaded expected event: cycle " << event.cycle << ", type "
             << static_cast<int>(event.type) << ", room " << event.roomId << endl;
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

///////////////////////////////////////////    recordQuit    /////////////////////////////////////////////

void LoadedGame::recordQuit()
{
    if (!silentMode)
        return;

    GameEvent actual(cycleCount, currentRoomId, GameEventType::QUIT);
    verifyEvent(actual);
}

///////////////////////////////////////////    showQuitScreen    /////////////////////////////////////////////

void LoadedGame::showQuitScreen()
{
    Renderer::clrscr();
    quitScreen.draw();
}
