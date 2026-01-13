#pragma once

#include "Game.h"
#include "Recorder.h"
#include <fstream>

using namespace std;

class NormalGame : public Game
{
    bool isRecording;
    ofstream recordFile;

public:
    NormalGame();
    NormalGame(int argc, char* argv[]);  // New: parses args and initializes
    ~NormalGame() override;
    void handleInput() override;
    void enableRecording(const string &filename);
    void disableRecording();

private:
    void recordAction(const PlayerKeyBinding& binding);
};