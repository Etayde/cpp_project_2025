#include "Spring.h"
#include "Player.h"
#include "Room.h"

//////////////////////////////////////////     Constructor           //////////////////////////////////////////

Spring::Spring(const std::vector<Point>& positions, Direction orient,
               Direction projDir, const Point& anchor)
    : StaticObject(positions.empty() ? Point(0, 0) : positions[0], '#', ObjectType::SPRING),
      orientation(orient), projectionDirection(projDir), wallAnchor(anchor),
      maxLength(positions.size()),
      compressingPlayer1Id(0), compressingPlayer2Id(0),
      player1Compression(0), player2Compression(0),
      launchedPlayer1Id(0), launchedPlayer2Id(0),
      player1LaunchFrames(0), player2LaunchFrames(0),
      player1LaunchSpeed(0), player2LaunchSpeed(0)
{
    for (const Point& p : positions)
        cells.push_back(SpringCell(p));
}

//////////////////////////////////////////        clone              //////////////////////////////////////////

GameObject* Spring::clone() const
{
    return new Spring(*this);
}

//////////////////////////////////////////    occupiesPosition       //////////////////////////////////////////

bool Spring::occupiesPosition(int x, int y) const
{
    for (const SpringCell& cell : cells)
        if (cell.position.x == x && cell.position.y == y)
            return true;
    return false;
}

//////////////////////////////////////////  isPlayerCompressing      //////////////////////////////////////////

bool Spring::isPlayerCompressing(int playerId) const
{
    if (compressingPlayer1Id == playerId) return player1Compression > 0;
    if (compressingPlayer2Id == playerId) return player2Compression > 0;
    return false;
}

//////////////////////////////////////////  getPlayerCompression     //////////////////////////////////////////

int Spring::getPlayerCompression(int playerId) const
{
    if (compressingPlayer1Id == playerId) return player1Compression;
    if (compressingPlayer2Id == playerId) return player2Compression;
    return 0;
}

//////////////////////////////////////////  isPlayerBeingLaunched    //////////////////////////////////////////

bool Spring::isPlayerBeingLaunched(int playerId) const
{
    if (launchedPlayer1Id == playerId) return player1LaunchFrames > 0;
    if (launchedPlayer2Id == playerId) return player2LaunchFrames > 0;
    return false;
}

//////////////////////////////////////////    addCompression         //////////////////////////////////////////

void Spring::addCompression(int playerId)
{
    int totalCompression = getTotalCompression();
    if (totalCompression >= maxLength)
        return;  // Fully compressed

    // Assign player to slot if not already tracking
    if (compressingPlayer1Id == 0)
        compressingPlayer1Id = playerId;
    else if (compressingPlayer2Id == 0 && compressingPlayer1Id != playerId)
        compressingPlayer2Id = playerId;

    // Increment compression
    if (compressingPlayer1Id == playerId)
        player1Compression++;
    else if (compressingPlayer2Id == playerId)
        player2Compression++;

    updateVisual();
    draw();
}

//////////////////////////////////////////     launchPlayer          //////////////////////////////////////////

void Spring::launchPlayer(int playerId)
{
    int compression = getPlayerCompression(playerId);
    if (compression == 0) return;

    // Set launch state for this player
    if (compressingPlayer1Id == playerId)
    {
        launchedPlayer1Id = playerId;
        player1LaunchFrames = compression * compression;  // Duration = compression²
        player1LaunchSpeed = compression;                 // Speed = compression

        // Clear compression
        compressingPlayer1Id = 0;
        player1Compression = 0;
    }
    else if (compressingPlayer2Id == playerId)
    {
        launchedPlayer2Id = playerId;
        player2LaunchFrames = compression * compression;
        player2LaunchSpeed = compression;

        compressingPlayer2Id = 0;
        player2Compression = 0;
    }

    updateVisual();
    draw();
}

//////////////////////////////////////////         reset             //////////////////////////////////////////

void Spring::reset()
{
    compressingPlayer1Id = 0;
    compressingPlayer2Id = 0;
    player1Compression = 0;
    player2Compression = 0;

    launchedPlayer1Id = 0;
    launchedPlayer2Id = 0;
    player1LaunchFrames = 0;
    player2LaunchFrames = 0;
    player1LaunchSpeed = 0;
    player2LaunchSpeed = 0;

    updateVisual();
    draw();
}

//////////////////////////////////////////        update             //////////////////////////////////////////

void Spring::update(Player* player1, Player* player2)
{
    // Update player 1 launch
    if (player1 && launchedPlayer1Id == player1->playerId && player1LaunchFrames > 0)
    {
        updateLaunch(player1);
        player1LaunchFrames--;

        if (player1LaunchFrames == 0)
        {
            // DEBUG: Log spring launch end
            gotoxy(0, 0);
            std::cout << "P1 launch END: dir=" << (int)projectionDirection
                      << " diff_x=" << player1->pos.diff_x
                      << " diff_y=" << player1->pos.diff_y << "    " << std::flush;

            launchedPlayer1Id = 0;
            player1LaunchSpeed = 0;
            // Don't clear velocity - player's input will set it
        }
    }

    // Update player 2 launch
    if (player2 && launchedPlayer2Id == player2->playerId && player2LaunchFrames > 0)
    {
        updateLaunch(player2);
        player2LaunchFrames--;

        if (player2LaunchFrames == 0)
        {
            // DEBUG: Log spring launch end
            gotoxy(0, 0);
            std::cout << "P2 launch END: dir=" << (int)projectionDirection
                      << " diff_x=" << player2->pos.diff_x
                      << " diff_y=" << player2->pos.diff_y << "    " << std::flush;

            launchedPlayer2Id = 0;
            player2LaunchSpeed = 0;
            // Don't clear velocity - player's input will set it
        }
    }
}

//////////////////////////////////////////     updateLaunch          //////////////////////////////////////////

void Spring::updateLaunch(Player* player)
{
    if (!player) return;

    int speed = 0;
    if (launchedPlayer1Id == player->playerId)
        speed = player1LaunchSpeed;
    else if (launchedPlayer2Id == player->playerId)
        speed = player2LaunchSpeed;

    if (speed == 0) return;

    // Set player's velocity to spring direction * speed
    // Player will handle lateral movement themselves
    switch (projectionDirection)
    {
        case Direction::UP:    player->pos.diff_y = -speed; break;
        case Direction::DOWN:  player->pos.diff_y = speed; break;
        case Direction::LEFT:  player->pos.diff_x = -speed; break;
        case Direction::RIGHT: player->pos.diff_x = speed; break;
        default: break;
    }
}

//////////////////////////////////////////     updateVisual          //////////////////////////////////////////

void Spring::updateVisual()
{
    int totalCompression = getTotalCompression();
    int visibleCells = maxLength - totalCompression;

    // Show cells near wall, hide cells at free end
    for (int i = 0; i < maxLength; i++)
    {
        if (projectionDirection == Direction::RIGHT || projectionDirection == Direction::DOWN)
        {
            // Wall at left/top - show first N cells
            cells[i].visible = (i < visibleCells);
        }
        else
        {
            // Wall at right/bottom - show last N cells
            cells[i].visible = (i >= totalCompression);
        }
    }
}

//////////////////////////////////////////         draw              //////////////////////////////////////////

void Spring::draw() const
{
    if (!active) return;

    for (const SpringCell& cell : cells)
    {
        gotoxy(cell.position.x, cell.position.y);
        std::cout << (cell.visible ? sprite : ' ') << std::flush;
    }
}
