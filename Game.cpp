#include "Game.h"
#include "Layouts.h"

Game::Game() {
    currentState = GameState::mainMenu;
    currentRoomId = -1;
    player1DropRequested = false;
    player2DropRequested = false;
}

Game::~Game() {
    // Cleanup if needed
}

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
                // Redraw current room if resuming
                if (currentState == GameState::inGame) {
                    getCurrentRoom()->draw();
                    player1.draw();
                    player2.draw();
                    player1.updateInventoryDisplay();
                    player2.updateInventoryDisplay();
                }
                break;

            case GameState::victory:
                showVictory();
                while (check_kbhit()) get_single_char();  // Clear buffer
                while (!check_kbhit()) sleep_ms(50);
                get_single_char();
                currentState = GameState::mainMenu;
                break;

            case GameState::gameOver:
                showGameOver();
                while (check_kbhit()) get_single_char();  // Clear buffer
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

void Game::showMainMenu() {
    mainMenuScreen.draw();
}

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

void Game::showInstructions() {
    instructionsScreen.draw();
}

void Game::handleInstructionsInput() {
    if (check_kbhit()) {
        char choice = get_single_char();
        if (choice == 27) {  // ESC key
            currentState = GameState::mainMenu;
        }
    }
}

void Game::showPauseMenu() {
    pauseScreen.draw();
}

void Game::handlePauseInput() {
    if (check_kbhit()) {
        char choice = get_single_char();
        if (choice == 27) {  // ESC - resume
            currentState = GameState::inGame;
        } else if (choice == 'h' || choice == 'H') {  // Return to main menu
            currentState = GameState::mainMenu;
        }
    }
}

void Game::showVictory() {
    victoryScreen.draw();
}

void Game::showGameOver() {
    gameOverScreen.draw();
}

void Game::initializeRooms() {
    // Initialize Room 0 - Switch puzzle (first room, no previous)
    rooms[0] = Room(0);
    rooms[0].initFromLayout(&room0Layout);
    rooms[0].spawnPoint = Point(3, 5, 0, 0, ' ');           // Coming from menu/start
    rooms[0].spawnPointFromNext = Point(75, 17, 0, 0, ' '); // Coming back from room 1
    rooms[0].nextRoomId = 1;
    rooms[0].prevRoomId = -1;  // No previous room

    // Initialize Room 1 - Bomb puzzle with dark zone
    rooms[1] = Room(1);
    rooms[1].initFromLayout(&room1Layout);
    rooms[1].spawnPoint = Point(3, 5, 0, 0, ' ');           // Coming from room 0
    rooms[1].spawnPointFromNext = Point(75, 17, 0, 0, ' '); // Coming back from room 2
    rooms[1].nextRoomId = 2;
    rooms[1].prevRoomId = 0;
    rooms[1].requiredSwitches = 0;  // No switches in this room
    // Add dark zone inside the inner room (where the key is)
    rooms[1].addDarkZone(21, 5, 46, 14);

    // Initialize Room 2 (Final room)
    rooms[2] = Room(2);
    rooms[2].initFromLayout(&room2Layout);
    rooms[2].spawnPoint = Point(3, 10, 0, 0, ' ');          // Coming from room 1
    rooms[2].spawnPointFromNext = Point(3, 10, 0, 0, ' ');  // No next room
    rooms[2].nextRoomId = -1;  // No next room (victory)
    rooms[2].prevRoomId = 1;
    rooms[2].requiredSwitches = 0;
}

void Game::startNewGame() {
    // Initialize rooms
    initializeRooms();

    // Initialize players
    player1 = Player(1, 5, 7, '$');
    player2 = Player(2, 5, 9, '&');
    
    player1.prevChar = ' ';
    player2.prevChar = ' ';
    player1.alive = true;
    player2.alive = true;

    // Set current room
    currentRoomId = 0;
    rooms[0].active = true;
}

void Game::gameLoop() {
    // Draw initial state
    getCurrentRoom()->draw();
    player1.draw();
    player2.draw();
    player1.updateInventoryDisplay();
    player2.updateInventoryDisplay();

    while (currentState == GameState::inGame) {
        handleInput();
        update();
        sleep_ms(100);  // Game speed ~10 FPS
    }
}

void Game::handleInput() {
    player1DropRequested = false;
    player2DropRequested = false;

    // Process all available input
    while (check_kbhit()) {
        int pressed = get_char_nonblocking();
        if (pressed == -1) break;

        // Check for ESC (pause)
        if (pressed == 27) {
            currentState = GameState::paused;
            return;
        }

        // Check key bindings
        for (int i = 0; i < NUM_KEY_BINDINGS; i++) {
            if (keyBindings[i].key == pressed) {
                if (keyBindings[i].playerID == 1) {
                    if (keyBindings[i].action == Action::DROP_ITEM) {
                        player1DropRequested = true;
                    } else {
                        player1.performAction(keyBindings[i].action);
                    }
                } else {
                    if (keyBindings[i].action == Action::DROP_ITEM) {
                        player2DropRequested = true;
                    } else {
                        player2.performAction(keyBindings[i].action);
                    }
                }
                break;
            }
        }
    }
}

void Game::update() {
    Room* room = getCurrentRoom();
    if (room == nullptr) return;

    // Handle drop requests
    if (player1DropRequested) {
        handleBombDrop(player1);
    }
    if (player2DropRequested) {
        handleBombDrop(player2);
    }

    // Update bomb timer
    updateBomb();

    // Check if game over from bomb
    if (currentState != GameState::inGame) return;

    // Move both players
    player1.move(room);
    player2.move(room);
    
    // Update visibility (for dark zones with torches)
    room->updateVisibility(&player1, &player2);
    room->drawDarkness();
    
    // Redraw players on top of darkness
    player1.draw();
    player2.draw();

    // Check if both players are at a door
    int doorResult = checkPlayersAtDoor();
    
    if (doorResult > 0) {
        // Going forward to next room
        int nextRoom = room->nextRoomId;
        
        if (nextRoom >= 0 && nextRoom < TOTAL_ROOMS) {
            // Mark door as unlocked for both directions
            room->doorToNextUnlocked = true;
            rooms[nextRoom].doorToPrevUnlocked = true;
            
            // Only use keys if room doesn't have switches and door wasn't already unlocked
            if (room->requiredSwitches == 0 && !room->doorToNextUnlocked) {
                player1.useKey();
                player2.useKey();
            }
            changeRoom(nextRoom, true);
        } else if (nextRoom == -1 && currentRoomId == TOTAL_ROOMS - 1) {
            // Victory!
            currentState = GameState::victory;
        }
    } else if (doorResult < 0) {
        // Going back to previous room
        int prevRoom = room->prevRoomId;
        
        if (prevRoom >= 0 && prevRoom < TOTAL_ROOMS) {
            changeRoom(prevRoom, false);
        }
    }

    // Check for victory condition in final room
    if (currentRoomId == TOTAL_ROOMS - 1) {
        // Both players in final room = victory
        currentState = GameState::victory;
    }
}

Room* Game::getCurrentRoom() {
    if (currentRoomId >= 0 && currentRoomId < TOTAL_ROOMS) {
        return &rooms[currentRoomId];
    }
    return nullptr;
}

int Game::checkPlayersAtDoor() {
    // Both players must be at a door
    if (!player1.isAtDoor() || !player2.isAtDoor()) {
        return 0;
    }
    
    // Check if they're at the same door
    if (player1.getDoorId() != player2.getDoorId()) {
        return 0;
    }
    
    Room* room = getCurrentRoom();
    if (room == nullptr) return 0;
    
    int doorId = player1.getDoorId();
    
    // Check if this is the door to the next room
    // Door ID matches nextRoomId means going forward
    if (doorId == room->nextRoomId) {
        // Check if door is unlocked
        if (room->doorToNextUnlocked) {
            return 1;  // Already unlocked, can pass
        }
        
        // If room has switches, door is unlocked when all switches are on
        if (room->requiredSwitches > 0) {
            if (room->isDoorUnlocked()) {
                return 1;
            }
            return 0;  // Switches not all on
        }
        
        // Otherwise, both players must have keys
        if (player1.hasKey() && player2.hasKey()) {
            return 1;
        }
        return 0;  // No keys
    }
    
    // Check if this is the door to the previous room
    // Door ID matches prevRoomId means going backward
    if (doorId == room->prevRoomId) {
        // Door to previous room is always unlocked if we came from there
        if (room->doorToPrevUnlocked) {
            return -1;  // Go back
        }
        return 0;  // Door not unlocked (shouldn't happen normally)
    }
    
    return 0;  // Unknown door
}

void Game::handleBombDrop(Player& player) {
    // Check if player has a bomb
    if (player.inventory.type != ObjectType::BOMB) {
        // Try regular drop
        player.dropItem(getCurrentRoom());
        return;
    }
    
    Room* room = getCurrentRoom();
    if (room == nullptr) return;
    
    // Don't allow dropping if a bomb is already active in this room
    if (room->bomb.active) {
        return;
    }
    
    // Drop the bomb
    Point dropPos = player.dropItem(room);
    
    if (dropPos.x >= 0 && dropPos.y >= 0) {
        // Start the bomb timer in this room
        room->bomb.x = dropPos.x;
        room->bomb.y = dropPos.y;
        room->bomb.fuseTimer = BOMB_FUSE_TIME;
        room->bomb.active = true;
    }
}

void Game::updateBomb() {
    Room* room = getCurrentRoom();
    if (room == nullptr) return;
    
    if (!room->bomb.active) return;
    
    room->bomb.fuseTimer--;
    
    // Visual feedback - blink the bomb
    if (room->bomb.fuseTimer % 10 < 5) {
        gotoxy(room->bomb.x, room->bomb.y);
        std::cout << '@';
    } else {
        gotoxy(room->bomb.x, room->bomb.y);
        std::cout << '*';
    }
    std::cout.flush();
    
    // Explode when timer reaches 0
    if (room->bomb.fuseTimer <= 0) {
        // Remove the bomb object first
        room->removeObjectAt(room->bomb.x, room->bomb.y);
        room->setCharAt(room->bomb.x, room->bomb.y, ' ');
        gotoxy(room->bomb.x, room->bomb.y);
        std::cout << ' ';
        std::cout.flush();
        
        // Explode!
        ExplosionResult result = room->explodeBomb(
            room->bomb.x, room->bomb.y, BOMB_RADIUS,
            &player1, &player2
        );
        
        // Redraw players after explosion
        player1.draw();
        player2.draw();
        
        // Check for game over conditions
        if (result.keyDestroyed || result.player1Hit || result.player2Hit) {
            currentState = GameState::gameOver;
        }
        
        // Deactivate bomb
        room->bomb.active = false;
    }
}

void Game::changeRoom(int newRoomId, bool goingForward) {
    if (newRoomId < 0 || newRoomId >= TOTAL_ROOMS) return;

    // Deactivate current room (but keep its state, including bomb timer)
    if (currentRoomId >= 0) {
        rooms[currentRoomId].active = false;
    }

    // Activate new room
    currentRoomId = newRoomId;
    rooms[newRoomId].active = true;

    // Set player positions based on direction
    Point spawn;
    if (goingForward) {
        spawn = rooms[newRoomId].spawnPoint;
    } else {
        spawn = rooms[newRoomId].spawnPointFromNext;
    }
    
    player1.setPosition(spawn.x, spawn.y);
    player2.setPosition(spawn.x, spawn.y + 2);
    player1.pos.diff_x = 0;
    player1.pos.diff_y = 0;
    player2.pos.diff_x = 0;
    player2.pos.diff_y = 0;
    player1.prevChar = ' ';
    player2.prevChar = ' ';
    player1.atDoor = false;
    player2.atDoor = false;

    // Draw new room
    rooms[newRoomId].draw();
    player1.draw();
    player2.draw();
    player1.updateInventoryDisplay();
    player2.updateInventoryDisplay();
}
