#include "Recorder.h"
#include <fstream>

//////////////////////////////////////////    ActionRecord::write    /////////////////////////////////////////////

void ActionRecord::write(ostream &output) const
{
    output << "CYCLE: " << cycle
           << " PLAYER: " << playerId
           << " ACTION: " << actionToString(action) << "\n";
}

//////////////////////////////////////////    ActionRecord::read    /////////////////////////////////////////////

bool ActionRecord::read(istream &input, ofstream &debug)
{
    string dummy;
    string actionStr;

    if (!(input >> dummy >> cycle)) {
        debug << dummy << " " << cycle << "\n";
        return false;
    }

    if (!(input >> dummy >> playerId)) {
        debug << dummy << " " << playerId << "\n";
        return false;
    }

    if (!(input >> dummy >> actionStr)) {
        debug << dummy << " " << actionStr << "\n";
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

    ofstream debug("readDEBUG.txt");
    debug << "Loading actions from file: " << filename << "\n";

    while (file.peek() != EOF) {
        ActionRecord record;
        if (!record.read(file, debug)) {
            debug << "Failed to read action record.\n";
            debug.close();
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

