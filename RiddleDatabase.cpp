//////////////////////////////////////       INCLUDES & FORWARDS       //////////////////////////////////////////

#include "RiddleDatabase.h"

//////////////////////////////////////////    Static Initialization    //////////////////////////////////////////

std::vector<RiddleData> RiddleDatabase::riddles;
bool RiddleDatabase::isActive = false;

//////////////////////////////////////////          initialize          //////////////////////////////////////////

void RiddleDatabase::initialize() {
    if (isActive) return;

    // Hardcoded riddles - easily replaceable with file loading later

    // Riddle 0
    {
        std::string opts[] = {"A Piano", "A Map", "A Tree", "A Clock"};
        riddles.push_back(RiddleData(0, "What has keys but can't open locks?", opts, 0));
    }

    // Riddle 1
    {
        std::string opts[] = {"A River", "A Clock", "A Machine", "A Person"};
        riddles.push_back(RiddleData(1, "What runs but never walks?", opts, 0));
    }

    // Riddle 2
    {
        std::string opts[] = {"A Towel", "A Sponge", "A Desert", "A Fire"};
        riddles.push_back(RiddleData(2, "What gets wetter as it dries?", opts, 0));
    }

    // Riddle 3
    {
        std::string opts[] = {"An Echo", "A Shadow", "A Memory", "A Dream"};
        riddles.push_back(RiddleData(3, "What can speak without a mouth?", opts, 0));
    }

    // Riddle 4
    {
        std::string opts[] = {"The Future", "Yesterday", "Tomorrow", "A Dream"};
        riddles.push_back(RiddleData(4, "What is always coming but never arrives?", opts, 2));
    }

    isActive = true;
}

//////////////////////////////////////////          getRiddle          //////////////////////////////////////////

const RiddleData* RiddleDatabase::getRiddle(int riddleId) {
    initialize();

    for (const RiddleData& r : riddles) {
        if (r.riddleId == riddleId)
            return &r;
    }

    return nullptr;
}

//////////////////////////////////////////       getTotalRiddles       //////////////////////////////////////////

int RiddleDatabase::getTotalRiddles() {
    initialize();
    return riddles.size();
}
