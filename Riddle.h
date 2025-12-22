#pragma once

////////////////////////////////////////       INCLUDES & FORWARDS       //////////////////////////////////////////

#include "InteractableObject.h"
#include "Constants.h"

class Player;  // Forward declaration
class Room;    // Forward declaration

//////////////////////////////////////////          Riddle            //////////////////////////////////////////

class Riddle : public InteractableObject
{
private:
    int riddleId;
    int correctAnswer;

public:
    Riddle() : InteractableObject(), riddleId(-1), correctAnswer(-1)
    {
        sprite = '?';
        type = ObjectType::RIDDLE;
    }

    Riddle(const Point &pos, int ridId = 0, int ansId = 0) :
                InteractableObject(pos, '?', ObjectType::RIDDLE),
                riddleId(ridId), correctAnswer(ansId) {}

    GameObject *clone() const override { return new Riddle(*this); }
    const char *getName() const override { return "Riddle"; }
    int getRiddleId() const { return riddleId; }
    int getCorrectAnswer() const { return correctAnswer; }

    bool isBlocking() const override { return false; }
    bool onExplosion() override { return true; }

    RiddleResult enterRiddle(Room *room, Player *triggeringPlayer);
    void playRiddleAnimation() const;
    void displayRiddleQuestion() const;
    int getPlayerAnswer() const;
    void displayFeedback(bool correct) const;
    void playExitAnimation() const;
    bool checkAnswer(int playerAnswer) const { return playerAnswer == correctAnswer; };


};