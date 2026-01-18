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

Game::Game()
    : silentMode(false), consoleInitialized(false), initErrorMessage(ErrorCode::NONE), initErrorRoomId(-1), gameOverMessege(GameOverMessege::NONE),
      cycleCount(0), currentState(GameState::mainMenu), currentRoomId(-1),
      gameInitialized(false)
{
  init_console();
  hideCursor();
  clrscr();
  consoleInitialized = true;

  Renderer::setSilentMode(silentMode);
}

//////////////////////////////////////////      Game Destructor       /////////////////////////////////////////////

Game::~Game()
{
  for (Screen *s : loadedScreens)
  {
    delete s;
  }
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
    if (arg == "-load")
    {
      return new LoadedGame(argc, argv);
    }
  }

  return new NormalGame(argc, argv);
}

//////////////////////////////////////////           run               /////////////////////////////////////////////


//////////////////////////////////////////   validateLegendPlacement  /////////////////////////////////////////////

ErrorCode Game::validateLegendPlacement(Room &room)
{
  if (room.baseLayout == nullptr)
    return ErrorCode::NONE;

  std::vector<Point> lMarkers;
  for (int y = 0; y < MAX_Y; y++)
  {
    for (int x = 0; x < MAX_X; x++)
    {
      if (room.baseLayout->getCharAt(x, y) == 'L')
      {
        lMarkers.push_back(Point(x, y));
      }
    }
  }

  if (lMarkers.empty())
  {
    return ErrorCode::L_NOT_FOUND;
  }

  if (lMarkers.size() > 1)
  {
    return ErrorCode::MULTIPLE_L;
  }

  Point lPos = lMarkers[0];
  int topLeftX = lPos.x - 1;
  int topLeftY = lPos.y - 1;
  int width = 22;
  int height = 5;
  if (topLeftX < 0 || topLeftY < 0 || topLeftX + width > MAX_X ||
      topLeftY + height > MAX_Y)
  {
    return ErrorCode::L_OUT_OF_BOUNDS;
  }

  for (int y = topLeftY; y < topLeftY + height; y++)
  {
    for (int x = topLeftX; x < topLeftX + width; x++)
    {
      char c = room.baseLayout->getCharAt(x, y);
      if (c != ObjectType::WALL && c != ObjectType::AIR && c != 'L')
      {
        return ErrorCode::LEGEND_OBSCURES_OBJECTS;
      }
    }
  }

  // Check P1 spawn (explicit)
  Point p1Spawn = room.getSpawnPoint(1);
  if (p1Spawn.x >= topLeftX && p1Spawn.x < topLeftX + width &&
      p1Spawn.y >= topLeftY && p1Spawn.y < topLeftY + height)
  {
      return ErrorCode::LEGEND_OBSCURES_SPAWN;
  }
  // Check P2 spawn (calculated)
  Point p2Spawn = room.getSpawnPoint(2);
  if (p2Spawn.x >= topLeftX && p2Spawn.x < topLeftX + width &&
      p2Spawn.y >= topLeftY && p2Spawn.y < topLeftY + height)
  {
      return ErrorCode::LEGEND_OBSCURES_SPAWN;
  }

  // Check P1 FromNext (explicit)
  Point p1Next = room.getSpawnPointFromNext(1);
  if (p1Next.x >= topLeftX && p1Next.x < topLeftX + width &&
      p1Next.y >= topLeftY && p1Next.y < topLeftY + height)
  {
      return ErrorCode::LEGEND_OBSCURES_SPAWN;
  }
  // Check P2 FromNext (calculated)
  Point p2Next = room.getSpawnPointFromNext(2);
  if (p2Next.x >= topLeftX && p2Next.x < topLeftX + width &&
      p2Next.y >= topLeftY && p2Next.y < topLeftY + height)
  {
      return ErrorCode::LEGEND_OBSCURES_SPAWN;
  }

  room.setLegendPoint(lPos.x, lPos.y);

  return ErrorCode::NONE;
}

//////////////////////////////////////////       startNewGame         /////////////////////////////////////////////

void Game::startNewGame()
{
  initializeRooms();
  
  if (rooms.empty())
  {
      return;
  }

  rooms[0].active = true;
  Point startPos1 = rooms[0].getSpawnPoint(1);
  Point startPos2 = rooms[0].getSpawnPoint(2);
  
  player1 = Player(1, startPos1.x, startPos1.y, PlayerSprites::PLAYER1);
  player2 = Player(2, startPos2.x, startPos2.y, PlayerSprites::PLAYER2);

  currentRoomId = 0;
  rooms[0].active = true;
  gameInitialized = true;
}

//////////////////////////////////////////         gameLoop           /////////////////////////////////////////////

void Game::gameLoop()
{
  Room *room = getCurrentRoom();

  if (aRiddle.isActive())
  {
    Renderer::clrscr();
    if (room)
    {
      room->draw();
    }
    player1.draw(room);
    player2.draw(room);
    if (room)
      room->drawLegend(&player1, &player2);

    while (aRiddle.isActive() && currentState == GameState::inGame)
    {
      RiddleResult result = aRiddle.riddle->enterRiddle(room, aRiddle.player, this);

      if (result == RiddleResult::NO_RIDDLE)
      {
        room->removeObjectAt(aRiddle.riddle->getX(), aRiddle.riddle->getY());
        aRiddle.reset();
        Renderer::clrscr();
        if (room)
        {
          room->draw();
        }
        player1.draw(room);
        player2.draw(room);
        if (room)
          room->drawLegend(&player1, &player2);
        break;
      };
      if (result == RiddleResult::SOLVED)
      {
        room->removeObjectAt(aRiddle.riddle->getX(), aRiddle.riddle->getY());
        aRiddle.reset();
        Renderer::clrscr();
        if (room)
        {
          room->draw();
        }
        player1.draw(room);
        player2.draw(room);
        if (room)
          room->drawLegend(&player1, &player2);
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
        if (room)
        {
          room->draw();
        }
        player1.draw(room);
        player2.draw(room);
        if (room)
          room->drawLegend(&player1, &player2);
        break;
      }
    }
  }
  else
  {
    if (room)
    {
      room->draw();
    }
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
  if (room == nullptr)
    return;

  for (Obstacle *obstacle : room->obstacles)
  {
    obstacle->resetPushState();
  }

  player1.move(room, &aRiddle.riddle, &aRiddle.player, &player2, this);
  player2.move(room, &aRiddle.riddle, &aRiddle.player, &player1, this);

  if (player1.requestPause || player2.requestPause)
  {
    player1.requestPause = false;
    player2.requestPause = false;
    currentState = GameState::paused;
    return;
  }

  ExplosionResult explosionResult = room->updateAllObjects(&player1, &player2);
  if (explosionResult.player1Hit)
    player1.loseLife(room, this);
  if (explosionResult.player2Hit)
    player2.loseLife(room, this);

  if (checkGameOver(explosionResult))
  {
    currentState = GameState::gameOver;
    return;
  }

  room->updateVisibility(&player1, &player2);
  room->drawDarkness();
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
  if (room)
  {
    room->draw();
  }
  player1.draw(room);
  player2.draw(room);
  if (room)
    room->drawLegend(&player1, &player2);

  if (aRiddle.isActive())
  {
    aRiddle.riddle->draw();
  }
}

/////////////////////////////////////////    canPassThroughDoor       /////////////////////////////////////////////

bool Game::canPassThroughDoor(Room *room, int doorId)
{
  if (room == nullptr)
    return false;

  int targetRoom = -1;
  if (doorId >= 0 && doorId < static_cast<int>(room->doorReqs.size()))
  {
      targetRoom = room->doorReqs[doorId].targetRoomId;
  }

  // If explicit target is set, check requirements
  if (targetRoom != -1)
  {
      return room->isDoorUnlocked(doorId) ||
             room->canOpenDoor(doorId, player1.getKeyCount(),
                               player2.getKeyCount());
  }

  if (doorId == room->nextRoomId || doorId == static_cast<int>(rooms.size()))
  {
    return room->isDoorUnlocked(doorId) ||
           room->canOpenDoor(doorId, player1.getKeyCount(),
                             player2.getKeyCount());
  }
  else if (doorId == room->prevRoomId)
  {
    return true;
  }
  return false;
}

/////////////////////////////////////////      checkRoomTransitions   /////////////////////////////////////////////

void Game::checkRoomTransitions()
{
  Room *room = getCurrentRoom();
  if (room == nullptr)
    return;

  // 1. Update Wait State (Generic for both Forward and Backward)
  // If player is at a passable door, set waitingAtDoor = true.
  // Player::draw() handles visibility (Invisible for Forward, Visible for Backward).
  
  auto updateWaitState = [&](Player &p) {
      if (p.isAtDoor())
      {
           if (canPassThroughDoor(room, p.getDoorId()))
           {
               if (!p.waitingAtDoor) p.waitingAtDoor = true;
           }
      }
      else
      {
          p.waitingAtDoor = false;
      }
  };

  updateWaitState(player1);
  updateWaitState(player2);

  // 2. Check for Shared Transition
  // We check if BOTH are at door (physically).
  // Note: Even if invisible, they are 'atDoor' physically.
  if (player1.isAtDoor() && player2.isAtDoor() &&
      player1.getDoorId() == player2.getDoorId())
  {
    int doorId = player1.getDoorId();
    if (canPassThroughDoor(room, doorId))
    {
      player1.waitingAtDoor = false;
      player2.waitingAtDoor = false;

      // Handle Forward Transition
      int targetRoom = -1;
      if (doorId >= 0 && doorId < static_cast<int>(room->doorReqs.size()))
      {
          targetRoom = room->doorReqs[doorId].targetRoomId;
      }

      if (targetRoom != -1 || doorId == room->nextRoomId || doorId == static_cast<int>(rooms.size()))
      {
        if (!room->isDoorUnlocked(doorId))
        {
          int keysNeeded = (doorId >= 0 && doorId < static_cast<int>(room->doorReqs.size()))
                               ? room->doorReqs[doorId].requiredKeys
                               : 0;

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
        
        int destinationId = (targetRoom != -1) ? targetRoom : room->nextRoomId;
        
        if (destinationId == -1) {
             currentState = GameState::victory;
             return;
        }

        changeRoom(destinationId, true);
      }
      // Handle Backward Transition
      else if (doorId == room->prevRoomId)
      {
        changeRoom(room->prevRoomId, false);
      }
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
    if (choice == char(Action::ESC))
      currentState = GameState::inGame;
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
  Renderer::gotoxy(22, 10);
  switch (initErrorMessage)
  {
  case ErrorCode::L_NOT_FOUND:
    Renderer::print("Error: No 'L' found in room ");
    Renderer::print(initErrorRoomId);
    Renderer::print("\n");
    break;
  case ErrorCode::MULTIPLE_L:
    Renderer::print("Error: Multiple 'L's found in room ");
    Renderer::print(initErrorRoomId);
    Renderer::print("\n");
    break;
  case ErrorCode::L_OUT_OF_BOUNDS:
    Renderer::print("Error: 'L' out of bounds in room ");
    Renderer::print(initErrorRoomId);
    Renderer::print("\n");
    break;
  case ErrorCode::LEGEND_OBSCURES_OBJECTS:
    Renderer::print("Error: Legend obscured objects in room ");
    Renderer::print(initErrorRoomId);
    Renderer::print("\n");
    break;
  case ErrorCode::LEGEND_OBSCURES_SPAWN:
    Renderer::print("Error: Legend obscured a player's spawn point in room ");
    Renderer::print(initErrorRoomId);
    Renderer::print("\n");
    break;
  case ErrorCode::FILE_NOT_FOUND:
    Renderer::print("Error: Required file not found\n");
    break;
  case ErrorCode::READ_ERROR:
    Renderer::print("Error: Failed to read file\n");
    break;
  case ErrorCode::NO_SCREENS_FOUND:
    Renderer::print("Error: No .screen files found in working directory.\n");
    break;
  case ErrorCode::MISSING_RANDOM_SEED:
    Renderer::print("Error: No random seed found in steps file.\n");
    break;
  default:
    Renderer::print("Unknown error\n");
    break;
  }
}

//////////////////////////////////////////      initializeRooms       /////////////////////////////////////////////

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

    if (screen == nullptr)
    {
      continue;
    }

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
  if (seed != 0) {
      g.seed(seed);
  } else {
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
    room.spawnPoint = metadatas[i].spawnPoint;
    room.spawnPointFromNext = metadatas[i].spawnPointFromNext;
    room.nextRoomId = metadatas[i].nextRoomId;
    room.prevRoomId = metadatas[i].prevRoomId;

    for (const auto &doorConfig : metadatas[i].doorConfigs)
    {
      room.setDoorRequirements(std::get<0>(doorConfig), std::get<1>(doorConfig),
                               std::get<2>(doorConfig), std::get<3>(doorConfig));
    }

    for (const auto &dz : metadatas[i].darkZones)
    {
      room.addDarkZone(dz.x1, dz.y1, dz.x2, dz.y2);
    }
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
    if (currentRoomId >= 0)
    {
      rooms[currentRoomId].active = false;
    }
    currentState = GameState::victory;
    return;
  }

  if (newRoomId < 0 || newRoomId >= static_cast<int>(rooms.size()))
  {
    return;
  }

  if (currentRoomId >= 0)
  {
    rooms[currentRoomId].active = false;
  }

  currentRoomId = newRoomId;
  rooms[newRoomId].active = true;

  rooms[newRoomId].active = true;

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

  player1.setPosition(nextPos1.x, nextPos1.y);
  player2.setPosition(nextPos2.x, nextPos2.y);

  player1.pos.diff_x = 0;
  player1.pos.diff_y = 0;
  player2.pos.diff_x = 0;
  player2.pos.diff_y = 0;
  player1.atDoor = false;
  player2.atDoor = false;
  player1.waitingAtDoor = false;
  player2.waitingAtDoor = false;

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
  int neededSwitches = room->getDoorReqSwitches(room->nextRoomId);
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
