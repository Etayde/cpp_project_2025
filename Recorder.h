#pragma once

#include <string>
#include <iostream>
#include <vector>
#include "Constants.h"
#include "Player.h"

using namespace std;

//////////////////////////////////////////     GameEventType & GameEvent     /////////////////////////////////////////////

enum class GameEventType {
    SCREEN_CHANGE,
    LIFE_LOST,
    RIDDLE_ANSWERED,
    QUIT
};

struct GameEvent {
    unsigned long cycle;
    GameEventType type;
    int roomId;

    // For LIFE_LOST
    int playerId;

    // For RIDDLE_ANSWERED
    std::string question;
    int answerGiven;  // 1-4
    bool wasCorrect;

    // Default constructor
    GameEvent() : cycle(0), type(GameEventType::SCREEN_CHANGE), roomId(0),
                  playerId(0), answerGiven(0), wasCorrect(false) {}

    // Constructor for SCREEN_CHANGE
    GameEvent(unsigned long c, int room)
        : cycle(c), type(GameEventType::SCREEN_CHANGE), roomId(room),
          playerId(0), answerGiven(0), wasCorrect(false) {}

    // Constructor for LIFE_LOST
    GameEvent(unsigned long c, int room, int player)
        : cycle(c), type(GameEventType::LIFE_LOST), roomId(room),
          playerId(player), answerGiven(0), wasCorrect(false) {}

    // Constructor for RIDDLE_ANSWERED
    GameEvent(unsigned long c, int room, const std::string& q, int answer, bool correct)
        : cycle(c), type(GameEventType::RIDDLE_ANSWERED), roomId(room),
          playerId(0), question(q), answerGiven(answer), wasCorrect(correct) {}

    // Constructor for QUIT
    GameEvent(unsigned long c, int room, GameEventType quitType)
        : cycle(c), type(quitType), roomId(room),
          playerId(0), answerGiven(0), wasCorrect(false) {}

    void write(std::ostream& out) const;
    bool read(std::istream& in);
};

//////////////////////////////////////////     Action Conversion     /////////////////////////////////////////////

inline string actionToString(Action action);
inline Action stringToAction(const string& str);
struct ActionRecord
{
    unsigned long cycle;
    int playerId;
    Action action;

    int answer;

    ActionRecord() : cycle(0), playerId(0), action(Action::STAY), answer(-1) {}

    ActionRecord(unsigned long c, const PlayerKeyBinding &binding)
        : cycle(c), playerId(binding.playerID),
          action(binding.action), answer(-1) {}
    
    ActionRecord(unsigned long c, int player, int ans)
        : cycle(c), playerId(player), action(Action::ANSWER_RIDDLE), answer(ans) {}
    void write(ostream &output) const;
    bool read(istream &input);
};

class RecordedSteps
{
private:
    vector<ActionRecord> actions;
    size_t currActionIndex;

public:
    RecordedSteps() : currActionIndex(0) {}

    void addAction(const ActionRecord& record) { actions.push_back(record); }

    ErrorCode loadFromFile(const string& filename);

    const ActionRecord* getCurrentAction() const;

    void advanceToNextAction() { if (currActionIndex < actions.size()) currActionIndex++; }

    bool hasMoreActions() const { return currActionIndex < actions.size(); }

    vector<ActionRecord> getActionsForCycle(unsigned long cycle) const;

    ActionRecord getActionAt(size_t index) const { return actions[index]; }

    size_t getCurrIndex() const { return currActionIndex; }

    void setRandomSeed(unsigned int seed) { randomSeed = seed; }
    unsigned int getRandomSeed() const { return randomSeed; }

private:
    unsigned int randomSeed = 0;
};