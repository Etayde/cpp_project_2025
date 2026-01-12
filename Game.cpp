//////////////////////////////////////       INCLUDES & FORWARDS       /////////////////////////////////////////////

#include "Game.h"
#include "Console.h"
#include "Constants.h"
#include "Layouts.h"
#include "LevelLoader.h"
#include "Obstacle.h"
#include "Riddle.h"
#include "Spring.h"

class Constants;

//////////////////////////////////////////     Game Constructor       /////////////////////////////////////////////

Game::Game()
    : silentMode(false), initErrorMessage(ErrorCode::NONE), initErrorRoomId(-1), gameOverMessege(GameOverMessege::NONE),
      cycleCount(0), currentState(GameState::mainMenu), currentRoomId(-1),
      gameInitialized(false)
{
  // Set renderer mode based on silentMode flag
  Renderer::setSilentMode(silentMode);
}

//////////////////////////////////////////      Game Destructor       /////////////////////////////////////////////

Game::~Game()
{
  // Clean up loaded screens
  for (Screen *s : loadedScreens)
  {
    delete s;
  }
  loadedScreens.clear();
}

//////////////////////////////////////////           run               /////////////////////////////////////////////

// Main game entry point - runs until quit
void Game::run()
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
      if (!gameInitialized)
      {
        startNewGame();
        gameInitialized = true;
      }
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

//////////////////////////////////////////   validateLegendPlacement  /////////////////////////////////////////////

ErrorCode Game::validateLegendPlacement(Room &room)
{
  if (room.baseLayout == nullptr)
    return ErrorCode::NONE; // Should not happen for file-loaded levels

  // 1. Find 'L' markers
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

  // Error 1: No 'L' indicator
  if (lMarkers.empty())
  {
    return ErrorCode::L_NOT_FOUND;
  }

  // Error 2: More than one 'L' indicator
  if (lMarkers.size() > 1)
  {
    return ErrorCode::MULTIPLE_L;
  }

  Point lPos = lMarkers[0];
  // Legend is 22x5, 'L' is at (topLeftX+1, topLeftY+1)
  // So TopLeft is (lPos.x - 1, lPos.y - 1)
  int topLeftX = lPos.x - 1;
  int topLeftY = lPos.y - 1;
  int width = 22;
  int height = 5;

  // Error 3: Legend doesn't have enough room (out of bounds)
  if (topLeftX < 0 || topLeftY < 0 || topLeftX + width > MAX_X ||
      topLeftY + height > MAX_Y)
  {
    return ErrorCode::L_OUT_OF_BOUNDS;
  }

  // Error 4: Accessible by game objects or invalid overlap
  // First, checking for invalid overlap (must be 'W' or ' ')
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

  // 4b. Check for overlap with Spawn Points (Player locations)
  // Check primary spawn point
  if (room.spawnPoint.x >= topLeftX && room.spawnPoint.x < topLeftX + width &&
      room.spawnPoint.y >= topLeftY && room.spawnPoint.y < topLeftY + height)
  {
    return ErrorCode::LEGEND_OBSCURES_SPAWN;
  }
  // Check secondary spawn point (from next room)
  if (room.spawnPointFromNext.x >= topLeftX &&
      room.spawnPointFromNext.x < topLeftX + width &&
      room.spawnPointFromNext.y >= topLeftY &&
      room.spawnPointFromNext.y < topLeftY + height)
  {
    return ErrorCode::LEGEND_OBSCURES_SPAWN;
  }

  room.setLegendPoint(lPos.x, lPos.y);

  return ErrorCode::NONE; // Valid
}

//////////////////////////////////////////       startNewGame         /////////////////////////////////////////////

void Game::startNewGame()
{
  initializeRooms();

  player1 = Player(1, 5, 7, PlayerSprites::PLAYER1);
  player2 = Player(2, 5, 9, PlayerSprites::PLAYER2);

  currentRoomId = 0;
  rooms[0].active = true;
  gameInitialized = true;
}

//////////////////////////////////////////         gameLoop           /////////////////////////////////////////////

void Game::gameLoop()
{
  Room *room = getCurrentRoom();

  // Check if we're resuming a riddle interaction
  if (aRiddle.isActive())
  {
    // Clear screen and redraw game state before showing riddle
    Renderer::clrscr();
    if (room)
    {
      room->draw();
    }
    player1.draw(room);
    player2.draw(room);
    if (room)
      room->drawLegend(&player1, &player2);

    // Riddle is active - keep showing it until answered or ESC multiple times
    while (aRiddle.isActive() && currentState == GameState::inGame)
    {
      RiddleResult result = aRiddle.riddle->enterRiddle(room, aRiddle.player);

      if (result == RiddleResult::NO_RIDDLE)
      {
        room->removeObjectAt(aRiddle.riddle->getX(), aRiddle.riddle->getY());
        aRiddle.reset(); // Clear active riddle
        // Riddle finished - redraw screen and fall through to normal game
        Renderer::clrscr();
        if (room)
        {
          room->draw();
        }
        player1.draw(room);
        player2.draw(room);
        if (room)
          room->drawLegend(&player1, &player2);
        break; // Exit riddle loop, continue to normal game
      };
      if (result == RiddleResult::SOLVED)
      {
        room->removeObjectAt(aRiddle.riddle->getX(), aRiddle.riddle->getY());
        aRiddle.reset(); // Clear active riddle
        // Riddle finished - redraw screen and fall through to normal game
        Renderer::clrscr();
        if (room)
        {
          room->draw();
        }
        player1.draw(room);
        player2.draw(room);
        if (room)
          room->drawLegend(&player1, &player2);
        break; // Exit riddle loop, continue to normal game
      }
      else if (result == RiddleResult::ESCAPED)
      {
        currentState = GameState::paused;
        return; // Pause - will come back here with aRiddle still set
      }
      else
      {
        // Failed - player answered wrong, reset aRiddle
        aRiddle.reset();
        // Riddle finished - redraw screen and fall through to normal game
        Renderer::clrscr();
        if (room)
        {
          room->draw();
        }
        player1.draw(room);
        player2.draw(room);
        if (room)
          room->drawLegend(&player1, &player2);
        break; // Exit riddle loop, continue to normal game
      }
    }
  }
  else
  {
    // Normal game start - draw room and start game updates
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
    Renderer::sleep_ms(100); // ~10 FPS
  }
}

//////////////////////////////////////////          update            /////////////////////////////////////////////

void Game::update()
{
  updateCycleCount();

  Room *room = getCurrentRoom();
  if (room == nullptr)
    return;

  // Reset obstacle push state for all obstacles each frame
  for (Obstacle *obstacle : room->obstacles)
  {
    obstacle->resetPushState();
  }

  // Handle player movement
  player1.move(room, &aRiddle.riddle, &aRiddle.player, &player2);
  player2.move(room, &aRiddle.riddle, &aRiddle.player, &player1);

  // Check if either player requested pause (from riddle ESC)
  if (player1.requestPause || player2.requestPause)
  {
    player1.requestPause = false;
    player2.requestPause = false;
    currentState = GameState::paused;
    return;
  }

  // Update all objects - also returns explosion results if bomb explodes
  ExplosionResult explosionResult = room->updateAllObjects(&player1, &player2);
  if (explosionResult.player1Hit)
    player1.loseLife(room);
  if (explosionResult.player2Hit)
    player2.loseLife(room);

  if (checkGameOver(explosionResult))
  {
    currentState = GameState::gameOver;
    return;
  }

  // Update visibility
  room->updateVisibility(&player1, &player2);
  room->drawDarkness();
  room->drawVisibleObjects();
  room->drawLegend(&player1, &player2);
  player1.draw(room);
  player2.draw(room);

  // Check room transitions
  checkRoomTransitions();

  Renderer::flush();
}

//////////////////////////////////////////       getCurrentRoom       /////////////////////////////////////////////

Room *Game::getCurrentRoom()
{
  if (currentRoomId >= 0 && currentRoomId < rooms.size())
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
    // Redraw the active riddle on top
    aRiddle.riddle->draw();
  }
}

/////////////////////////////////////////    canPassThroughDoor       /////////////////////////////////////////////

// Check if a door can be passed through (unlocked or requirements met)
bool Game::canPassThroughDoor(Room *room, int doorId)
{
  if (room == nullptr)
    return false;

  if (doorId == room->nextRoomId || doorId == rooms.size())
  {
    return room->isDoorUnlocked(doorId) ||
           room->canOpenDoor(doorId, player1.getKeyCount(),
                             player2.getKeyCount());
  }
  else if (doorId == room->prevRoomId)
  {
    return true; // Backward doors always passable
  }
  return false;
}

/////////////////////////////////////////      checkRoomTransitions   /////////////////////////////////////////////

// Checks if players have reached a door and can pass through, and handles room
// transition if so
void Game::checkRoomTransitions()
{

  Room *room = getCurrentRoom();
  if (room == nullptr)
    return;

  // BOTH players at the same door - trigger transition
  if (player1.isAtDoor() && player2.isAtDoor() &&
      player1.getDoorId() == player2.getDoorId())
  {
    int doorId = player1.getDoorId();
    if (canPassThroughDoor(room, doorId))
    {
      // Reset waiting state
      player1.waitingAtDoor = false;
      player2.waitingAtDoor = false;

      // Forward door check
      if (doorId == room->nextRoomId || doorId == rooms.size())
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

          // Award points for unlocking the door (first time only)

          player1.incrementScore(100 * player1.getLives());
          player2.incrementScore(100 * player2.getLives());
        }
        room->unlockDoor(doorId);

        // Check if completing this room triggers victory
        if (currentRoomId == -1)
        {
          currentState = GameState::victory;
          return;
        }
        changeRoom(room->nextRoomId, true);
      }
      // Backward door check
      else if (doorId == room->prevRoomId)
      {
        changeRoom(room->prevRoomId, false);
      }
    }
  }
  // ONE player at door - make them wait (cosmetic)
  else if (player1.isAtDoor() && !player2.isAtDoor())
  {
    if (canPassThroughDoor(room, player1.getDoorId()))
    {
      if (!player1.waitingAtDoor)
      {
        player1.waitingAtDoor = true;
        player1.draw(room); // Redraw to hide
      }
    }
  }
  else if (player2.isAtDoor() && !player1.isAtDoor())
  {
    if (canPassThroughDoor(room, player2.getDoorId()))
    {
      if (!player2.waitingAtDoor)
      {
        player2.waitingAtDoor = true;
        player2.draw(room); // Redraw to hide
      }
    }
  }
  // Neither or not at same door - reset waiting state
  else
  {
    if (player1.waitingAtDoor)
    {
      player1.waitingAtDoor = false;
      player1.draw(room); // Redraw to show
    }
    if (player2.waitingAtDoor)
    {
      player2.waitingAtDoor = false;
      player2.draw(room); // Redraw to show
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
      gameInitialized = false; // Reset when going to main menu
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
  default:
    Renderer::print("Unknown error\n");
    break;
  }
}

//////////////////////////////////////////      initializeRooms       /////////////////////////////////////////////

// Initialize all rooms with layouts and requirements
void Game::initializeRooms()
{

  rooms.clear();
  loadedScreens.clear();

  // Try to load riddles from file first
  int riddlesLoaded = LevelLoader::loadRiddleFile();
  (void)riddlesLoaded; // Suppress unused variable warning

  // PHASE 1: Load all screens from files
  std::vector<Screen *> screens;
  std::vector<RoomMetadata> metadatas;
  int fileNumber = 1;

  while (true)
  {
    RoomMetadata metadata;
    Screen *screen = LevelLoader::loadScreenFile(fileNumber, metadata);

    if (screen == nullptr)
    {
      break; // No more files found
    }

    screens.push_back(screen);
    metadatas.push_back(metadata);
    fileNumber++;
  }

  // If no files were loaded, show error
  if (screens.empty())
  {
    currentState = GameState::error;
    return;
  }

  // PHASE 2: Reserve exact capacity (prevents reallocation)
  rooms.reserve(screens.size());
  loadedScreens.reserve(screens.size());

  // PHASE 3: Create all rooms (no reallocation will occur)
  int riddleCounter = 0;
  for (size_t i = 0; i < screens.size(); i++)
  {
    loadedScreens.push_back(screens[i]);
    rooms.push_back(Room(static_cast<int>(i)));

    Room &room = rooms.back();
    room.initFromLayout(screens[i], &riddleCounter);
    room.spawnPoint = metadatas[i].spawnPoint;
    room.spawnPointFromNext = metadatas[i].spawnPointFromNext;
    room.nextRoomId = metadatas[i].nextRoomId;
    room.prevRoomId = metadatas[i].prevRoomId;

    // Apply door requirements from metadata
    for (const auto &doorConfig : metadatas[i].doorConfigs)
    {
      room.setDoorRequirements(std::get<0>(doorConfig), std::get<1>(doorConfig),
                               std::get<2>(doorConfig));
    }

    // Apply dark zones from metadata
    for (const auto &dz : metadatas[i].darkZones)
    {
      room.addDarkZone(dz.x1, dz.y1, dz.x2, dz.y2);
    }

    // Validate Legend Placement
    ErrorCode validationResult = validateLegendPlacement(room);
    if (validationResult != ErrorCode::NONE)
    {
      initErrorMessage = validationResult;
      initErrorRoomId = i + 1;
      currentState = GameState::error;
      return;
    }
  }

  // Set which room triggers victory when completed
  finalRoomId = static_cast<int>(rooms.size()) - 1;
}

//////////////////////////////////////////        changeRoom          /////////////////////////////////////////////

void Game::changeRoom(int newRoomId, bool goingForward)
{

  // Check for victory FIRST (before error validation)
  // newRoomId == -1 or newRoomId == rooms.size() means the player has won
  if (newRoomId == -1 || newRoomId == static_cast<int>(rooms.size()))
  {
    if (currentRoomId >= 0)
    {
      rooms[currentRoomId].active = false;
    }
    currentState = GameState::victory;
    return;
  }

  // Now validate that it's a real room ID
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

  Point spawn = goingForward ? rooms[newRoomId].spawnPoint
                             : rooms[newRoomId].spawnPointFromNext;

  player1.setPosition(spawn.x, spawn.y);
  player2.setPosition(spawn.x, spawn.y + 1);

  player1.pos.diff_x = 0;
  player1.pos.diff_y = 0;
  player2.pos.diff_x = 0;
  player2.pos.diff_y = 0;
  player1.atDoor = false;
  player2.atDoor = false;

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
