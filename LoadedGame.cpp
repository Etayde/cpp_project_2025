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
    silentMode = silent;
    Renderer::setSilentMode(silentMode);

    initErrorMessage = loadActions(filename);
    if (initErrorMessage != ErrorCode::NONE) {
        currentState = GameState::error;
        return;
    }
    
    unsigned int seed = steps.getRandomSeed();
    if (seed == 0) {
        initErrorMessage = ErrorCode::MISSING_RANDOM_SEED;
        currentState = GameState::error;
        return;
    }
    
    initializeRooms(seed);
    currentState = GameState::inGame;

    if (currentState == GameState::inGame)
    {
        ErrorCode resultError = loadExpectedResults("adv-world.result.txt");
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
    bool silent = false;

    for (int i = 1; i < argc; i++)
    {
        std::string arg(argv[i]);

        if (arg == "-silent")
        {
            silent = true;
        }
    }

    silentMode = silent;
    Renderer::setSilentMode(silentMode);

    initErrorMessage = loadActions("adv-world.steps.txt");
    if (initErrorMessage != ErrorCode::NONE) {
        currentState = GameState::error;
        return;
    }
    
    unsigned int seed = steps.getRandomSeed();
    if (seed == 0) {
        initErrorMessage = ErrorCode::MISSING_RANDOM_SEED;
        currentState = GameState::error;
        return;
    }
    
    initializeRooms(seed);
    setCurrentState(GameState::inGame);

    if (currentState == GameState::inGame)
    {
        ErrorCode resultError = loadExpectedResults("adv-world.result.txt");
        if (silentMode && resultError != ErrorCode::NONE)
        {
            initErrorMessage = resultError;
            currentState = GameState::error;
        }
    }
}

void LoadedGame::handleInput()
{
    const ActionRecord* curr = steps.getCurrentAction();

    while (curr != nullptr && curr->cycle == cycleCount)
    {
        if (curr->action != Action::ANSWER_RIDDLE) {
            Player& player = (curr->playerId == 1) ? player1 : player2;
            player.performAction(curr->action, getCurrentRoom());
        }

        steps.advanceToNextAction();
        curr = steps.getCurrentAction();
    }
}

void LoadedGame::gameLoop()
{
    if (getCurrentRoomId() == 0 && cycleCount == 0)
    {
        reportScreenChange(0);
    }

    Room *room = getCurrentRoom();

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
        if (shouldQuit())
        {
            currentState = GameState::quit;
            return;
        }

        handleInput();
        update();
        Renderer::sleep_ms(50);
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
        if (testPassed && expectedEventIndex < expectedEvents.size())
        {
          testFailed("Expected more events (got " + std::to_string(expectedEventIndex) +
                     ", expected " + std::to_string(expectedEvents.size()) + ")");
        }

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
        reportQuit();  // Verify quit event against expected

        if (testPassed && expectedEventIndex < expectedEvents.size())
        {
          testFailed("Expected more events (got " + std::to_string(expectedEventIndex) +
                     ", expected " + std::to_string(expectedEvents.size()) + ")");
        }

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
    quitCycle = -1;
    while (file >> std::ws && file.peek() != EOF)
    {
        GameEvent event;
        if (!event.read(file))
        {
            file.close();
            return ErrorCode::READ_ERROR;
        }

        if (event.type == GameEventType::QUIT)
        {
            quitCycle = static_cast<long>(event.cycle);
        }

        expectedEvents.push_back(event);
    }

    file.close();
    return ErrorCode::NONE;
}

///////////////////////////////////////////    verifyEvent    /////////////////////////////////////////////


void LoadedGame::changeRoom(int newRoomId, bool goingForward)
{
    Game::changeRoom(newRoomId, goingForward);
    
    if (newRoomId >= 0 && newRoomId < static_cast<int>(rooms.size())) {
        reportScreenChange(newRoomId);
    }
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

    if (expected.cycle != actual.cycle)
    {
        testFailed("Event at wrong cycle: expected " + std::to_string(expected.cycle) +
                   ", got " + std::to_string(actual.cycle));
        return false;
    }

    if (expected.type != actual.type)
    {
        testFailed("Event type mismatch at cycle " + std::to_string(actual.cycle));
        return false;
    }

    if (expected.roomId != actual.roomId)
    {
        testFailed("Event room mismatch at cycle " + std::to_string(actual.cycle) +
                   ": expected room " + std::to_string(expected.roomId) +
                   ", got room " + std::to_string(actual.roomId));
        return false;
    }

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
    if (testPassed)
    {
        testPassed = false;
        testFailureDetails = details;
    }
}

///////////////////////////////////////////    checkMissedEvents    /////////////////////////////////////////////

void LoadedGame::checkMissedEvents()
{
    while (expectedEventIndex < expectedEvents.size() &&
           expectedEvents[expectedEventIndex].cycle == cycleCount)
    {
        testFailed("Expected event at cycle " + std::to_string(cycleCount) + " did not occur");
        return;
    }
}

///////////////////////////////////////////    reportScreenChange    /////////////////////////////////////////////

void LoadedGame::reportScreenChange(int roomId)
{
    if (!silentMode)
        return;

    GameEvent actual(cycleCount, roomId);
    verifyEvent(actual);
}

///////////////////////////////////////////    reportLifeLost    /////////////////////////////////////////////

void LoadedGame::reportLifeLost(int playerId)
{
    if (!silentMode)
        return;

    GameEvent actual(cycleCount, currentRoomId, playerId);
    verifyEvent(actual);
}

///////////////////////////////////////////    onRiddleAttempt    /////////////////////////////////////////////

void LoadedGame::onRiddleAttempt(const std::string& question, int answer, bool correct)
{
    if (!silentMode)
        return;

    GameEvent actual(cycleCount, currentRoomId, question, answer, correct);
    verifyEvent(actual);
}

///////////////////////////////////////////    reportQuit    /////////////////////////////////////////////

void LoadedGame::reportQuit()
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

///////////////////////////////////////////    getRiddleInput    /////////////////////////////////////////////

int LoadedGame::getRiddleInput(unsigned long cycle)
{
    vector<ActionRecord> actions = steps.getActionsForCycle(cycle);
    for (const auto& action : actions)
    {
        if (action.action == Action::ANSWER_RIDDLE)
        {
            return action.answer;
        }
    }
    return -1;
}
