//////////////////////////////////////       INCLUDES & FORWARDS       //////////////////////////////////////////

#include "Riddle.h"
#include "Layouts.h"
#include "RiddleDatabase.h"
#include "Player.h"
#include "Room.h"
#include "Console.h"
using std::cout;

//////////////////////////////////////////         enterRiddle         //////////////////////////////////////////

RiddleResult Riddle::enterRiddle(Room *room, Player *triggeringPlayer)
{
    playRiddleAnimation();

    if (!displayRiddleQuestion())
        return RiddleResult::NO_RIDDLE;

    int playerAnswer = getPlayerAnswer();

    if (playerAnswer == -1)
    {
        return RiddleResult::ESCAPED;
    }

    bool correct = checkAnswer(playerAnswer);

    if (correct && firstAttempt && triggeringPlayer != nullptr)
    {
        triggeringPlayer->incrementScore(100);
    }

    firstAttempt = false;

    if (!correct && triggeringPlayer != nullptr)
    {
        triggeringPlayer->decreaseLives();
        triggeringPlayer->fallBack(room);
        triggeringPlayer->startRespawn();
    }

    displayFeedback(correct);

    playExitAnimation();

    if (room != nullptr)
        room->draw();

    if (correct)
        return RiddleResult::SOLVED;

    return RiddleResult::FAILED;
}

//////////////////////////////////////////     displayRiddleQuestion    //////////////////////////////////////////

bool Riddle::displayRiddleQuestion()
{
    const RiddleData *data = RiddleDatabase::getRiddle(riddleId);
    if (data == nullptr)
    {
        return false;
    }

    int startX = 11;
    int startY = 4;
    int endY = 16;
    for (int i = 0; i < endY; i++)
    {
        gotoxy(startX, startY + i);
        cout << riddlePopupScreen[i];
    }

    gotoxy(13, 6);
    cout << data->question;

    for (int i = 0; i < 4; i++)
    {
        gotoxy(13, 8 + i);
        cout << (i + 1) << ") " << data->options[i];
    }

    gotoxy(13, 13);
    cout << "Choose answer (1-4)";
    cout.flush();
    return true;
}

//////////////////////////////////////////       getPlayerAnswer        //////////////////////////////////////////

int Riddle::getPlayerAnswer() const
{
    showCursor();

    while (check_kbhit())
        get_char_nonblocking();

    while (true)
    {
        int key = get_single_char();

        if (key == 27)
        {
            hideCursor();
            return -1;
        }

        if (key >= '1' && key <= '4')
        {
            hideCursor();
            return key - '1';
        }
    }
}

//////////////////////////////////////////       displayFeedback        //////////////////////////////////////////

void Riddle::displayFeedback(bool correct) const
{

    if (correct && firstAttempt)
    {
        gotoxy(33, 14);
        cout << "CORRECT!";
        gotoxy(18, 16);
        cout << "P" << solvingPlayerId << "(" << solvingPlayerSprite << ") "
                                                                        " solved the riddle on the first try!";

        gotoxy(31, 18);
        cout << "+100 Points!";

        gotoxy(27, 19);
        cout << "Riddle disapeared...";
    }
    else if (correct)
    {
        gotoxy(33, 14);
        cout << "CORRECT!";

        gotoxy(20, 16);
        cout << "P" << solvingPlayerId << " (" << solvingPlayerSprite << ") "
                                                                         " solved the riddle!";

        gotoxy(27, 17);
        cout << "Riddle disapeared...";
    }
    else
    {
        gotoxy(24, 17);
        cout << "INCORRECT! Try again later.";
    }
    cout.flush();

    sleep_ms(2000);
}

//////////////////////////////////////////    playRiddleAnimation      //////////////////////////////////////////

void Riddle::playRiddleAnimation() const
{
    while (check_kbhit())
        get_char_nonblocking();
}

//////////////////////////////////////////     playExitAnimation       //////////////////////////////////////////

void Riddle::playExitAnimation() const
{
}

//////////////////////////////////////////        checkAnswer           //////////////////////////////////////////

bool Riddle::checkAnswer(int playerAnswer) const
{
    const RiddleData *data = RiddleDatabase::getRiddle(riddleId);
    if (data == nullptr)
    {
        return false;
    }
    return playerAnswer == data->correctAnswerIndex;
}
