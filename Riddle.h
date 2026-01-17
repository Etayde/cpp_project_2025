#pragma once

////////////////////////////////////////       INCLUDES & FORWARDS       //////////////////////////////////////////

#include "InteractableObject.h"
#include "Constants.h"
#include "Player.h"

class Room;
class Game;

//////////////////////////////////////////          Riddle            //////////////////////////////////////////

class Riddle : public InteractableObject
{
    bool firstAttempt;
    char solvingPlayerSprite;
    int solvingPlayerId;
    int riddleId;
    int correctAnswer;

public:

    Riddle() : InteractableObject(), firstAttempt(true), solvingPlayerSprite(' '),
               solvingPlayerId(-1), riddleId(-1), correctAnswer(-1)
    {
        sprite = '?';
        type = ObjectType::RIDDLE;
    }

    Riddle(const Point &pos, int ridId = 0, int ansId = 0) : InteractableObject(pos, '?', ObjectType::RIDDLE),
                                                             firstAttempt(true), solvingPlayerSprite(' '), solvingPlayerId(-1),
                                                             riddleId(ridId), correctAnswer(ansId) {}

    GameObject *clone() const override { return new Riddle(*this); }
    const char *getName() const override { return "Riddle"; }
    int getRiddleId() const { return riddleId; }
    int getCorrectAnswer() const { return correctAnswer; }

    bool isBlocking() const override { return false; }
    bool onExplosion() override { return true; }

    RiddleResult enterRiddle(Room *room, Player *triggeringPlayer, Game *game = nullptr);
    void playRiddleAnimation() const;
    bool displayRiddleQuestion();
    int getPlayerAnswer(const Game* gameContext = nullptr) const;
    void displayFeedback(bool correct) const;
    void playExitAnimation() const;
    bool checkAnswer(int playerAnswer) const;
    void reset()
    {
        solvingPlayerId = -1;
        solvingPlayerSprite = ' ';
    }
    void setSolvingPlayer(Player &player)
    {
        solvingPlayerId = player.getId();
        solvingPlayerSprite = player.getSprite();
    }
    void makeAir()
    {
        sprite = ' ';
        type = ObjectType::AIR;
    }
};