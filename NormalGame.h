#pragma once

#include "Game.h"
#include "Recorder.h"
#include <fstream>

using namespace std;

class NormalGame : public Game
{
    bool isRecording;
    ofstream recordFile;
    ofstream resultFile;

public:
    NormalGame();
    NormalGame(int argc, char* argv[]);  // New: parses args and initializes
    ~NormalGame() override;
    void handleInput() override;
    void enableRecording(const string &filename);
    void disableRecording();

protected:
    // Event recording overrides
    void recordScreenChange(int roomId) override;
    void recordLifeLost(int playerId) override;
    void recordRiddleAttempt(const std::string& question, int answer, bool correct) override;

private:
    void recordAction(const PlayerKeyBinding& binding);
    void recordScreenTransition(int roomId);  // For steps file
};