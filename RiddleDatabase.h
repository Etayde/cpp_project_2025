#pragma once

//////////////////////////////////////       INCLUDES & FORWARDS       //////////////////////////////////////////

#include <vector>
#include <string>

//////////////////////////////////////////          RiddleData          //////////////////////////////////////////

struct RiddleData {
    int riddleId;
    std::string question;
    std::string options[4];  // Exactly 4 options
    int correctAnswerIndex;  // 0-3 (maps to keys 1-4)

    RiddleData() : riddleId(-1), correctAnswerIndex(-1) {}

    RiddleData(int id, const std::string& q, const std::string opt[4], int correct)
        : riddleId(id), correctAnswerIndex(correct) {
        question = q;
        for (int i = 0; i < 4; i++)
            options[i] = opt[i];
    }
};

//////////////////////////////////////////       RiddleDatabase       //////////////////////////////////////////

class RiddleDatabase {
private:
    static std::vector<RiddleData> riddles;
    static bool isActive;

    static void initialize();  // Hardcoded riddles (easily swappable with file loading)

public:
    static const RiddleData* getRiddle(int riddleId);
    static int getTotalRiddles();
};
