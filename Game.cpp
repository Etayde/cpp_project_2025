//////////////////////////////////////       INCLUDES & FORWARDS       //////////////////////////////////////////

#include "Game.h"
#include "Layouts.h"

//////////////////////////////////////////     Game Constructor       //////////////////////////////////////////

Game::Game()
    : currentState(GameState::mainMenu), currentRoomId(-1) {}

Game::~Game() {}

//////////////////////////////////////////           run              //////////////////////////////////////////

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
                sleep_ms(50);
            }
            break;

        case GameState::instructions:
            showInstructions();
            while (currentState == GameState::instructions)
            {
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
            while (currentState == GameState::paused)
            {
                handlePauseInput();
                sleep_ms(50);
            }
            if (currentState == GameState::inGame)
                redrawCurrentRoom();
            break;

        case GameState::victory:
            showVictory();
            while (check_kbhit())
                get_single_char();
            while (!check_kbhit())
                sleep_ms(50);
            get_single_char();
            currentState = GameState::mainMenu;
            break;

        case GameState::gameOver:
            showGameOver();
            while (check_kbhit())
                get_single_char();
            while (!check_kbhit())
                sleep_ms(50);
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

void Game::startNewGame()
{
    initializeRooms();

    player1 = Player(1, 5, 7, PlayerSprites::PLAYER1);
    player2 = Player(2, 5, 9, PlayerSprites::PLAYER2);

    currentRoomId = 0;
    rooms[0].active = true;
}

//////////////////////////////////////////         gameLoop           //////////////////////////////////////////

void Game::gameLoop()
{
    Room *room = getCurrentRoom();
    if (room)
    {
        room->draw();
    }
    player1.draw(room);
    player2.draw(room);
    player1.updateInventoryDisplay();
    player2.updateInventoryDisplay();

    while (currentState == GameState::inGame)
    {
        handleInput();
        update();
        sleep_ms(100); // ~10 FPS
    }
}

//////////////////////////////////////////        handleInput         //////////////////////////////////////////

void Game::handleInput()
{

    while (check_kbhit())
    {
        int pressed = get_char_nonblocking();

        if (pressed == -1)
            break;

        // Detect and ignore special key sequences (arrow keys, function keys, etc.)
        // On Windows, special keys send 0 or 224 followed by a key code (apperently :) )
        if (pressed == 0 || pressed == 224)
        {
            if (check_kbhit())
            {
                get_char_nonblocking();
            }
            continue;
        }

        if (pressed == 27)
        { // ESC
            currentState = GameState::paused;
            return;
        }

        for (int i = 0; i < NUM_KEY_BINDINGS; i++)
        {
            if (keyBindings[i].key == pressed)
            {
                Player &player = (keyBindings[i].playerID == 1) ? player1 : player2;

                if (keyBindings[i].action == Action::DROP_ITEM)
                {
                    if (getCurrentRoom())
                        getCurrentRoom()->handleBombDrop(player);
                }
                else
                {
                    player.performAction(keyBindings[i].action, getCurrentRoom());
                }
                break;
            }
        }
    }
}

//////////////////////////////////////////          update            //////////////////////////////////////////

void Game::update()
{
    Room *room = getCurrentRoom();
    if (room == nullptr)
        return;

    // Handle player movement
    player1.move(room);
    player2.move(room);

    // Update bomb
    bool causedGameOver = room->updateBomb(&player1, &player2);
    if (causedGameOver)
    {
        currentState = GameState::gameOver;
        return;
    }

    // Update visibility
    room->updateVisibility(&player1, &player2);
    room->drawDarkness();
    player1.draw(room);
    player2.draw(room);

    // Check room transitions
    checkRoomTransitions();
}

//////////////////////////////////////////       getCurrentRoom       //////////////////////////////////////////

Room *Game::getCurrentRoom()
{
    if (currentRoomId >= 0 && currentRoomId < TOTAL_ROOMS)
        return &rooms[currentRoomId];
    return nullptr;
}

//////////////////////////////////////////     redrawCurrentRoom      //////////////////////////////////////////

void Game::redrawCurrentRoom()
{
    clrscr();
    Room *room = getCurrentRoom();
    if (room)
    {
        room->draw();
    }
    player1.draw(room);
    player2.draw(room);
    player1.updateInventoryDisplay();
    player2.updateInventoryDisplay();
}

/////////////////////////////////////////    canPassThroughDoor       //////////////////////////////////////

// Check if a door can be passed through (unlocked or requirements met)
bool Game::canPassThroughDoor(Room *room, int doorId)
{
    if (room == nullptr)
        return false;

    if (doorId == room->nextRoomId)
    {
        return room->isDoorUnlocked(doorId) ||
               room->canOpenDoor(doorId, player1.getKeyCount(), player2.getKeyCount());
    }
    else if (doorId == room->prevRoomId)
    {
        return true; // Backward doors always passable
    }
    return false;
}

/////////////////////////////////////////      checkRoomTransitions       //////////////////////////////////////

// Checks if players have reached a door and can pass through, and handles room transition if so
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
            if (doorId == room->nextRoomId)
            {
                if (!room->isDoorUnlocked(doorId))
                {
                    int keysNeeded = room->doorReqs[doorId].requiredKeys;

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
                }
                room->unlockDoor(doorId);

                // Check if completing this room triggers victory
                if (currentRoomId == finalRoomId)
                {
                    currentState = GameState::victory;
                    return;
                }
                changeRoom(doorId, true);
            }
            // Backward door check
            else if (doorId == room->prevRoomId)
            {
                changeRoom(doorId, false);
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

//////////////////////////////////////////       Menu Handlers        //////////////////////////////////////////

void Game::showMainMenu()
{
    mainMenuScreen.draw();
}

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

void Game::showInstructions()
{
    instructionsScreen.draw();
}

void Game::handleInstructionsInput()
{
    if (check_kbhit())
    {
        char choice = get_single_char();
        if (choice == 27)
            currentState = GameState::mainMenu;
    }
}

void Game::showPauseMenu()
{
    pauseScreen.draw();
}

void Game::handlePauseInput()
{
    if (check_kbhit())
    {
        char choice = get_single_char();
        if (choice == 27)
            currentState = GameState::inGame;
        else if (choice == 'h' || choice == 'H')
            currentState = GameState::mainMenu;
    }
}

void Game::showVictory()
{
    victoryScreen.draw();
}

void Game::showGameOver()
{
    gameOverScreen.draw();
}

//////////////////////////////////////////      initializeRooms       //////////////////////////////////////////

// Initialize all rooms with layouts and requirements
void Game::initializeRooms()
{
    // Set which room triggers victory when completed
    finalRoomId = 1; // Game ends after completing room 1 - can be changed as needed

    // Initialize rooms vector
    rooms.resize(2);

    // Room 0
    rooms[0] = Room(0);
    rooms[0].initFromLayout(&room0Layout);
    rooms[0].spawnPoint = Point(3, 5, 0, 0, ' ');
    rooms[0].spawnPointFromNext = Point(75, 17, 0, 0, ' ');
    rooms[0].nextRoomId = 1;
    rooms[0].prevRoomId = -1;
    rooms[0].setDoorRequirements(1, 0, 2); // Door 1: 0 keys, 2 switches
    rooms[0].addSpring({Point(73, 9), Point(74, 9), Point(75, 9), Point(76, 9), Point(77, 9)}, 5); //spring 1
    rooms[0].addSpring({Point(51, 1), Point(51, 2), Point(51, 3), Point(51, 4)}, 4); //spring 2
    rooms[0].addSpring({Point(1, 10), Point(2, 10), Point(3, 10)}, 3); //spring 3
    rooms[0].addSpring({Point(62, 18), Point(62, 19)}, 2); //spring 4
    // Example: Add a 3-character horizontal spring at positions (10,10), (11,10), (12,10)
    // Make sure there's a wall at (9,10) - the spring will project right
    // rooms[0].addSpring({Point(10, 10), Point(11, 10), Point(12, 10)}, 3);

    // Room 1
    rooms[1] = Room(1);
    rooms[1].initFromLayout(&room1Layout);
    rooms[1].spawnPoint = Point(3, 5, 0, 0, ' ');
    rooms[1].spawnPointFromNext = Point(75, 17, 0, 0, ' ');
    rooms[1].nextRoomId = 2; // Door 2 is the forward door (triggers victory)
    rooms[1].prevRoomId = 0;
    rooms[1].setDoorRequirements(0, 0, 0); // Door 0: No requirements
    rooms[1].setDoorRequirements(2, 2, 0); // Door 2: 2 keys, 0 switches
    rooms[1].addDarkZone(20, 5, 46, 14);
    rooms[1].addDarkZone(62, 5, 77, 8);

    // Example: Add a 2-character vertical spring at positions (25,10), (25,11)
    // Make sure there's a wall at (25,9) - the spring will project down
    // rooms[1].addSpring({Point(25, 10), Point(25, 11)}, 2);
}

//////////////////////////////////////////        changeRoom          //////////////////////////////////////////

void Game::changeRoom(int newRoomId, bool goingForward)
{

    if (newRoomId < 0 || newRoomId >= TOTAL_ROOMS)
        return;
    if (currentRoomId >= 0)
    {
        rooms[currentRoomId].active = false;
    }

    currentRoomId = newRoomId;
    rooms[newRoomId].active = true;

    Point spawn = goingForward ? rooms[newRoomId].spawnPoint : rooms[newRoomId].spawnPointFromNext;

    player1.setPosition(spawn.x, spawn.y);
    player2.setPosition(spawn.x, spawn.y + 1);

    player1.pos.diff_x = 0;
    player1.pos.diff_y = 0;
    player2.pos.diff_x = 0;
    player2.pos.diff_y = 0;
    player1.atDoor = false;
    player2.atDoor = false;

    // Reset spring states when changing rooms
    player1.inSpringMotion = false;
    player1.springMomentum = 0;
    player1.springFramesRemaining = 0;
    player2.inSpringMotion = false;
    player2.springMomentum = 0;
    player2.springFramesRemaining = 0;

    player1.prevChar = ' ';
    player2.prevChar = ' ';

    clrscr();
    if (getCurrentRoom())
    {
        getCurrentRoom()->draw();
    }
    player1.updateInventoryDisplay();
    player2.updateInventoryDisplay();
}
