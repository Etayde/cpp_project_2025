//////////////////////////////////////       INCLUDES & FORWARDS
/////////////////////////////////////////////

#include "Game.h"
#include "Console.h"
#include "Constants.h"
#include "DebugLog.h"
#include "Layouts.h"
#include "LevelLoader.h"
#include "Obstacle.h"
#include "Riddle.h"
#include "Spring.h"

#include <queue>
#include <set>

class Constants;

//////////////////////////////////////////     Game Constructor
/////////////////////////////////////////////

Game::Game()
    : currentState(GameState::mainMenu), currentRoomId(-1),
      gameInitialized(false) {}

Game::~Game() {
  // Clean up loaded screens
  for (Screen *s : loadedScreens) {
    delete s;
  }
  loadedScreens.clear();
}

//////////////////////////////////////////           run
/////////////////////////////////////////////

// Main game entry point - runs until quit
void Game::run() {
  bool running = true;

  while (running) {
    switch (currentState) {
    case GameState::mainMenu:
      showMainMenu();
      while (currentState == GameState::mainMenu) {
        handleMainMenuInput();
        sleep_ms(50);
      }
      break;

    case GameState::instructions:
      showInstructions();
      while (currentState == GameState::instructions) {
        handleInstructionsInput();
        sleep_ms(50);
      }
      break;

    case GameState::inGame:
      if (!gameInitialized) {
        startNewGame();
        gameInitialized = true;
      }
      gameLoop();
      break;

    case GameState::paused:
      showPauseMenu();
      while (currentState == GameState::paused) {
        handlePauseInput();
        sleep_ms(50);
      }
      // Don't redraw - let the calling context handle screen restoration
      // This allows riddles to persist across pause/resume
      break;

    case GameState::victory:
      showVictory();
      while (check_kbhit())
        get_single_char();
      while (!check_kbhit())
        sleep_ms(50);
      get_single_char();
      gameInitialized = false; // Reset for next game
      currentState = GameState::mainMenu;
      break;

    case GameState::gameOver:
      showGameOver();
      while (check_kbhit())
        get_single_char();
      while (!check_kbhit())
        sleep_ms(50);
      get_single_char();
      gameInitialized = false; // Reset for next game
      currentState = GameState::mainMenu;
      break;

    case GameState::error:
      showErrorScreen();
      while (check_kbhit())
        get_single_char();
      while (!check_kbhit())
        sleep_ms(50);
      get_single_char();
      gameInitialized = false; // Reset for next game
      currentState = GameState::mainMenu;
      break;

    case GameState::quit:
      running = false;
      break;
    }
  }
}

//////////////////////////////////////////   validateLegendPlacement
/////////////////////////////////////////////

int Game::validateLegendPlacement(Room &room) {
  if (room.baseLayout == nullptr)
    return 0; // Should not happen for file-loaded levels

  // 1. Find 'L' markers
  std::vector<Point> lMarkers;
  for (int y = 0; y < MAX_Y; y++) {
    for (int x = 0; x < MAX_X; x++) {
      if (room.baseLayout->getCharAt(x, y) == 'L') {
        lMarkers.push_back(Point(x, y));
      }
    }
  }

  // Error 1: No 'L' indicator
  if (lMarkers.empty()) {
    return 1;
  }

  // Error 2: More than one 'L' indicator
  if (lMarkers.size() > 1) {
    return 2;
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
      topLeftY + height > MAX_Y) {
    return 3;
  }

  // Error 4: Accessible by game objects or invalid overlap
  // First, checking for invalid overlap (must be 'W' or ' ')
  for (int y = topLeftY; y < topLeftY + height; y++) {
    for (int x = topLeftX; x < topLeftX + width; x++) {
      char c = room.baseLayout->getCharAt(x, y);
      if (c != 'W' && c != ' ' && c != 'L') {
        return 4;
      }
    }
  }

  // Second, checking reachability (BFS from spawn)
  bool visited[MAX_Y][MAX_X] = {false};
  std::queue<Point> q;

  // Start BFS from spawn point
  Point start = room.spawnPoint;
  if (start.x >= 0 && start.x < MAX_X && start.y >= 0 && start.y < MAX_Y) {
    q.push(start);
    visited[start.y][start.x] = true;
  }

  while (!q.empty()) {
    Point curr = q.front();
    q.pop();

    // Check if current reachable tile is inside Legend Area
    if (curr.x >= topLeftX && curr.x < topLeftX + width && curr.y >= topLeftY &&
        curr.y < topLeftY + height) {
      return 4; // Legend overlaps reachable area
    }

    // Neighbors
    int dx[] = {0, 0, 1, -1};
    int dy[] = {1, -1, 0, 0};

    for (int i = 0; i < 4; i++) {
      int nx = curr.x + dx[i];
      int ny = curr.y + dy[i];

      if (nx >= 0 && nx < MAX_X && ny >= 0 && ny < MAX_Y) {
        if (!visited[ny][nx]) {
          char c = room.baseLayout->getCharAt(nx, ny);
          // Walkable if NOT a wall
          if (c != 'W' && c != 'w') {
            visited[ny][nx] = true;
            q.push(Point(nx, ny));
          }
        }
      }
    }
  }

  return 0; // Valid
}

//////////////////////////////////////////       startNewGame
/////////////////////////////////////////////

void Game::startNewGame() {
  initializeRooms();

  player1 = Player(1, 5, 7, PlayerSprites::PLAYER1);
  player2 = Player(2, 5, 9, PlayerSprites::PLAYER2);

  currentRoomId = 0;
  rooms[0].active = true;
  gameInitialized = true;
}

//////////////////////////////////////////         gameLoop
/////////////////////////////////////////////

void Game::gameLoop() {
  Room *room = getCurrentRoom();

  // Check if we're resuming a riddle interaction
  if (aRiddle.isActive()) {
    // Clear screen and redraw game state before showing riddle
    clrscr();
    if (room) {
      room->draw();
    }
    player1.draw(room);
    player2.draw(room);
    player1.updateInventoryDisplay();
    player2.updateInventoryDisplay();

    // Riddle is active - keep showing it until answered or ESC multiple times
    while (aRiddle.isActive() && currentState == GameState::inGame) {
      RiddleResult result = aRiddle.riddle->enterRiddle(room, aRiddle.player);

      if (result == RiddleResult::SOLVED) {
        room->removeObjectAt(aRiddle.riddle->getX(), aRiddle.riddle->getY());
        aRiddle.reset(); // Clear active riddle
        // Riddle finished - redraw screen and fall through to normal game
        clrscr();
        if (room) {
          room->draw();
        }
        player1.draw(room);
        player2.draw(room);
        player1.updateInventoryDisplay();
        player2.updateInventoryDisplay();
        break; // Exit riddle loop, continue to normal game
      } else if (result == RiddleResult::ESCAPED) {
        currentState = GameState::paused;
        return; // Pause - will come back here with aRiddle still set
      } else {
        // Failed - player answered wrong, reset aRiddle
        aRiddle.reset();
        // Riddle finished - redraw screen and fall through to normal game
        clrscr();
        if (room) {
          room->draw();
        }
        player1.draw(room);
        player2.draw(room);
        player1.updateInventoryDisplay();
        player2.updateInventoryDisplay();
        break; // Exit riddle loop, continue to normal game
      }
    }
  } else {
    // Normal game start - draw room and start game updates
    if (room) {
      room->draw();
    }
    player1.draw(room);
    player2.draw(room);
    player1.updateInventoryDisplay();
    player2.updateInventoryDisplay();
  }

  while (currentState == GameState::inGame) {
    handleInput();
    update();
    sleep_ms(100); // ~10 FPS
  }
}

//////////////////////////////////////////        handleInput
/////////////////////////////////////////////

void Game::handleInput() {

  while (check_kbhit()) {
    int pressed = get_char_nonblocking();

    if (pressed == -1)
      break;

    // Detect and ignore special key sequences (arrow keys, function keys, etc.)
    // On Windows, special keys send 0 or 224 followed by a key code (apperently
    // :) )
    if (pressed == 0 || pressed == 224) {
      if (check_kbhit()) {
        get_char_nonblocking();
      }
      continue;
    }

    for (int i = 0; i < NUM_KEY_BINDINGS; i++) {
      if (keyBindings[i].key == pressed) {
        if (keyBindings[i].action == Action::ESC) {
          currentState = GameState::paused;
          return;
        }
        Player &player = (keyBindings[i].playerID == 1) ? player1 : player2;

        player.performAction(keyBindings[i].action, getCurrentRoom());
        break;
      }
    }
  }
}

//////////////////////////////////////////          update
/////////////////////////////////////////////

void Game::update() {
  Room *room = getCurrentRoom();
  if (room == nullptr)
    return;

  // Reset obstacle push state for all obstacles each frame
  for (Obstacle *obstacle : room->obstacles) {
    obstacle->resetPushState();
  }

  // Handle player movement
  player1.move(room, &aRiddle.riddle, &aRiddle.player, &player2);
  player2.move(room, &aRiddle.riddle, &aRiddle.player, &player1);

  // Check if either player requested pause (from riddle ESC)
  if (player1.requestPause || player2.requestPause) {
    player1.requestPause = false;
    player2.requestPause = false;
    currentState = GameState::paused;
    return;
  }

  // Update all objects - also returns explosion results if bomb explodes
  ExplosionResult explosionResult = room->updateAllObjects(&player1, &player2);
  if (explosionResult.player1Hit || explosionResult.player2Hit ||
      explosionResult.keyDestroyed) {
    currentState = GameState::gameOver;
    return;
  }

  // Update visibility
  room->updateVisibility(&player1, &player2);
  room->drawDarkness();
  room->drawVisibleObjects();
  player1.draw(room);
  player2.draw(room);

  // Check room transitions
  checkRoomTransitions();
}

//////////////////////////////////////////       getCurrentRoom
/////////////////////////////////////////////

Room *Game::getCurrentRoom() {
  if (currentRoomId >= 0 && currentRoomId < TOTAL_ROOMS)
    return &rooms[currentRoomId];
  return nullptr;
}

//////////////////////////////////////////     redrawCurrentRoom
/////////////////////////////////////////////

void Game::redrawCurrentRoom() {
  clrscr();
  Room *room = getCurrentRoom();
  if (room) {
    room->draw();
  }
  player1.draw(room);
  player2.draw(room);
  player1.updateInventoryDisplay();
  player2.updateInventoryDisplay();

  if (aRiddle.isActive()) {
    // Redraw the active riddle on top
    aRiddle.riddle->draw();
  }
}

/////////////////////////////////////////    canPassThroughDoor
/////////////////////////////////////////

// Check if a door can be passed through (unlocked or requirements met)
bool Game::canPassThroughDoor(Room *room, int doorId) {
  if (room == nullptr)
    return false;

  if (doorId == room->nextRoomId) {
    return room->isDoorUnlocked(doorId) ||
           room->canOpenDoor(doorId, player1.getKeyCount(),
                             player2.getKeyCount());
  } else if (doorId == room->prevRoomId) {
    return true; // Backward doors always passable
  }
  return false;
}

/////////////////////////////////////////      checkRoomTransitions
/////////////////////////////////////////

// Checks if players have reached a door and can pass through, and handles room
// transition if so
void Game::checkRoomTransitions() {

  Room *room = getCurrentRoom();
  if (room == nullptr)
    return;

  // BOTH players at the same door - trigger transition
  if (player1.isAtDoor() && player2.isAtDoor() &&
      player1.getDoorId() == player2.getDoorId()) {
    int doorId = player1.getDoorId();

    if (canPassThroughDoor(room, doorId)) {
      // Reset waiting state
      player1.waitingAtDoor = false;
      player2.waitingAtDoor = false;

      // Forward door check
      if (doorId == room->nextRoomId) {
        if (!room->isDoorUnlocked(doorId)) {
          int keysNeeded = room->doorReqs[doorId].requiredKeys;

          int keysConsumed = 0;
          while (keysConsumed < keysNeeded && player1.getKeyCount() > 0) {
            player1.useKey();
            keysConsumed++;
          }
          while (keysConsumed < keysNeeded && player2.getKeyCount() > 0) {
            player2.useKey();
            keysConsumed++;
          }
        }
        room->unlockDoor(doorId);

        // Check if completing this room triggers victory
        if (currentRoomId == finalRoomId) {
          currentState = GameState::victory;
          return;
        }
        changeRoom(doorId, true);
      }
      // Backward door check
      else if (doorId == room->prevRoomId) {
        changeRoom(doorId, false);
      }
    }
  }
  // ONE player at door - make them wait (cosmetic)
  else if (player1.isAtDoor() && !player2.isAtDoor()) {
    if (canPassThroughDoor(room, player1.getDoorId())) {
      if (!player1.waitingAtDoor) {
        player1.waitingAtDoor = true;
        player1.draw(room); // Redraw to hide
      }
    }
  } else if (player2.isAtDoor() && !player1.isAtDoor()) {
    if (canPassThroughDoor(room, player2.getDoorId())) {
      if (!player2.waitingAtDoor) {
        player2.waitingAtDoor = true;
        player2.draw(room); // Redraw to hide
      }
    }
  }
  // Neither or not at same door - reset waiting state
  else {
    if (player1.waitingAtDoor) {
      player1.waitingAtDoor = false;
      player1.draw(room); // Redraw to show
    }
    if (player2.waitingAtDoor) {
      player2.waitingAtDoor = false;
      player2.draw(room); // Redraw to show
    }
  }
}

//////////////////////////////////////////       Menu Handlers
/////////////////////////////////////////////

void Game::showMainMenu() { mainMenuScreen.draw(); }

void Game::handleMainMenuInput() {
  if (check_kbhit()) {
    char choice = get_single_char();
    switch (choice) {
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

void Game::handleInstructionsInput() {
  if (check_kbhit()) {
    char choice = get_single_char();
    if (choice == char(Action::ESC))
      currentState = GameState::mainMenu;
  }
}

void Game::showPauseMenu() { pauseScreen.draw(); }

void Game::handlePauseInput() {
  if (check_kbhit()) {
    char choice = get_single_char();
    if (choice == char(Action::ESC))
      currentState = GameState::inGame;
    else if (choice == 'h' || choice == 'H') {
      gameInitialized = false; // Reset when going to main menu
      currentState = GameState::mainMenu;
    }
  }
}

void Game::showVictory() { victoryScreen.draw(); }

void Game::showGameOver() { gameOverScreen.draw(); }

void Game::showErrorScreen() {
  initErrorScreen.draw();
  while (check_kbhit())
    get_single_char();
  while (!check_kbhit())
    sleep_ms(50);
  get_single_char();
  currentState = GameState::mainMenu;
}

//////////////////////////////////////////      initializeRooms
////////////////////////////////////////////////

// Initialize all rooms with layouts and requirements
void Game::initializeRooms() {

  rooms.clear();
  loadedScreens.clear();

  // Try to load riddles from file first
  int riddlesLoaded = LevelLoader::loadRiddleFile();
  (void)riddlesLoaded; // Suppress unused variable warning

  // PHASE 1: Load all screens from files
  std::vector<Screen *> screens;
  std::vector<RoomMetadata> metadatas;
  int fileNumber = 1;

  while (true) {
    RoomMetadata metadata;
    Screen *screen = LevelLoader::loadScreenFile(fileNumber, metadata);

    if (screen == nullptr) {
      break; // No more files found
    }

    screens.push_back(screen);
    metadatas.push_back(metadata);
    fileNumber++;
  }

  // If no files were loaded, show error
  if (screens.empty()) {
    currentState = GameState::error;
    return;
  }

  // PHASE 2: Reserve exact capacity (prevents reallocation)
  rooms.reserve(screens.size());
  loadedScreens.reserve(screens.size());

  // PHASE 3: Create all rooms (no reallocation will occur)
  int riddleCounter = 0;
  for (size_t i = 0; i < screens.size(); i++) {
    loadedScreens.push_back(screens[i]);
    rooms.push_back(Room(static_cast<int>(i)));

    Room &room = rooms.back();
    room.initFromLayout(screens[i], &riddleCounter);
    room.spawnPoint = metadatas[i].spawnPoint;
    room.spawnPointFromNext = metadatas[i].spawnPointFromNext;
    room.nextRoomId = metadatas[i].nextRoomId;
    room.prevRoomId = metadatas[i].prevRoomId;

    // Apply door requirements from metadata
    for (const auto &doorConfig : metadatas[i].doorConfigs) {
      room.setDoorRequirements(std::get<0>(doorConfig), std::get<1>(doorConfig),
                               std::get<2>(doorConfig));
    }

    // Apply dark zones from metadata
    for (const auto &dz : metadatas[i].darkZones) {
      room.addDarkZone(dz.x1, dz.y1, dz.x2, dz.y2);
    }

    // Validate Legend Placement
    int validationResult = validateLegendPlacement(room);
    if (validationResult != 0) {
      clrscr();
      std::cout << "Error validating legend in room " << i
                << " (File: " << LevelLoader::getScreenFilename(fileNumber + i)
                << "): Code " << validationResult << std::endl;
      std::cout << "1: No 'L', 2: Multiple 'L', 3: Out of bounds, 4: "
                   "Overlaps/Accessible"
                << std::endl;
      exit(0);
    }
  }

  // Set which room triggers victory when completed
  finalRoomId = static_cast<int>(rooms.size()) - 1;
}

//////////////////////////////////////////        changeRoom
/////////////////////////////////////////////

void Game::changeRoom(int newRoomId, bool goingForward) {

  if (newRoomId < 0 || newRoomId >= TOTAL_ROOMS)
    return;
  if (currentRoomId >= 0) {
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

  clrscr();
  if (getCurrentRoom()) {
    getCurrentRoom()->draw();
  }
  player1.updateInventoryDisplay();
  player2.updateInventoryDisplay();
}

//////////////////////////////////////////        showErrorScreen
/////////////////////////////////////////////

// void Game::showErrorScreen() {
//   initErrorScreen.draw();
//   gotoxy(22, 10);
//   cout <<

// }
