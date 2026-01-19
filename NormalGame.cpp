#include "Game.h"
#include "NormalGame.h"
#include "Console.h"
#include "Riddle.h"
#include "Room.h"
#include "Player.h"
#include "Constants.h"
#include "Constants.h"
#include <string>
#include <random>
#include "LevelLoader.h"

//////////////////////////////////////////     NormalGame Constructor     /////////////////////////////////////////////

NormalGame::NormalGame() : Game() {}

/////////////////////////////////////     NormalGame Constructor (with args)     /////////////////////////////////////////////

NormalGame::NormalGame(int argc, char* argv[]) : NormalGame()
{
    init_console();
    hideCursor();
    clrscr();
    consoleInitialized = true;

    bool saveMode = false;

    for (int i = 1; i < argc; i++)
    {
        std::string arg(argv[i]);

        if (arg == "-save") saveMode = true;
    }

    if (saveMode)
    {
        enableRecording("adv-world.steps.txt");
        resultFile.open("adv-world.result.txt");
        
        // Generate and save seed
        unsigned int seed = std::random_device{}();
        recordFile << "RANDOM_SEED: " << seed << " SCREENS: ";
        
        std::vector<std::string> levelFiles = LevelLoader::discoverLevelFiles();
        for (size_t i = 0; i < levelFiles.size(); i++) {
            recordFile << levelFiles[i];
            if (i < levelFiles.size() - 1) recordFile << ",";
        }
        recordFile << "\n";
        
        initializeRooms(seed);
    }
    else initializeRooms();
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

//////////////////////////////////////////           run               /////////////////////////////////////////////

void NormalGame::run()
{
  bool running = true;

  while (running)
  {
    switch (currentState)
    {
    case GameState::mainMenu:
      showMainMenu();
      while (currentState == GameState::mainMenu)
      {
        handleMainMenuInput();
        Renderer::sleep_ms(50);
      }
      break;

    case GameState::instructions:
      showInstructions();
      while (currentState == GameState::instructions)
      {
        handleInstructionsInput();
        Renderer::sleep_ms(50);
      }
      break;

    case GameState::inGame:
      if (!gameInitialized) startNewGame();
      gameLoop();
      break;

    case GameState::paused:
      showPauseMenu();
      while (currentState == GameState::paused)
      {
        handlePauseInput();
        Renderer::sleep_ms(50);
      }
      break;

    case GameState::victory:
      showVictory();
      Renderer::gotoxy(20, 18);
      Renderer::print("Press any key to return to main menu");
      while (check_kbhit())
        get_single_char();
      while (!check_kbhit())
        Renderer::sleep_ms(50);
      get_single_char();
      gameInitialized = false;
      currentState = GameState::mainMenu;
      break;

    case GameState::gameOver:
      showGameOver();
      Renderer::gotoxy(20, 12);
      Renderer::print("Press any key to return to main menu");
      while (check_kbhit())
        get_single_char();
      while (!check_kbhit())
        Renderer::sleep_ms(50);
      get_single_char();
      gameInitialized = false;
      currentState = GameState::mainMenu;
      break;

    case GameState::error:
      showErrorScreen();
      Renderer::gotoxy(20, 12);
      Renderer::print("Press any key to return to main menu");
      while (check_kbhit())
        get_single_char();
      while (!check_kbhit())
        Renderer::sleep_ms(50);
      get_single_char();
      gameInitialized = false;
      currentState = GameState::mainMenu;
      break;

    case GameState::quit:
      running = false;
      break;
    }
  }
}

//////////////////////////////////////////         gameLoop           /////////////////////////////////////////////

void NormalGame::gameLoop()
{
  if (getCurrentRoomId() == 0 && cycleCount == 0) reportScreenChange(0);
    
  Room *room = getCurrentRoom();

  if (aRiddle.isActive())
  {
    Renderer::clrscr();

    if (room) room->draw();

    player1.draw(room);
    player2.draw(room);

    if (room) room->drawLegend(&player1, &player2);

    while (aRiddle.isActive() && currentState == GameState::inGame)
    {
      RiddleResult result = aRiddle.riddle->enterRiddle(room, aRiddle.player, this);

      if (result == RiddleResult::NO_RIDDLE)
      {
        room->removeObjectAt(aRiddle.riddle->getX(), aRiddle.riddle->getY());
        aRiddle.reset();
        Renderer::clrscr();

        if (room) room->draw();

        player1.draw(room);
        player2.draw(room);

        if (room) room->drawLegend(&player1, &player2);

        break;
      };
      if (result == RiddleResult::SOLVED)
      {
        room->removeObjectAt(aRiddle.riddle->getX(), aRiddle.riddle->getY());
        aRiddle.reset();
        Renderer::clrscr();

        if (room) room->draw();

        player1.draw(room);
        player2.draw(room);

        if (room) room->drawLegend(&player1, &player2);
        break;
      }

      else if (result == RiddleResult::ESCAPED)
      {
        currentState = GameState::paused;
        return; // Exit game loop to handle pause
      }

      else
      {
        aRiddle.reset();
        Renderer::clrscr();

        if (room) room->draw();

        player1.draw(room);
        player2.draw(room);
        
        if (room) room->drawLegend(&player1, &player2);

        break;
      }
    }
  }
  else
  {
    if (room) room->draw();

    player1.draw(room);
    player2.draw(room);

    if (room) room->drawLegend(&player1, &player2);
  }

  while (currentState == GameState::inGame)
  {
    handleInput();
    update();
    Renderer::sleep_ms(100);
  }
}

//////////////////////////////////////////        changeRoom          /////////////////////////////////////////////

void NormalGame::changeRoom(int newRoomId, bool goingForward)
{
    Game::changeRoom(newRoomId, goingForward);
    
    if (newRoomId >= 0 && newRoomId < static_cast<int>(rooms.size())) reportScreenChange(newRoomId);
}

//////////////////////////////////////////        handleInput         /////////////////////////////////////////////

void NormalGame::handleInput()
{

  while (check_kbhit())
  {
    int pressed = get_char_nonblocking();

    if (pressed == -1) break;

    if (pressed == 0 || pressed == 224)
    {
      if (check_kbhit()) get_char_nonblocking();
      continue;
    }

    for (int i = 0; i < NUM_KEY_BINDINGS; i++)
    {
      if (keyBindings[i].key == pressed)
      {
        if (keyBindings[i].action == Action::ESC) // Not recording ESC
        {
          currentState = GameState::paused;
          return;
        }

        recordAction(keyBindings[i]);

        Player &player = (keyBindings[i].playerID == 1) ? player1 : player2;

        player.performAction(keyBindings[i].action, getCurrentRoom());
        break;
      }
    }
  }
}



void NormalGame::recordAction(const PlayerKeyBinding& binding)
{
    if (!isRecording || !recordFile.is_open()) return;

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

///////////////////////////////////////////    reportScreenChange    /////////////////////////////////////////////

void NormalGame::reportScreenChange(int roomId)
{
    if (resultFile.is_open())
    {
        GameEvent event(cycleCount, roomId);
        event.write(resultFile);
        resultFile.flush();
    }
}

///////////////////////////////////////////    reportLifeLost    /////////////////////////////////////////////

void NormalGame::reportLifeLost(int playerId)
{
    if (!resultFile.is_open()) return;

    GameEvent event(cycleCount, currentRoomId, playerId);
    event.write(resultFile);
    resultFile.flush();
}

///////////////////////////////////////////    onRiddleAttempt    /////////////////////////////////////////////

void NormalGame::onRiddleAttempt(const std::string& question, int answer, bool correct)
{
    if (!resultFile.is_open()) return;

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
      reportQuit();
      gameInitialized = false;
      currentState = GameState::mainMenu;
    }
  }
}

///////////////////////////////////////////    reportQuit    /////////////////////////////////////////////

void NormalGame::reportQuit()
{
    if (!resultFile.is_open()) return;

    GameEvent event(cycleCount, currentRoomId, GameEventType::QUIT);
    event.write(resultFile);
    resultFile.flush();
}

///////////////////////////////////////////    reportRiddleAnswer    /////////////////////////////////////////////

void NormalGame::reportRiddleAnswer(int answer)
{
    if (!isRecording || !recordFile.is_open()) return;
    
    int playerId = 1;
    if (aRiddle.isActive() && aRiddle.player != nullptr) playerId = aRiddle.player->getId();
    
    ActionRecord ar(cycleCount, playerId, answer);
    ar.write(recordFile);
    recordFile.flush();
}

///////////////////////////////////////////    getRiddleInput    /////////////////////////////////////////////

int NormalGame::getRiddleInput(unsigned long cycle)
{
    (void)cycle;
    return -1;
}