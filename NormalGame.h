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
    unsigned int randomSeed = 0;

private:
    void recordAction(const PlayerKeyBinding& binding);
    void writeStepsHeader();
    void toggleColorModeBanner();

protected:
    void reportScreenChange(int roomId) override;
    void reportLifeLost(int playerId) override;
    void onRiddleAttempt(const std::string& question, int answer, bool correct) override;
    void reportQuit() override;
    int getRiddleInput(unsigned long cycle) override;
    void reportRiddleAnswer(int answer) override;

public:
    NormalGame();
    NormalGame(int argc, char* argv[]);
    ~NormalGame() override;

    void run() override;
    void gameLoop() override;
    void handleInput() override;
    void handlePauseInput() override;
    void changeRoom(int newRoomId, bool goingForward) override;

    void enableRecording(const string &filename);
    void disableRecording();

    // Menu overrides
    void handleMainMenuInput() override;
};