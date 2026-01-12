#pragma once

#include "Game.h"
#include "Recorder.h"
#include <string>
#include <vector>
#include <fstream>

using namespace std;

class LoadedGame : public Game {

    RecordedSteps steps;

public:
    LoadedGame(const string &filename, bool silent = false);
    void handleInput() override;

private:
    ErrorCode loadActions(const string& filename) { return steps.loadFromFile(filename); }
};