#include "LoadedGame.h"
#include "Recorder.h"

using namespace std;

LoadedGame::LoadedGame(const string& filename, bool silent) : Game(), steps()
{
    silentMode = silent;  // Set the flag inherited from Game
    Renderer::setSilentMode(silentMode);  // Update renderer mode

    initErrorMessage = loadActions(filename);
    if (initErrorMessage != ErrorCode::NONE) currentState = GameState::error;
}

void LoadedGame::handleInput() 
{
    ActionRecord curr = steps.getActionAt(steps.getCurrIndex());

    if (curr.action == Action::ESC)
    {
        currentState = GameState::paused;
        return;
    }

    Player &player = (curr.playerId == 1) ? player1 : player2;

    steps.advanceToNextAction();

    player.performAction(curr.action, getCurrentRoom());
}
    