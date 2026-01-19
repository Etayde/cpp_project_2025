//////////////////////////////////////       INCLUDES & FORWARDS       /////////////////////////////////////////////

#include "Game.h"
#include "NormalGame.h"
#include "LoadedGame.h"
#include "Console.h"
#include "Constants.h"
#include "Layouts.h"
#include "LevelLoader.h"
#include "Obstacle.h"
#include "Riddle.h"
#include "Spring.h"
#include "Spring.h"
#include <string>
#include <numeric>
#include <algorithm>
#include <random>

class Constants;

//////////////////////////////////////////     Game Constructor       /////////////////////////////////////////////

Game* Game::currentInstance = nullptr;

Game::Game()
    : silentMode(false), consoleInitialized(false), initErrorMessage(ErrorCode::NONE), initErrorRoomId(-1), 
      gameOverMessege(GameOverMessege::NONE), cycleCount(0), currentState(GameState::mainMenu), 
      currentRoomId(-1), gameInitialized(false), colorMode(true)
{
  currentInstance = this;
  Renderer::setSilentMode(silentMode);
}

// Free function used by Console.h to check color mode in OOP way
bool isGameColorEnabled()
{
  return Game::isColorEnabled();
}

//////////////////////////////////////////      Game Destructor       /////////////////////////////////////////////

Game::~Game()
{
  for (Screen *s : loadedScreens) delete s;
  loadedScreens.clear();

  if (consoleInitialized)
  {
    showCursor();
    if (!silentMode){
      clrscr();
      cleanup_console();
    }
    std::cout << "Thanks for playing!" << std::endl;
  }
}

//////////////////////////////////////////   createFromArgs (Factory)    /////////////////////////////////////////////

Game* Game::createFromArgs(int argc, char* argv[])
{
  for (int i = 1; i < argc; i++)
  {
    std::string arg(argv[i]);
    if (arg == "-load") return new LoadedGame(argc, argv);
  }

  return new NormalGame(argc, argv);
}

//////////////////////////////////////////           run               /////////////////////////////////////////////


//////////////////////////////////////////   validateLegendPlacement  /////////////////////////////////////////////

ErrorCode Game::validateLegendPlacement(Room &room)
{
  if (room.getBaseLayout() == nullptr)
    return ErrorCode::NONE;

  std::vector<Point> lMarkers;
  for (int y = 0; y < MAX_Y; y++)
  {
    for (int x = 0; x < MAX_X; x++)
    {
      if (room.getBaseLayout()->getCharAt(x, y) == 'L') lMarkers.push_back(Point(x, y));
    }
  }

  if (lMarkers.empty()) return ErrorCode::L_NOT_FOUND;

  if (lMarkers.size() > 1) return ErrorCode::MULTIPLE_L;

  Point lPos = lMarkers[0];
  int topLeftX = lPos.getX() - 1;
  int topLeftY = lPos.getY() - 1;
  int width = 22;
  int height = 5;
  if (topLeftX < 0 || topLeftY < 0 || topLeftX + width > MAX_X ||
      topLeftY + height > MAX_Y) return ErrorCode::L_OUT_OF_BOUNDS;

  for (int y = topLeftY; y < topLeftY + height; y++)
  {
    for (int x = topLeftX; x < topLeftX + width; x++)
    {
      char c = room.getBaseLayout()->getCharAt(x, y);
      if (c != ObjectType::WALL && c != ObjectType::AIR && c != 'L')
       return ErrorCode::LEGEND_OBSCURES_OBJECTS;
    }
  }

  Point p1Spawn = room.getSpawnPoint(1);
  if (p1Spawn.getX() >= topLeftX && p1Spawn.getX() < topLeftX + width &&
      p1Spawn.getY() >= topLeftY && p1Spawn.getY() < topLeftY + height)
      return ErrorCode::LEGEND_OBSCURES_SPAWN;
  
  Point p2Spawn = room.getSpawnPoint(2);
  if (p2Spawn.getX() >= topLeftX && p2Spawn.getX() < topLeftX + width &&
      p2Spawn.getY() >= topLeftY && p2Spawn.getY() < topLeftY + height)
      return ErrorCode::LEGEND_OBSCURES_SPAWN;

  Point p1Next = room.getSpawnPointFromNext(1);
  if (p1Next.getX() >= topLeftX && p1Next.getX() < topLeftX + width &&
      p1Next.getY() >= topLeftY && p1Next.getY() < topLeftY + height)
      return ErrorCode::LEGEND_OBSCURES_SPAWN;
  
  Point p2Next = room.getSpawnPointFromNext(2);
  if (p2Next.getX() >= topLeftX && p2Next.getX() < topLeftX + width &&
      p2Next.getY() >= topLeftY && p2Next.getY() < topLeftY + height)
      return ErrorCode::LEGEND_OBSCURES_SPAWN;

  room.setLegendPoint(lPos.getX(), lPos.getY());

  return ErrorCode::NONE;
}

//////////////////////////////////////////       startNewGame         /////////////////////////////////////////////

void Game::startNewGame()
{
  cycleCount = 0;
  if (rooms.empty()) initializeRooms();
  
  if (rooms.empty()) return;

  rooms[0].setActive(true);
  Point startPos1 = rooms[0].getSpawnPoint(1);
  Point startPos2 = rooms[0].getSpawnPoint(2);
  
  player1 = Player(1, startPos1.getX(), startPos1.getY(), PlayerSprites::PLAYER1);
  player2 = Player(2, startPos2.getX(), startPos2.getY(), PlayerSprites::PLAYER2);

  currentRoomId = 0;
  rooms[0].setActive(true);
  gameInitialized = true;
}

//////////////////////////////////////////         gameLoop           /////////////////////////////////////////////

void Game::gameLoop()
{
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
        return;
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

    if (room)
      room->drawLegend(&player1, &player2);
  }

  while (currentState == GameState::inGame)
  {
    handleInput();
    update();
    Renderer::sleep_ms(100);
  }
}

//////////////////////////////////////////          update            /////////////////////////////////////////////

void Game::update()
{
  updateCycleCount();

  Room *room = getCurrentRoom();
  if (room == nullptr) return;

  room->resetAllObstaclePushStates();

  player1.move(room, &aRiddle.riddle, &aRiddle.player, &player2, this);
  player2.move(room, &aRiddle.riddle, &aRiddle.player, &player1, this);

  if (player1.hasRequestedPause() || player2.hasRequestedPause())
  {
    player1.setRequestPause(false);
    player2.setRequestPause(false);
    currentState = GameState::paused;
    return;
  }

  ExplosionResult explosionResult = room->updateAllObjects(&player1, &player2);
  if (explosionResult.player1Hit) player1.loseLife(room, this);
  if (explosionResult.player2Hit) player2.loseLife(room, this);

  if (checkGameOver(explosionResult))
  {
    currentState = GameState::gameOver;
    return;
  }

  room->updateVisibility(&player1, &player2);
  room->drawDarkness(&player1, &player2);
  room->drawVisibleObjects();
  room->drawLegend(&player1, &player2);
  player1.draw(room);
  player2.draw(room);

  checkRoomTransitions();

  Renderer::flush();
}

//////////////////////////////////////////       getCurrentRoom       /////////////////////////////////////////////

Room *Game::getCurrentRoom()
{
  if (currentRoomId >= 0 && currentRoomId < static_cast<int>(rooms.size()))
    return &rooms[currentRoomId];
  return nullptr;
}

//////////////////////////////////////////     redrawCurrentRoom      /////////////////////////////////////////////

void Game::redrawCurrentRoom()
{
  Renderer::clrscr();
  Room *room = getCurrentRoom();
  if (room) room->draw();
  
  player1.draw(room);
  player2.draw(room);
  
  if (room) room->drawLegend(&player1, &player2);

  if (aRiddle.isActive()) aRiddle.riddle->draw();
}

/////////////////////////////////////////    canPassThroughDoor       /////////////////////////////////////////////

bool Game::canPassThroughDoor(Room *room, int doorId)
{
  if (room == nullptr) return false;

  int targetRoom = room->getDoorTargetRoomId(doorId);

  // If explicit target is set, check requirements
  if (targetRoom != -1)
  {
      return room->isDoorUnlocked(doorId) ||
             room->canOpenDoor(doorId, player1.getKeyCount(),
                               player2.getKeyCount());
  }

  if (doorId == room->getNextRoomId() || doorId == static_cast<int>(rooms.size()))
  {
    return room->isDoorUnlocked(doorId) ||
           room->canOpenDoor(doorId, player1.getKeyCount(),
                             player2.getKeyCount());
  }
  else if (doorId == room->getPrevRoomId()) return true;
  
  return false;
}

/////////////////////////////////////////      checkRoomTransitions   /////////////////////////////////////////////

void Game::checkRoomTransitions()
{
  Room *room = getCurrentRoom();
  if (room == nullptr) return;
  
  auto updateWaitState = [&](Player &p) {
      if (p.isAtDoor())
      {
           if (canPassThroughDoor(room, p.getDoorId()))
           {
               if (!p.isWaitingAtDoor()) p.setWaitingAtDoor(true);
           }
      }
      else p.setWaitingAtDoor(false);
  };

  updateWaitState(player1);
  updateWaitState(player2);

  if (player1.isAtDoor() && player2.isAtDoor() &&
      player1.getDoorId() == player2.getDoorId())
  {
    int doorId = player1.getDoorId();
    if (canPassThroughDoor(room, doorId))
    {
      player1.setWaitingAtDoor(false);
      player2.setWaitingAtDoor(false);

      int targetRoom = room->getDoorTargetRoomId(doorId);

      if (targetRoom != -1 || doorId == room->getNextRoomId() || doorId == static_cast<int>(rooms.size()))
      {
        if (!room->isDoorUnlocked(doorId))
        {
          int keysNeeded = room->getDoorRequiredKeys(doorId);

          int keysConsumed = 0;
          while (keysConsumed < keysNeeded && player1.getKeyCount() > 0)
          {
            player1.useKey();
            keysConsumed++;
          }
          while (keysConsumed < keysNeeded && player2.getKeyCount() > 0)
          {
            player2.useKey();
            keysConsumed++;
          }

          player1.incrementScore(100 * player1.getLives());
          player2.incrementScore(100 * player2.getLives());
        }
        room->unlockDoor(doorId);

        if (currentRoomId == -1) 
        {
          currentState = GameState::victory;
          return;
        }
        
        int destinationId = (targetRoom != -1) ? targetRoom : room->getNextRoomId();
        
        if (destinationId == -1) {
             currentState = GameState::victory;
             return;
        }

        changeRoom(destinationId, true);
      }
      else if (doorId == room->getPrevRoomId()) changeRoom(room->getPrevRoomId(), false);
    }
  }
}

//////////////////////////////////////////       Menu Handlers        /////////////////////////////////////////////

void Game::showMainMenu() { mainMenuScreen.draw(); }

void Game::handleMainMenuInput()
{
  if (check_kbhit())
  {
    char choice = get_single_char();
    switch (choice)
    {
    case '1':
      currentState = GameState::inGame;
      break;
    case '8':
      currentState = GameState::instructions;
      break;
    case '9':
      currentState = GameState::quit;
      break;
    }
  }
}

void Game::showInstructions() { instructionsScreen.draw(); }

void Game::handleInstructionsInput()
{
  if (check_kbhit())
  {
    char choice = get_single_char();
    if (choice == static_cast<char>(Action::ESC))
      currentState = GameState::mainMenu;
  }
}

void Game::showPauseMenu() { pauseScreen.draw(); }

void Game::handlePauseInput()
{
  if (check_kbhit())
  {
    char choice = get_single_char();
    if (choice == char(Action::ESC)) currentState = GameState::inGame;
    else if (choice == 'h' || choice == 'H')
    {
      gameInitialized = false; // Reset game when going to main menu
      currentState = GameState::mainMenu;
    }
  }
}

void Game::showVictory()
{
  victoryScreen.draw();
  Renderer::gotoxy(40, 13);
  Renderer::print(player1.getScore());
  Renderer::print("\n");
  Renderer::gotoxy(40, 14);
  Renderer::print(player2.getScore());
  Renderer::print("\n");
}

void Game::showGameOver()
{

  gameOverScreen.draw();

  switch (gameOverMessege)
  {
  case GameOverMessege::PLAYER1_DIED:
    Renderer::gotoxy(24, 8);
    Renderer::print("Player 1 ($)  has died.\n");
    break;
  case GameOverMessege::PLAYER2_DIED:
    Renderer::gotoxy(24, 8);
    Renderer::print("Player 2 (&)  has died.\n");
    break;
  case GameOverMessege::VALUABLE_DESTROYED:
    Renderer::gotoxy(18, 8);
    Renderer::print("An essential object has been destroyed.\n");
    Renderer::gotoxy(18, 9);
    Renderer::print(" The game cannot continue without it.\n");
    break;
  default:
    Renderer::gotoxy(24, 8);
    Renderer::print("Unknown game over messege.\n");
    break;
  }
}

void Game::showErrorScreen()
{
  initErrorScreen.draw();
  switch (initErrorMessage)
  {
  case ErrorCode::L_NOT_FOUND:
    Renderer::gotoxy(26, 10);
    Renderer::print("Error: No 'L' found in room.");
    Renderer::print(initErrorRoomId);
    Renderer::print("\n");
    break;
  case ErrorCode::MULTIPLE_L:
    Renderer::gotoxy(23, 10);
    Renderer::print("Error: Multiple 'L's found in room.");
    Renderer::print(initErrorRoomId);
    Renderer::print("\n");
    break;
  case ErrorCode::L_OUT_OF_BOUNDS:
    Renderer::gotoxy(24, 10);
    Renderer::print("Error: 'L' out of bounds in room.");
    Renderer::print(initErrorRoomId);
    Renderer::print("\n");
    break;
  case ErrorCode::LEGEND_OBSCURES_OBJECTS:
    Renderer::gotoxy(21, 10);
    Renderer::print("Error: Legend obscured objects in room.");
    Renderer::print(initErrorRoomId);
    Renderer::print("\n");
    break;
  case ErrorCode::LEGEND_OBSCURES_SPAWN:
    Renderer::gotoxy(13, 10);
    Renderer::print("Error: Legend obscured a player's spawn point in room.");
    Renderer::print(initErrorRoomId);
    Renderer::print("\n");
    break;
  case ErrorCode::FILE_NOT_FOUND:
    Renderer::gotoxy(25, 10);
    Renderer::print("Error: Required file not found.\n");
    break;
  case ErrorCode::READ_ERROR:
    Renderer::gotoxy(27, 10);
    Renderer::print("Error: Failed to read file.\n");
    break;
  case ErrorCode::NO_SCREENS_FOUND:
    Renderer::gotoxy(15, 10);
    Renderer::print("Error: No .screen files found in working directory.\n");
    break;
  case ErrorCode::MISSING_RANDOM_SEED:
    Renderer::gotoxy(19, 10);
    Renderer::print("Error: No random seed found in steps file.\n");
    break;
  case ErrorCode::SCREEN_MISMATCH:
    Renderer::gotoxy(5, 10);
    Renderer::print("Error: Screen files mismatch between steps file and current directory.\n");
    break;
  default:
    Renderer::gotoxy(34, 10);
    Renderer::print("Unknown error\n");
    break;
  }
}

//////////////////////////////////////////      initializeRooms       /////////////////////////////////////////////

// Seed handling implemented with AI
void Game::initializeRooms(unsigned int seed)
{

  rooms.clear();
  loadedScreens.clear();

  int riddlesLoaded = LevelLoader::loadRiddleFile();
  (void)riddlesLoaded;

  std::vector<Screen *> screens;
  std::vector<RoomMetadata> metadatas;
  std::vector<std::string> levelFiles = LevelLoader::discoverLevelFiles();

  if (levelFiles.empty())
  {
    initErrorMessage = ErrorCode::NO_SCREENS_FOUND;
    currentState = GameState::error;
    return;
  }

  for (const std::string &filename : levelFiles)
  {
    RoomMetadata metadata;
    Screen *screen = LevelLoader::loadScreenFile(filename, metadata);

    if (screen == nullptr) continue;

    screens.push_back(screen);
    metadatas.push_back(metadata);
  }

  if (screens.empty())
  {
    currentState = GameState::error;
    return;
  }

  rooms.reserve(screens.size());
  loadedScreens.reserve(screens.size());

  std::vector<int> riddleIds(riddlesLoaded);
  std::iota(riddleIds.begin(), riddleIds.end(), 0);
  
  std::mt19937 g;

  if (seed != 0) g.seed(seed);
  
  else
  {
    std::random_device rd;
    g.seed(rd());
  }

  std::shuffle(riddleIds.begin(), riddleIds.end(), g);

  int riddleIndex = 0;
  for (size_t i = 0; i < screens.size(); i++)
  {
    loadedScreens.push_back(screens[i]);
    rooms.push_back(Room(static_cast<int>(i)));

    Room &room = rooms.back();
    room.initFromLayout(screens[i], &riddleIds, &riddleIndex);
    room.setSpawnPoint(metadatas[i].spawnPoint);
    room.setSpawnPointFromNext(metadatas[i].spawnPointFromNext);
    room.setNextRoomId(metadatas[i].nextRoomId);
    room.setPrevRoomId(metadatas[i].prevRoomId);

    for (const auto &doorConfig : metadatas[i].doorConfigs)
    {
      room.setDoorRequirements(std::get<0>(doorConfig), std::get<1>(doorConfig),
                               std::get<2>(doorConfig), std::get<3>(doorConfig));
    }

    for (const auto &dz : metadatas[i].darkZones)
      room.addDarkZone(dz.x1, dz.y1, dz.x2, dz.y2);
    
    ErrorCode validationResult = validateLegendPlacement(room);
    if (validationResult != ErrorCode::NONE)
    {
      initErrorMessage = validationResult;
      initErrorRoomId = i + 1;
      currentState = GameState::error;
      return;
    }
  }

  finalRoomId = static_cast<int>(rooms.size()) - 1;
}

//////////////////////////////////////////        changeRoom          /////////////////////////////////////////////

void Game::changeRoom(int newRoomId, bool goingForward)
{

  if (newRoomId == -1 || newRoomId == static_cast<int>(rooms.size()))
  {
    if (currentRoomId >= 0) rooms[currentRoomId].setActive(false);
    currentState = GameState::victory;
    return;
  }

  if (newRoomId < 0 || newRoomId >= static_cast<int>(rooms.size())) return;

  if (currentRoomId >= 0) rooms[currentRoomId].setActive(false);

  currentRoomId = newRoomId;
  rooms[newRoomId].setActive(true);

  rooms[newRoomId].setActive(true);

  Point nextPos1, nextPos2;
  if (goingForward)
  {
      nextPos1 = rooms[newRoomId].getSpawnPoint(1);
      nextPos2 = rooms[newRoomId].getSpawnPoint(2);
  }
  else
  {
      nextPos1 = rooms[newRoomId].getSpawnPointFromNext(1);
      nextPos2 = rooms[newRoomId].getSpawnPointFromNext(2);
  }

  player1.setPosition(nextPos1.getX(), nextPos1.getY());
  player2.setPosition(nextPos2.getX(), nextPos2.getY());

  player1.stopMovement();
  player2.stopMovement();
  player1.setAtDoor(false);
  player2.setAtDoor(false);
  player1.setWaitingAtDoor(false);
  player2.setWaitingAtDoor(false);

  Renderer::clrscr();
  if (getCurrentRoom())
  {
    getCurrentRoom()->draw();
    getCurrentRoom()->drawLegend(&player1, &player2);
  }
}

//////////////////////////////////////////       checkGameOver        /////////////////////////////////////////////

bool Game::checkGameOver(const ExplosionResult &result)
{
  if (player1.isDead())
  {
    setGameOverMessege(GameOverMessege::PLAYER1_DIED);
    return true;
  }

  if (player2.isDead())
  {
    setGameOverMessege(GameOverMessege::PLAYER2_DIED);
    return true;
  }

  Room *room = getCurrentRoom();
  int neededSwitches = room->getDoorReqSwitches(room->getNextRoomId());
  int totalSwitches = room->getTotalSwitches();

  if (result.keyDestroyed)
  {
    setGameOverMessege(GameOverMessege::VALUABLE_DESTROYED);
    return true;
  }

  if (neededSwitches > 0 && totalSwitches < neededSwitches)
  {
    setGameOverMessege(GameOverMessege::VALUABLE_DESTROYED);
    return true;
  }

  return false;
}
