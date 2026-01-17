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
    NormalGame(int argc, char* argv[]);
    ~NormalGame() override;
    void handleInput() override;
    void handlePauseInput() override;
    void enableRecording(const string &filename);
    void disableRecording();

protected:
    void recordScreenChange(int roomId) override;
    void recordLifeLost(int playerId) override;
    void recordRiddleAttempt(const std::string& question, int answer, bool correct) override;
    void recordQuit() override;

private:
    void recordAction(const PlayerKeyBinding& binding);
    void recordScreenTransition(int roomId);
    void recordRiddleAnswer(int answer) override;
};