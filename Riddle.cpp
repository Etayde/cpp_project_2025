//////////////////////////////////////       INCLUDES & FORWARDS       //////////////////////////////////////////

#include "Riddle.h"
#include "Game.h"
#include "Layouts.h"
#include "RiddleDatabase.h"
#include "Player.h"
#include "Room.h"
#include "Console.h"
#include "Renderer.h"
#include <iostream>

using namespace std;

//////////////////////////////////////////         enterRiddle         //////////////////////////////////////////

RiddleResult Riddle::enterRiddle(Room *room, Player *triggeringPlayer, Game *game)
{
    playRiddleAnimation();

    if (!displayRiddleQuestion())
        return RiddleResult::NO_RIDDLE;

    int playerAnswer = getPlayerAnswer(game);

    if (playerAnswer == -1)
    {
        return RiddleResult::ESCAPED;
    }

    bool correct = checkAnswer(playerAnswer);

    if (game != nullptr)
    {
        const RiddleData *data = RiddleDatabase::getRiddle(riddleId);
        std::string questionText = data ? data->question : "";
        game->recordRiddleAttempt(questionText, playerAnswer + 1, correct);
    }

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
        Renderer::printAt(startX, startY + i, riddlePopupScreen[i]);
    }

    Renderer::gotoxy(13, 6);
    Renderer::print(data->question);

    for (int i = 0; i < 4; i++)
    {
        Renderer::gotoxy(13, 8 + i);
        Renderer::print(i + 1);
        Renderer::print(") ");
        Renderer::print(data->options[i]);
    }

    Renderer::gotoxy(13, 13);
    Renderer::print("Choose answer (1-4)");
    Renderer::flush();
    return true;
}

int Riddle::getPlayerAnswer(const Game* gameContext) const
{
    if (gameContext != nullptr)
    {
        int recordedAnswer = const_cast<Game*>(gameContext)->getRecordedRiddleAnswer(gameContext->getCycleCount());
        if (recordedAnswer != -1)
        {
            return recordedAnswer;
        }
    }

    Renderer::showCursor();

    while (check_kbhit())
        get_char_nonblocking();

    while (true)
    {
        int key = get_single_char();

        if (key == 27)
        {
            Renderer::hideCursor();
            return -1;
        }

        if (key >= '1' && key <= '4')
        {
            Renderer::hideCursor();
            int answer = key - '1';
            
            if (gameContext != nullptr)
            {
                 const_cast<Game*>(gameContext)->recordRiddleAnswer(answer);
            }
            
            return answer;
        }
    }
}


//////////////////////////////////////////       displayFeedback        //////////////////////////////////////////

void Riddle::displayFeedback(bool correct) const
{
    if (Renderer::shouldRender())
    {
        if (correct && firstAttempt)
        {
            Renderer::printAt(33, 14, "CORRECT!");

            Renderer::gotoxy(18, 16);
            cout << "P" << solvingPlayerId << "(" << solvingPlayerSprite << ") " << " solved the riddle on the first try!";
            Renderer::printAt(31, 18, "+100 Points!");
            Renderer::printAt(27, 19, "Riddle disapeared...");
        }
        else if (correct)
        {
            Renderer::printAt(33, 14, "CORRECT!");

            Renderer::gotoxy(20, 16);
            cout << "P" << solvingPlayerId << " (" << solvingPlayerSprite << ") " << " solved the riddle!";
            Renderer::printAt(27, 17, "Riddle disapeared...");
        }
        else Renderer::printAt(24, 17, "INCORRECT! Try again later.");
       
        Renderer::flush();

        Renderer::sleep_ms(2000);
    }
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
