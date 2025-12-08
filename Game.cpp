
#include "Game.h"
#include "Layouts.h"



//////////////////////////////////////////     Game Constructor       //////////////////////////////////////////

Game::Game() 
    : currentState(GameState::mainMenu), currentRoomId(-1) {}

Game::~Game() {}

//////////////////////////////////////////           run              //////////////////////////////////////////

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
                startNewGame();
                gameLoop();
                break;

            case GameState::paused:
                showPauseMenu();
                while (currentState == GameState::paused) {
                    handlePauseInput();
                    sleep_ms(50);
                }
                if (currentState == GameState::inGame) redrawCurrentRoom();
                break;

            case GameState::victory:
                showVictory();
                while (check_kbhit()) get_single_char();
                while (!check_kbhit()) sleep_ms(50);
                get_single_char();
                currentState = GameState::mainMenu;
                break;

            case GameState::gameOver:
                showGameOver();
                while (check_kbhit()) get_single_char();
                while (!check_kbhit()) sleep_ms(50);
                get_single_char();
                currentState = GameState::mainMenu;
                break;

            case GameState::quit:
                running = false;
                break;
        }
    }
}

//////////////////////////////////////////       startNewGame         //////////////////////////////////////////

void Game::startNewGame() {
    initializeRooms();

    player1 = Player(1, 5, 7, PlayerSprites::PLAYER1);
    player2 = Player(2, 5, 9, PlayerSprites::PLAYER2);

    currentRoomId = 0;
    rooms[0].active = true;
}

//////////////////////////////////////////         gameLoop           //////////////////////////////////////////

void Game::gameLoop() {
    if(getCurrentRoom()) { getCurrentRoom()->draw(); }
    player1.draw();
    player2.draw();
    player1.updateInventoryDisplay();
    player2.updateInventoryDisplay();

    while (currentState == GameState::inGame) {
        handleInput();
        update();
        sleep_ms(100);  // ~10 FPS
    }
}

//////////////////////////////////////////        handleInput         //////////////////////////////////////////

void Game::handleInput() {

    while (check_kbhit()) {
        int pressed = get_char_nonblocking();
        
        if (pressed == -1) break;
        if (pressed == 27) {  // ESC = pause
            currentState = GameState::paused;
            return;
        }

        for (int i = 0; i < NUM_KEY_BINDINGS; i++) {
            if (keyBindings[i].key == pressed) {
                Player& player = (keyBindings[i].playerID == 1) ? player1 : player2;

                if (keyBindings[i].action == Action::DROP_ITEM) {
                    if (getCurrentRoom()) getCurrentRoom()->handleBombDrop(player);
                } else {
                    player.performAction(keyBindings[i].action);
                }
                break;
            }
        }
    }
}

//////////////////////////////////////////          update            //////////////////////////////////////////

void Game::update() {
    Room* room = getCurrentRoom();
    if (room == nullptr) return;

    // Handle player movement
    player1.move(room);
    player2.move(room);

    // Update bomb
    bool causedGameOver = room->updateBomb(&player1, &player2);
    if (causedGameOver) {
        currentState = GameState::gameOver;
        return;
    }
    
    // Update visibility
    room->updateVisibility(&player1, &player2);
    room->drawDarkness();
    player1.draw();
    player2.draw();

    // Check room transitions
    checkRoomTransitions();

    // Victory check
    if (currentRoomId == TOTAL_ROOMS - 1 && currentState != GameState::victory) 
        { currentState = GameState::victory; }
}

//////////////////////////////////////////       getCurrentRoom       //////////////////////////////////////////

Room* Game::getCurrentRoom() {
    if (currentRoomId >= 0 && currentRoomId < TOTAL_ROOMS) return &rooms[currentRoomId];
    return nullptr;
}

//////////////////////////////////////////     redrawCurrentRoom      //////////////////////////////////////////

void Game::redrawCurrentRoom() {
    clrscr();
    if (getCurrentRoom()) { getCurrentRoom()->draw(); }
    player1.draw();
    player2.draw();
    player1.updateInventoryDisplay();
    player2.updateInventoryDisplay();
}

/////////////////////////////////////////      checkRoomTransitions       //////////////////////////////////////

// Checks if players have reached a door and can pass through, and handles room transition if so
void Game::checkRoomTransitions() {

    Room* room = getCurrentRoom();
    if (room == nullptr) return;

    // Check if both players are at the same door
    if (!player1.isAtDoor() || !player2.isAtDoor()) return;
    if (player1.getDoorId() != player2.getDoorId()) return;
    
    int doorId = player1.getDoorId();
    
    // Forward door check
    if (doorId == room->nextRoomId) {
        if (room->isDoorUnlocked(doorId) ||
            room->canOpenDoor(doorId, player1.getKeyCount(), player2.getKeyCount())) {
            
                if (!room->isDoorUnlocked(doorId)) {
                    int keysNeeded = room->doorReqs[doorId].requiredKeys;
                    for (int i = 0; i < keysNeeded; i++) {
                        player1.useKey();
                        player2.useKey();
                    }
                }
                room->unlockDoor(doorId);
                
                // Forward door
                if (doorId == room->nextRoomId)      { changeRoom(doorId, true); }
            }  
        // Backward door
        else if (doorId == room->prevRoomId) { changeRoom(doorId, false); }
    }

}

//////////////////////////////////////////       Menu Handlers        //////////////////////////////////////////

void Game::showMainMenu() {
    mainMenuScreen.draw();
}

void Game::handleMainMenuInput() {
    if (check_kbhit()) {
        char choice = get_single_char();
        switch (choice) {
            case '1': currentState = GameState::inGame; break;
            case '8': currentState = GameState::instructions; break;
            case '9': currentState = GameState::quit; break;
        }
    }
}

void Game::showInstructions() {
    instructionsScreen.draw();
}

void Game::handleInstructionsInput() {
    if (check_kbhit()) {
        char choice = get_single_char();
        if (choice == 27) currentState = GameState::mainMenu;  // ESC
    }
}

void Game::showPauseMenu() {
    pauseScreen.draw();
}

void Game::handlePauseInput() {
    if (check_kbhit()) {
        char choice = get_single_char();
        if (choice == 27) currentState = GameState::inGame;  // ESC = resume
        else if (choice == 'h' || choice == 'H') currentState = GameState::mainMenu;
    }
}

void Game::showVictory() {
    victoryScreen.draw();
}

void Game::showGameOver() {
    gameOverScreen.draw();
}

//////////////////////////////////////////      initializeRooms       //////////////////////////////////////////

// Initialize all rooms with layouts and requirements
void Game::initializeRooms() {
    // Room 0: Switch puzzle
    rooms[0] = Room(0);
    rooms[0].initFromLayout(&room0Layout);
    rooms[0].spawnPoint = Point(3, 5, 0, 0, ' ');
    rooms[0].spawnPointFromNext = Point(75, 17, 0, 0, ' ');
    rooms[0].nextRoomId = 1;
    rooms[0].prevRoomId = -1;
    rooms[0].setDoorRequirements(1, 1, 0);  // Door 1: 1 key per player

    // Room 1: Bomb puzzle with dark zone
    rooms[1] = Room(1);
    rooms[1].initFromLayout(&room1Layout);
    rooms[1].spawnPoint = Point(3, 5, 0, 0, ' ');
    rooms[1].spawnPointFromNext = Point(75, 17, 0, 0, ' ');
    rooms[1].nextRoomId = 2;
    rooms[1].prevRoomId = 0;
    rooms[1].setDoorRequirements(2, 1, 0);  // Door 2: 1 key per player
    rooms[1].addDarkZone(21, 5, 46, 14);    // Dark zone inside inner room

    // Room 2: Final room (victory)
    rooms[2] = Room(2);
    rooms[2].initFromLayout(&room2Layout);
    rooms[2].spawnPoint = Point(3, 10, 0, 0, ' ');
    rooms[2].spawnPointFromNext = Point(3, 10, 0, 0, ' ');
    rooms[2].nextRoomId = -1;
    rooms[2].prevRoomId = 1;
}

//////////////////////////////////////////        changeRoom          //////////////////////////////////////////

void Game::changeRoom(int newRoomId, bool goingForward) {
    
    if (newRoomId < 0 || newRoomId >= TOTAL_ROOMS) return;
    if (currentRoomId >= 0) { rooms[currentRoomId].active = false; }

    currentRoomId = newRoomId;
    rooms[newRoomId].active = true;

    Point spawn = goingForward ? rooms[newRoomId].spawnPoint : rooms[newRoomId].spawnPointFromNext;
    
    player1.setPosition(spawn.x, spawn.y);
    player2.setPosition(spawn.x, spawn.y + 1);  // Slight offset for 2nd player
    
    player1.pos.diff_x = 0; player1.pos.diff_y = 0;
    player2.pos.diff_x = 0; player2.pos.diff_y = 0;
    player1.atDoor = false; player2.atDoor = false;

    clrscr();
    if(getCurrentRoom()) { getCurrentRoom()->draw(); }
    player1.updateInventoryDisplay();
    player2.updateInventoryDisplay();

}

