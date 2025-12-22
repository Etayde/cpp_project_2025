#include "Spring.h"
#include "Player.h"
#include "Room.h"

//////////////////////////////////////////     startCompression     //////////////////////////////////////////

// Called when a player first steps onto the spring
void Spring::startCompression(Player* player)
{
    if (player == nullptr)
        return;

    // First player compressing this spring
    if (compressingPlayer == nullptr)
    {
        compressingPlayer = player;
        playerCompressionAmount = 1;
        compress(1);
        draw();
    }
    // Second player joining compression
    else if (compressingPlayer != player && secondCompressingPlayer == nullptr)
    {
        secondCompressingPlayer = player;
        secondPlayerCompressionAmount = 1;
        // Update total compression
        compress(playerCompressionAmount + secondPlayerCompressionAmount);
        draw();
    }
}

//////////////////////////////////////////    continueCompression    //////////////////////////////////////////

// Continue compressing the spring - returns true if compression continued
bool Spring::continueCompression(Player* player, Direction moveDir)
{
    if (player == nullptr)
        return false;

    // Check if moving toward wall (opposite to projection direction)
    bool compressingTowardWall = false;
    if (projectionDirection == Direction::UP && moveDir == Direction::DOWN) compressingTowardWall = true;
    if (projectionDirection == Direction::DOWN && moveDir == Direction::UP) compressingTowardWall = true;
    if (projectionDirection == Direction::LEFT && moveDir == Direction::RIGHT) compressingTowardWall = true;
    if (projectionDirection == Direction::RIGHT && moveDir == Direction::LEFT) compressingTowardWall = true;

    if (!compressingTowardWall)
        return false;

    // Determine which player this is and increment their compression
    int* compressionAmount = nullptr;
    if (player == compressingPlayer)
        compressionAmount = &playerCompressionAmount;
    else if (player == secondCompressingPlayer)
        compressionAmount = &secondPlayerCompressionAmount;
    else
        return false; // Player not compressing this spring

    // Check if not fully compressed
    int totalCompression = playerCompressionAmount + secondPlayerCompressionAmount;
    if (totalCompression >= length)
        return false;

    // Increment compression for this player
    (*compressionAmount)++;
    totalCompression = playerCompressionAmount + secondPlayerCompressionAmount;

    // Update visual compression
    compress(totalCompression);
    draw();

    return true;
}

//////////////////////////////////////////     releaseForPlayer     //////////////////////////////////////////

// Release spring and launch player(s)
void Spring::releaseForPlayer(Player* player)
{
    if (player == nullptr)
        return;

    // Check if this player is compressing the spring
    if (player != compressingPlayer && player != secondCompressingPlayer)
        return;

    // Calculate launch parameters for each player
    if (compressingPlayer != nullptr && playerCompressionAmount > 0)
    {
        int compression = playerCompressionAmount;

        // Set player's motion state
        // Speed = compression cells/frame, Duration = compression frames
        // Total distance = compression * compression = compression²
        compressingPlayer->inSpringMotion = true;
        compressingPlayer->springMomentum = compression;  // Cells per frame
        compressingPlayer->springDirection = projectionDirection;
        compressingPlayer->springFramesRemaining = compression;  // Number of frames
    }

    if (secondCompressingPlayer != nullptr && secondPlayerCompressionAmount > 0)
    {
        int compression = secondPlayerCompressionAmount;

        // Set player's motion state
        secondCompressingPlayer->inSpringMotion = true;
        secondCompressingPlayer->springMomentum = compression;  // Cells per frame
        secondCompressingPlayer->springDirection = projectionDirection;
        secondCompressingPlayer->springFramesRemaining = compression;  // Number of frames
    }

    // Reset spring state
    release();
    draw();
    compressingPlayer = nullptr;
    secondCompressingPlayer = nullptr;
    playerCompressionAmount = 0;
    secondPlayerCompressionAmount = 0;
}

//////////////////////////////////////////   updateLaunchedPlayer   //////////////////////////////////////////

// Move player during spring motion - returns true if motion continues
bool Spring::updateLaunchedPlayer(Player* player, Room* room)
{
    // This method is kept for future use, but for now we'll keep the
    // existing implementation in Player::moveWithSpringMomentum()
    // to avoid breaking changes during refactoring
    return false;
}

//////////////////////////////////////////   isCompressedByPlayer   //////////////////////////////////////////

// Check if a specific player is compressing this spring
bool Spring::isCompressedByPlayer(const Player* player) const
{
    return (player != nullptr && (player == compressingPlayer || player == secondCompressingPlayer));
}

//////////////////////////////////////////  isTwoPlayerCompression  //////////////////////////////////////////

// Check if two players are simultaneously compressing
bool Spring::isTwoPlayerCompression() const
{
    return (compressingPlayer != nullptr && secondCompressingPlayer != nullptr);
}
