//////////////////////////////////////       INCLUDES & FORWARDS       //////////////////////////////////////////

#include "Riddle.h"
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

    // 6. Apply penalty for wrong answer
    if (!correct && triggeringPlayer != nullptr) {
        triggeringPlayer->loseLife();  // Subtract 1 life
    }

    // 7. Show feedback
    displayFeedback(correct);

    // 8. Exit animation
    playExitAnimation();

    // 9. Restore screen (only when riddle is answered, not on ESC)
    if (room != nullptr)
        room->draw();

    // 10. Return result
    if (correct)
        return RiddleResult::SOLVED;

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
    gotoxy(11, 4);
    cout << "/";
    for (int i = 1; i < 56; i++) {
        gotoxy(11 + i, 4);
        cout << "-";
    }
    gotoxy(67, 4);
    cout << "\\";

    // Draw sides and clear interior (rows 5-19)
    for (int row = 5; row < 20; row++) {
        gotoxy(11, row);
        cout << "|";
        // Clear interior with spaces
        for (int col = 12; col < 67; col++) {
            cout << " ";
        }
        gotoxy(67, row);
        cout << "|";
    }

    // Draw bottom border
    gotoxy(11, 20);
    cout << "\\";
    for (int i = 1; i < 56; i++) {
        gotoxy(11 + i, 20);
        cout << "-";
    }
    gotoxy(67, 20);
    cout << "/";

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
    gotoxy(13, 15);
    if (correct)
        cout << "CORRECT! The riddle disappears...";
    else
        cout << "INCORRECT! Try again later.";
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
