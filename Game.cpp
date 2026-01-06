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
    : initErrorMessage(0), initErrorRoomId(-1), gameOverMessege(GameOverMessege::NONE),
      currentState(GameState::mainMenu), currentRoomId(-1),
      gameInitialized(false) {}

//////////////////////////////////////////      Game Destructor       /////////////////////////////////////////////

Game::~Game() {
  // Clean up loaded screens
  for (Screen *s : loadedScreens) {
    delete s;
  }
  loadedScreens.clear();
}

//////////////////////////////////////////           run               /////////////////////////////////////////////

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
      break;

    case GameState::victory:
      showVictory();
      while (check_kbhit())
        get_single_char();
      while (!check_kbhit())
        sleep_ms(50);
      get_single_char();
      gameInitialized = false;
      currentState = GameState::mainMenu;
      break;

    case GameState::gameOver:
      showGameOver();
      while (check_kbhit())
        get_single_char();
      while (!check_kbhit())
        sleep_ms(50);
      get_single_char();
      gameInitialized = false;
      currentState = GameState::mainMenu;
      break;

    case GameState::error:
      showErrorScreen();
      while (check_kbhit())
        get_single_char();
      while (!check_kbhit())
        sleep_ms(50);
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
      if (c != ObjectType::WALL && c != ObjectType::AIR && c != 'L') {
        return 4;
      }
    }
  }

  // 4b. Check for overlap with Spawn Points (Player locations)
  // Check primary spawn point
  if (room.spawnPoint.x >= topLeftX && room.spawnPoint.x < topLeftX + width &&
      room.spawnPoint.y >= topLeftY && room.spawnPoint.y < topLeftY + height) {
    return 5;
  }
  // Check secondary spawn point (from next room)
  if (room.spawnPointFromNext.x >= topLeftX &&
      room.spawnPointFromNext.x < topLeftX + width &&
      room.spawnPointFromNext.y >= topLeftY &&
      room.spawnPointFromNext.y < topLeftY + height) {
    return 5;
  }

  room.setLegendPoint(lPos.x, lPos.y);

  return 0; // Valid
}

//////////////////////////////////////////       startNewGame         /////////////////////////////////////////////

void Game::startNewGame() {
  initializeRooms();

  player1 = Player(1, 5, 7, PlayerSprites::PLAYER1);
  player2 = Player(2, 5, 9, PlayerSprites::PLAYER2);

  currentRoomId = 0;
  rooms[0].active = true;
  gameInitialized = true;
}

//////////////////////////////////////////         gameLoop           /////////////////////////////////////////////

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
    if (room) room->drawLegend(&player1, &player2);


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
        if (room) room->drawLegend(&player1, &player2);
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
        if (room) room->drawLegend(&player1, &player2);
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
    if (room) room->drawLegend(&player1, &player2);
  }

  while (currentState == GameState::inGame) {
    handleInput();
    update();
    sleep_ms(100); // ~10 FPS
  }
}

//////////////////////////////////////////        handleInput         /////////////////////////////////////////////

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

//////////////////////////////////////////          update            /////////////////////////////////////////////

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
  if (explosionResult.player1Hit) player1.loseLife(room);
  if (explosionResult.player2Hit) player2.loseLife(room);

  if (checkGameOver(explosionResult)) {
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

  std::cout << std::flush;
}

//////////////////////////////////////////       getCurrentRoom       /////////////////////////////////////////////

Room *Game::getCurrentRoom() {
  if (currentRoomId >= 0 && currentRoomId < rooms.size())
    return &rooms[currentRoomId];
  return nullptr;
}

//////////////////////////////////////////     redrawCurrentRoom      /////////////////////////////////////////////

void Game::redrawCurrentRoom() {
  clrscr();
  Room *room = getCurrentRoom();
  if (room) {
    room->draw();
  }
  player1.draw(room);
  player2.draw(room);
  if (room) room->drawLegend(&player1, &player2);


  if (aRiddle.isActive()) {
    // Redraw the active riddle on top
    aRiddle.riddle->draw();
  }
}

/////////////////////////////////////////    canPassThroughDoor       /////////////////////////////////////////////

// Check if a door can be passed through (unlocked or requirements met)
bool Game::canPassThroughDoor(Room *room, int doorId) {
  if (room == nullptr)
    return false;

  if (doorId == room->nextRoomId || doorId == rooms.size()) {
    return room->isDoorUnlocked(doorId) ||
           room->canOpenDoor(doorId, player1.getKeyCount(),
                             player2.getKeyCount());

  } else if (doorId == room->prevRoomId) {
    return true; // Backward doors always passable
  }
  return false;
}

/////////////////////////////////////////      checkRoomTransitions   /////////////////////////////////////////////

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
      if (doorId == room->nextRoomId || doorId == rooms.size()) {
        if (!room->isDoorUnlocked(doorId)) {
          int keysNeeded = (doorId >= 0 && doorId < static_cast<int>(room->doorReqs.size())) 
                            ? room->doorReqs[doorId].requiredKeys : 0;

          int keysConsumed = 0;
          while (keysConsumed < keysNeeded && player1.getKeyCount() > 0) {
            player1.useKey();
            keysConsumed++;
          }
          while (keysConsumed < keysNeeded && player2.getKeyCount() > 0) {
            player2.useKey();
            keysConsumed++;
          }

          // Award points for unlocking the door (first time only)

          player1.incrementScore(100 * player1.getLives());
          player2.incrementScore(100 * player2.getLives());
        }
        room->unlockDoor(doorId);

        // Check if completing this room triggers victory
        if (currentRoomId == -1) {
          currentState = GameState::victory;
          return;
        }
        changeRoom(room->nextRoomId, true);
      }
      // Backward door check
      else if (doorId == room->prevRoomId) {
        changeRoom(room->prevRoomId, false);
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

//////////////////////////////////////////       Menu Handlers        /////////////////////////////////////////////

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
    if (choice == static_cast<char>(Action::ESC))
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

void Game::showVictory() { 
  victoryScreen.draw();
  gotoxy(40, 13);
  cout << player1.getScore() << endl;
  gotoxy(40, 14);
  cout << player2.getScore() << endl;
}

void Game::showGameOver() { 

  gameOverScreen.draw(); 

  switch (gameOverMessege) {
  case GameOverMessege::PLAYER1_DIED:
    gotoxy(24, 8);
    cout << "Player 1 ($)  has died." << endl;
    break;
  case GameOverMessege::PLAYER2_DIED:
    gotoxy(24,8);
    cout << "Player 2 (&)  has died." << endl;
    break;
  case GameOverMessege::VALUABLE_DESTROYED:
    gotoxy(18,8);
    cout << "An essential object has been destroyed." << endl;
    gotoxy(18,9);
    cout << " The game cannot continue without it." << endl;
    break;
  default:
    gotoxy(24,8);
    cout << "Unknown game over messege." << endl;
    break;
  }
}

void Game::showErrorScreen() {
  initErrorScreen.draw();
  gotoxy(22, 10);
  switch (initErrorMessage) {
  case 1:
    cout << "Error: No 'L' found in room " << initErrorRoomId << endl;
    break;
  case 2:
    cout << "Error: Multiple 'L's found in room " << initErrorRoomId << endl;
    break;
  case 3:
    cout << "Error: 'L' out of bounds in room " << initErrorRoomId << endl;
    break;
  case 4:
    cout << "Error: Legend obscured objects in room " << initErrorRoomId << endl;
    break;
  case 5:
    cout << "Error: Legend obscured a player's spawn point in room " << initErrorRoomId << endl;
    break;
  default:
    cout << "Unknown error" << endl;
    break;
  }
}

//////////////////////////////////////////      initializeRooms       /////////////////////////////////////////////

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

void Game::changeRoom(int newRoomId, bool goingForward) {

  if (newRoomId < -1 || newRoomId > rooms.size()){
    gotoxy(1,1);
    std::cout << "newRoomId: " << newRoomId << std::endl;
    std::cout << "rooms.size(): " << rooms.size() << std::endl;
    return;}
  if (currentRoomId >= 0) {
    rooms[currentRoomId].active = false;
  }

  if (newRoomId == rooms.size()) {
    gotoxy(1,1);
    std::cout << "newRoomId: " << newRoomId << std::endl;
    std::cout << "rooms.size(): " << rooms.size() << std::endl;
    currentState = GameState::victory;
    return;
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
    getCurrentRoom()->drawLegend(&player1, &player2);
  }

}

//////////////////////////////////////////       checkGameOver        /////////////////////////////////////////////

bool Game::checkGameOver(const ExplosionResult& result) {
  if (player1.isDead()) {
    setGameOverMessege(GameOverMessege::PLAYER1_DIED);
    return true;
  } 

  if (player2.isDead()) {
    setGameOverMessege(GameOverMessege::PLAYER2_DIED);
    return true;
  }

  Room* room = getCurrentRoom();
  int neededSwitches = room->getDoorReqSwitches(room->nextRoomId);
  int totalSwitches = room->getTotalSwitches();
  
  if (result.keyDestroyed) {
    setGameOverMessege(GameOverMessege::VALUABLE_DESTROYED);
    return true;
  }

  if (neededSwitches > 0 && totalSwitches < neededSwitches) {
    setGameOverMessege(GameOverMessege::VALUABLE_DESTROYED);
    return true;
  }

  return false;
}
