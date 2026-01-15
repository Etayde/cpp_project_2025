#include "Recorder.h"
#include <fstream>
#include <sstream>

//////////////////////////////////////////    GameEvent::write    /////////////////////////////////////////////

void GameEvent::write(std::ostream& out) const
{
    switch (type) {
        case GameEventType::SCREEN_CHANGE:
            out << "SCREEN_CHANGE CYCLE: " << cycle << " ROOM: " << roomId << "\n";
            break;
        case GameEventType::LIFE_LOST:
            out << "LIFE_LOST CYCLE: " << cycle << " ROOM: " << roomId
                << " PLAYER: " << playerId << "\n";
            break;
        case GameEventType::RIDDLE_ANSWERED:
            out << "RIDDLE CYCLE: " << cycle << " ROOM: " << roomId
                << " QUESTION: \"" << question << "\""
                << " ANSWER: " << answerGiven
                << " CORRECT: " << (wasCorrect ? "YES" : "NO") << "\n";
            break;
    }
}

//////////////////////////////////////////    GameEvent::read    /////////////////////////////////////////////

bool GameEvent::read(std::istream& in)
{
    std::string eventType;
    std::string dummy;

    if (!(in >> eventType)) {
        return false;
    }

    // Parse CYCLE: <cycle>
    if (!(in >> dummy >> cycle)) {
        return false;
    }

    // Parse ROOM: <roomId>
    if (!(in >> dummy >> roomId)) {
        return false;
    }

    if (eventType == "SCREEN_CHANGE") {
        type = GameEventType::SCREEN_CHANGE;
    }
    else if (eventType == "LIFE_LOST") {
        type = GameEventType::LIFE_LOST;
        // Parse PLAYER: <playerId>
        if (!(in >> dummy >> playerId)) {
            return false;
        }
    }
    else if (eventType == "RIDDLE") {
        type = GameEventType::RIDDLE_ANSWERED;
        // Parse QUESTION: "..."
        if (!(in >> dummy)) {  // "QUESTION:"
            return false;
        }
        // Read quoted question - find opening quote
        char c;
        while (in.get(c) && c != '"') {}
        // Read until closing quote
        question.clear();
        while (in.get(c) && c != '"') {
            question += c;
        }
        // Parse ANSWER: <answerGiven>
        if (!(in >> dummy >> answerGiven)) {
            return false;
        }
        // Parse CORRECT: YES/NO
        std::string correctStr;
        if (!(in >> dummy >> correctStr)) {
            return false;
        }
        wasCorrect = (correctStr == "YES");
    }
    else {
        return false;  // Unknown event type
    }

    return true;
}

//////////////////////////////////////////    ActionRecord::write    /////////////////////////////////////////////

void ActionRecord::write(ostream &output) const
{
    output << "CYCLE: " << cycle
           << " PLAYER: " << playerId
           << " ACTION: " << actionToString(action) << "\n";
}

//////////////////////////////////////////    ActionRecord::read    /////////////////////////////////////////////

bool ActionRecord::read(istream &input)
{
    string dummy;
    string actionStr;

    if (!(input >> dummy >> cycle)) {
        return false;
    }

    if (!(input >> dummy >> playerId)) {
        return false;
    }

    if (!(input >> dummy >> actionStr)) {
        return false;
    }

    action = stringToAction(actionStr);

    return true; 
}

//////////////////////////////////////////    RecordedSteps::loadFromFile    /////////////////////////////////////////////

ErrorCode RecordedSteps::loadFromFile(const string& filename)
{
    ifstream file(filename);
    if (!file.is_open()) {
        return ErrorCode::FILE_NOT_FOUND;
    }

    actions.clear();
    currActionIndex = 0;

    while (file >> ws && file.peek() != EOF) {
        ActionRecord record;
        if (!record.read(file)) {
            file.close();
            return ErrorCode::READ_ERROR;
        }
        actions.push_back(record);
    }

    file.close();
    return ErrorCode::NONE;
}

//////////////////////////////////////////    RecordedSteps::getCurrentAction    /////////////////////////////////////////////

const ActionRecord* RecordedSteps::getCurrentAction() const
{
    if (currActionIndex >= actions.size()) {
        return nullptr;
    }
    return &actions[currActionIndex];
}

//////////////////////////////////////////    RecordedSteps::getActionsForCycle    /////////////////////////////////////////////

vector<ActionRecord> RecordedSteps::getActionsForCycle(unsigned long curr) const
{
    vector<ActionRecord> result;
    for (const ActionRecord& record : actions) {
        if (record.cycle == curr) {
            result.push_back(record);
        }
    }
    return result;
}

//////////////////////////////////////////    RecordedSteps::actionAt    /////////////////////////////////////////////

bool RecordedSteps::actionAt(const unsigned long cycle)
{
    for (const ActionRecord& record : actions) {
        if (record.cycle == cycle) {
            return true;
        }
    }
    return false;
}
