#include "Recorder.h"
#include <fstream>

//////////////////////////////////////////    Action Conversion    /////////////////////////////////////////////

inline Action stringToAction(const string& str) {
    if (str == "MOVE_UP")    return Action::MOVE_UP;
    if (str == "MOVE_DOWN")  return Action::MOVE_DOWN;
    if (str == "MOVE_LEFT")  return Action::MOVE_LEFT;
    if (str == "MOVE_RIGHT") return Action::MOVE_RIGHT;
    if (str == "STAY")       return Action::STAY;
    if (str == "DROP_ITEM")  return Action::DROP_ITEM;
    if (str == "ESC")        return Action::ESC;
    if (str == "ANSWER_RIDDLE") return Action::ANSWER_RIDDLE;
    return Action::STAY;
}

inline string actionToString(Action action) {
    switch (action) {
        case Action::MOVE_UP:    return "MOVE_UP";
        case Action::MOVE_DOWN:  return "MOVE_DOWN";
        case Action::MOVE_LEFT:  return "MOVE_LEFT";
        case Action::MOVE_RIGHT: return "MOVE_RIGHT";
        case Action::STAY:       return "STAY";
        case Action::DROP_ITEM:  return "DROP_ITEM";
        case Action::ANSWER_RIDDLE: return "ANSWER_RIDDLE";
        case Action::ESC:        return "ESC";
        default:                 return "UNKNOWN";
    }
}

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
        case GameEventType::QUIT:
            out << "QUIT CYCLE: " << cycle << " ROOM: " << roomId << "\n";
            break;
    }
}

//////////////////////////////////////////    GameEvent::read    /////////////////////////////////////////////

bool GameEvent::read(std::istream& in)
{
    std::string eventType;
    std::string dummy;

    if (!(in >> eventType)) return false;

    if (!(in >> dummy >> cycle)) return false;

    if (!(in >> dummy >> roomId)) return false;

    if (eventType == "SCREEN_CHANGE") type = GameEventType::SCREEN_CHANGE;
    
    else if (eventType == "LIFE_LOST") 
    {
        type = GameEventType::LIFE_LOST;
        if (!(in >> dummy >> playerId)) 
            return false;
    }
    else if (eventType == "RIDDLE") 
    {
        type = GameEventType::RIDDLE_ANSWERED;
        if (!(in >> dummy)) 
            return false;
        char c;
        while (in.get(c) && c != '"') {}
        question.clear();
        while (in.get(c) && c != '"') {
            question += c;
        }
        if (!(in >> dummy >> answerGiven)) 
            return false;
        
        std::string correctStr;
        if (!(in >> dummy >> correctStr)) 
            return false;
        
        wasCorrect = (correctStr == "YES");
    }
    else if (eventType == "QUIT") 
        type = GameEventType::QUIT;
    
    else 
        return false;  // Unknown event type

    return true;
}

//////////////////////////////////////////    ActionRecord::write    /////////////////////////////////////////////

void ActionRecord::write(ostream &output) const
{
    output << "CYCLE: " << cycle
           << " PLAYER: " << playerId
           << " ACTION: " << actionToString(action);
    
    if (action == Action::ANSWER_RIDDLE) output << " ANSWER: " << answer;
    
    output << "\n";
}

//////////////////////////////////////////    ActionRecord::read    /////////////////////////////////////////////

bool ActionRecord::read(istream &input)
{
    string dummy;
    string actionStr;

    if (!(input >> dummy >> cycle)) return false;

    if (!(input >> dummy >> playerId)) return false;

    if (!(input >> dummy >> actionStr)) return false;

    action = stringToAction(actionStr);

    if (action == Action::ANSWER_RIDDLE) {
        string answerLabel;
        if (!(input >> answerLabel >> answer)) return false;
    }

    return true; 
}

//////////////////////////////////////////    RecordedSteps::loadFromFile    /////////////////////////////////////////////

// Made with AI
ErrorCode RecordedSteps::loadFromFile(const string& filename)
{
    ifstream file(filename);
    if (!file.is_open()) return ErrorCode::FILE_NOT_FOUND;

    actions.clear();
    currActionIndex = 0;
    randomSeed = 0; // Default

    // Check for RANDOM_SEED line
    if (file.peek() != EOF) {
        string line;
        streampos oldPos = file.tellg();
        file >> line;
        if (line == "RANDOM_SEED:") {
             file >> randomSeed;
        } else {
             // Not a seed line, rewind
             file.seekg(oldPos);
        }
    }

    while (file >> ws && file.peek() != EOF) {
        ActionRecord record;
        if (!record.read(file)) {
            // Check if it's just garbage or end
             // Actually, read handles structure.
             // If read fails mid-stream, it's an error.
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
    if (currActionIndex >= actions.size()) return nullptr;
    return &actions[currActionIndex];
}

//////////////////////////////////////////    RecordedSteps::getActionsForCycle    /////////////////////////////////////////////

vector<ActionRecord> RecordedSteps::getActionsForCycle(unsigned long curr) const
{
    vector<ActionRecord> result;
    for (const ActionRecord& record : actions) {
        if (record.cycle == curr) result.push_back(record);
    }
    return result;
}



