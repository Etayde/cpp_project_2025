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
    RIDDLE_ANSWERED
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

    void write(std::ostream& out) const;
    bool read(std::istream& in);
};

//////////////////////////////////////////     Action Conversion     /////////////////////////////////////////////

// Convert Action enum to string
inline string actionToString(Action action) {
    switch (action) {
        case Action::MOVE_UP:    return "MOVE_UP";
        case Action::MOVE_DOWN:  return "MOVE_DOWN";
        case Action::MOVE_LEFT:  return "MOVE_LEFT";
        case Action::MOVE_RIGHT: return "MOVE_RIGHT";
        case Action::STAY:       return "STAY";
        case Action::DROP_ITEM:  return "DROP_ITEM";
        case Action::ESC:        return "ESC";
        default:                 return "UNKNOWN";
    }
}

// Convert string to Action enum (returns Action::STAY on invalid input)
inline Action stringToAction(const string& str) {
    if (str == "MOVE_UP")    return Action::MOVE_UP;
    if (str == "MOVE_DOWN")  return Action::MOVE_DOWN;
    if (str == "MOVE_LEFT")  return Action::MOVE_LEFT;
    if (str == "MOVE_RIGHT") return Action::MOVE_RIGHT;
    if (str == "STAY")       return Action::STAY;
    if (str == "DROP_ITEM")  return Action::DROP_ITEM;
    if (str == "ESC")        return Action::ESC;
    return Action::STAY;
}

struct ActionRecord
{
    unsigned long cycle;
    int playerId;
    Action action;

    // Default constructor
    ActionRecord() : cycle(0), playerId(0), action(Action::STAY) {}

    // Constructor from PlayerKeyBinding
    ActionRecord(unsigned long c, const PlayerKeyBinding &binding)
        : cycle(c), playerId(binding.playerID),
          action(binding.action) {}

    // Serialize to output stream
    // Format: CYCLE: <cycle> PLAYER: <player> ACTION: <action>
    void write(ostream &output) const;

    // Deserialize from input stream (reads one line)
    // Returns true on success, false on error (no exceptions)
    bool read(istream &input);
};

class RecordedSteps
{
private:
    vector<ActionRecord> actions;
    size_t currActionIndex;

public:
    // Constructor
    RecordedSteps() : currActionIndex(0) {}

    // Add action to the vector
    void addAction(const ActionRecord& record) { actions.push_back(record); }

    // Load actions from file (calls read() for each line)
    ErrorCode loadFromFile(const string& filename);

    // Get current action (returns nullptr if no more actions)
    const ActionRecord* getCurrentAction() const;

    // Move to next action
    void advanceToNextAction() { if (currActionIndex < actions.size()) currActionIndex++; }

    // Check if there are more actions
    bool hasMoreActions() const { return currActionIndex < actions.size(); }

    // Get all actions for a specific cycle number
    vector<ActionRecord> getActionsForCycle(unsigned long cycle) const;

    bool actionAt(const unsigned long cycle);

    ActionRecord getActionAt(size_t index) const { return actions[index]; }

    size_t getCurrIndex() const { return currActionIndex; }
    
};