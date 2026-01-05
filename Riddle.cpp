//////////////////////////////////////       INCLUDES & FORWARDS       //////////////////////////////////////////

#include "Riddle.h"
#include "Layouts.h"
#include "RiddleDatabase.h"
#include "Player.h"
#include "Room.h"
#include "Console.h"
using std::cout, std::endl;

//////////////////////////////////////////         enterRiddle         //////////////////////////////////////////

RiddleResult Riddle::enterRiddle(Room *room, Player *triggeringPlayer) {
    // 1. Entry animation (flush input buffer)
    playRiddleAnimation();

    // 2. Display UI
    displayRiddleQuestion();

    // 3. Get player input (blocking)
    int playerAnswer = getPlayerAnswer();

    // 4. Handle ESC - don't restore screen, keep riddle visible
    if (playerAnswer == -1) {
        return RiddleResult::ESCAPED;
    }

    // 5. Check answer
    bool correct = checkAnswer(playerAnswer);

    if (correct && firstAttempt && triggeringPlayer != nullptr) {
        triggeringPlayer->incrementScore(100);
    }

    firstAttempt = false;

    // 6. Apply penalty for wrong answer
    if (!correct && triggeringPlayer != nullptr) {
        triggeringPlayer->decreaseLives();
        triggeringPlayer->fallBack(room);
        triggeringPlayer->startRespawn();
    }

    // 7. Show feedback
    displayFeedback(correct);

    // 8. Exit animation
    playExitAnimation();

    // 9. Restore screen (only when riddle is answered, not on ESC)
    if (room != nullptr)
        room->draw();

    // 10. Return result
    if (correct) return RiddleResult::SOLVED;

    return RiddleResult::FAILED;
}

//////////////////////////////////////////     displayRiddleQuestion    //////////////////////////////////////////

void Riddle::displayRiddleQuestion() const {
    const RiddleData* data = RiddleDatabase::getRiddle(riddleId);
    if (data == nullptr) {
        gotoxy(13, 6);
        cout << "Error: Riddle not found!";
        return;
    }

    // Draw top border
    int startX = 11;
    int startY = 4;
    int endY = 16;
    for (int i = 0; i < endY; i++) {
        gotoxy(startX, startY + i);
        cout << riddlePopupScreen[i];
    }

    // Display question
    gotoxy(13, 6);
    cout << data->question;

    // Display options
    for (int i = 0; i < 4; i++) {
        gotoxy(13, 8 + i);
        cout << (i + 1) << ") " << data->options[i];
    }

    // Display prompt
    gotoxy(13, 13);
    cout << "Choose answer (1-4)";
    cout.flush();
}

//////////////////////////////////////////       getPlayerAnswer        //////////////////////////////////////////

int Riddle::getPlayerAnswer() const {
    showCursor();

    // Flush input buffer first
    while (check_kbhit())
        get_char_nonblocking();

    while (true) {
        int key = get_single_char();  // Blocking

        // ESC
        if (key == 27) {
            hideCursor();
            return -1;
        }

        // Valid answers (1-4)
        if (key >= '1' && key <= '4') {
            hideCursor();
            return key - '1';  // Convert to 0-3
        }

        // Invalid key - ignore and loop
    }
}

//////////////////////////////////////////       displayFeedback        //////////////////////////////////////////

void Riddle::displayFeedback(bool correct) const {

    if (correct && firstAttempt)
    {
        gotoxy(33, 13);
        cout << "CORRECT!";
        gotoxy(18, 15);
        cout << "P" << solvingPlayerId << "(" << solvingPlayerSprite << ") " " solved the riddle on the first try!";
        
        gotoxy(31, 17);
        cout << "+100 Points!";

        gotoxy(27, 19);
        cout << "Riddle disapeared...";
    }
    else if (correct)
    {
        gotoxy(33, 13);
        cout << "CORRECT!";

        gotoxy(18, 15);
        cout << "P" << solvingPlayerId << "(" << solvingPlayerSprite << ") " " solved the riddle!";
        
        gotoxy(27, 17);
        cout << "Riddle disapeared...";
    }
    else 
    {
        gotoxy(24, 17);
        cout << "INCORRECT! Try again later.";
    }
    cout.flush();

    sleep_ms(2000);  // Wait for player to read
}

//////////////////////////////////////////    playRiddleAnimation      //////////////////////////////////////////

void Riddle::playRiddleAnimation() const {
    // Flush input buffer
    while (check_kbhit())
        get_char_nonblocking();
}

//////////////////////////////////////////     playExitAnimation       //////////////////////////////////////////

void Riddle::playExitAnimation() const {
    // No-op for MVP
}
