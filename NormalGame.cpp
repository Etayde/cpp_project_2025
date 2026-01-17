#include "NormalGame.h"
#include "Console.h"
#include "Constants.h"
#include <string>

//////////////////////////////////////////     NormalGame Constructor     /////////////////////////////////////////////

NormalGame::NormalGame() : Game() {}

//////////////////////////////////////////     NormalGame Constructor (with args)     /////////////////////////////////////////////

NormalGame::NormalGame(int argc, char* argv[]) : NormalGame()
{
    bool saveMode = false;

    for (int i = 1; i < argc; i++)
    {
        std::string arg(argv[i]);

        if (arg == "-save")
        {
            saveMode = true;
        }
    }

    if (saveMode)
    {
        enableRecording("adv-world.steps.txt");
        resultFile.open("adv-world.result.txt");
    }
}

//////////////////////////////////////////     NormalGame Destructor     /////////////////////////////////////////////

NormalGame::~NormalGame()
{
    disableRecording();
    if (resultFile.is_open()) resultFile.close();
    
    if (consoleInitialized)
    {
      clrscr();
      cleanup_console();
    }  
}

//////////////////////////////////////////        handleInput         /////////////////////////////////////////////

void NormalGame::handleInput()
{

  while (check_kbhit())
  {
    int pressed = get_char_nonblocking();

    if (pressed == -1)
      break;

    // Detect and ignore special key sequences (arrow keys, function keys, etc.)
    // On Windows, special keys send 0 or 224 followed by a key code (apperently
    // :) )
    if (pressed == 0 || pressed == 224)
    {
      if (check_kbhit())
      {
        get_char_nonblocking();
      }
      continue;
    }

    for (int i = 0; i < NUM_KEY_BINDINGS; i++)
    {
      if (keyBindings[i].key == pressed)
      {
        if (keyBindings[i].action == Action::ESC)
        {
          currentState = GameState::paused;
          return;
        }

        recordAction(keyBindings[i]);  // Only record non-ESC actions

        Player &player = (keyBindings[i].playerID == 1) ? player1 : player2;

        player.performAction(keyBindings[i].action, getCurrentRoom());
        break;
      }
    }
  }
}



void NormalGame::recordAction(const PlayerKeyBinding& binding)
{
    if (!isRecording || !recordFile.is_open())
        return;

    ActionRecord record(cycleCount, binding);

    record.write(recordFile);
    recordFile.flush();
}

///////////////////////////////////////////    enableRecording    /////////////////////////////////////////////

void NormalGame::enableRecording(const string &filename)
{
    recordFile.open(filename);
    if (recordFile.is_open()) isRecording = true;
}

///////////////////////////////////////////    disableRecording    /////////////////////////////////////////////

void NormalGame::disableRecording()
{
    if (recordFile.is_open()) recordFile.close();
    isRecording = false;
}

///////////////////////////////////////////    recordScreenChange    /////////////////////////////////////////////

void NormalGame::recordScreenChange(int roomId)
{
    if (resultFile.is_open())
    {
        GameEvent event(cycleCount, roomId);
        event.write(resultFile);
        resultFile.flush();
    }
}

///////////////////////////////////////////    recordLifeLost    /////////////////////////////////////////////

void NormalGame::recordLifeLost(int playerId)
{
    if (!resultFile.is_open())
        return;

    GameEvent event(cycleCount, currentRoomId, playerId);
    event.write(resultFile);
    resultFile.flush();
}

///////////////////////////////////////////    recordRiddleAttempt    /////////////////////////////////////////////

void NormalGame::recordRiddleAttempt(const std::string& question, int answer, bool correct)
{
    if (!resultFile.is_open())
        return;

    GameEvent event(cycleCount, currentRoomId, question, answer, correct);
    event.write(resultFile);
    resultFile.flush();
}

//////////////////////////////////////////     handlePauseInput     /////////////////////////////////////////////

void NormalGame::handlePauseInput()
{
  if (check_kbhit())
  {
    char choice = get_single_char();
    if (choice == char(Action::ESC))
      currentState = GameState::inGame;
    else if (choice == 'h' || choice == 'H')
    {
      recordQuit();
      gameInitialized = false;
      currentState = GameState::mainMenu;
    }
  }
}

///////////////////////////////////////////    recordQuit    /////////////////////////////////////////////

void NormalGame::recordQuit()
{
    if (!resultFile.is_open())
        return;

    GameEvent event(cycleCount, currentRoomId, GameEventType::QUIT);
    event.write(resultFile);
    resultFile.flush();
}

///////////////////////////////////////////    recordRiddleAnswer    /////////////////////////////////////////////

void NormalGame::recordRiddleAnswer(int answer)
{
    if (!isRecording || !recordFile.is_open())
        return;
    
    int playerId = 1;
    if (aRiddle.isActive() && aRiddle.player != nullptr) {
        playerId = aRiddle.player->playerId;
    }
    
    ActionRecord ar(cycleCount, playerId, answer);
    ar.write(recordFile);
    recordFile.flush();
}