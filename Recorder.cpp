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

    while (file.peek() != EOF) {
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

