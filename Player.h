#pragma once

//////////////////////////////////////////       INCLUDES & FORWARDS
/////////////////////////////////////////////

#include "Console.h"
#include "Constants.h"
#include "GameObject.h"
#include "Momentum.h"
#include "Point.h"

class Room;
class Spring;

//////////////////////////////////////////      PlayerKeyBinding
/////////////////////////////////////////////

// Maps keyboard key to player action
struct PlayerKeyBinding {
  char key;
  int playerID;
  Action action;
};

// Key bindings for both players
// Player 1: WAXDS for movement, E to drop
// Player 2: IJKLM for movement, O to drop
static const PlayerKeyBinding keyBindings[] = {
    {'w', 1, Action::MOVE_UP},    {'W', 1, Action::MOVE_UP},
    {'x', 1, Action::MOVE_DOWN},  {'X', 1, Action::MOVE_DOWN},
    {'a', 1, Action::MOVE_LEFT},  {'A', 1, Action::MOVE_LEFT},
    {'d', 1, Action::MOVE_RIGHT}, {'D', 1, Action::MOVE_RIGHT},
    {'s', 1, Action::STAY},       {'S', 1, Action::STAY},
    {'e', 1, Action::DROP_ITEM},  {'E', 1, Action::DROP_ITEM},
    {27, 1, Action::ESC},

    {'i', 2, Action::MOVE_UP},    {'I', 2, Action::MOVE_UP},
    {'m', 2, Action::MOVE_DOWN},  {'M', 2, Action::MOVE_DOWN},
    {'j', 2, Action::MOVE_LEFT},  {'J', 2, Action::MOVE_LEFT},
    {'l', 2, Action::MOVE_RIGHT}, {'L', 2, Action::MOVE_RIGHT},
    {'k', 2, Action::STAY},       {'K', 2, Action::STAY},
    {'o', 2, Action::DROP_ITEM},  {'O', 2, Action::DROP_ITEM},
    {27, 2, Action::ESC}};

static const int NUM_KEY_BINDINGS =
    sizeof(keyBindings) / sizeof(keyBindings[0]);

//////////////////////////////////////////          Player
/////////////////////////////////////////////

// Represents a player character
class Player {
public:
  Point pos;
  GameObject *inventory; // Polymorphic pointer to held item
  int playerId;
  char sprite;
  bool atDoor;
  int doorId;
  bool alive;
  int keyCount; // Number of keys collected
  int lives;    // Player's remaining lives (for riddles and future features)
  int score;
  bool waitingAtDoor; // True if player has crossed through door and is waiting
  bool requestPause;  // Set when ESC pressed during riddle
  Momentum springMomentum;
  int respawnTimer;

public:
  // Constructors & Destructor
  Player();
  Player(int id, int startX, int startY, char playerSprite);
  ~Player();
  Player(const Player &other);
  Player &operator=(const Player &other);

  // Position getters
  int getX() const { return pos.x; }
  int getY() const { return pos.y; }
  Point getPosition() const { return pos; }
  char getSprite() const { return sprite; }

  // State getters
  bool isAtDoor() const { return atDoor; }
  int getDoorId() const { return doorId; }
  int getId() const { return playerId; }
  bool isAlive() const { return alive; }
  bool isDead() const { return !alive; }
  bool hasItem() const { return inventory != nullptr && inventory->isActive(); }
  bool hasTorch() const {
    return inventory != nullptr && inventory->getType() == ObjectType::TORCH;
  }
  bool hasKey() const {
    return inventory != nullptr && inventory->getType() == ObjectType::KEY;
  }
  bool hasBomb() const {
    return inventory != nullptr && inventory->getType() == ObjectType::BOMB;
  }
  int getKeyCount() const { return keyCount; }
  int getLives() const { return lives; }
  int getScore() const { return score; }
  GameObject *getInventory() { return inventory; }
  const GameObject *getInventory() const { return inventory; }
  ObjectType getInventoryType() const {
    return inventory ? inventory->getType() : ObjectType::AIR;
  }
  bool isLaunched() const { return springMomentum.isActive(); }
  bool isRespawning() const { return respawnTimer > 0; }

  // Setters
  void setPosition(int x, int y) {
    pos.x = x;
    pos.y = y;
  }
  void setDirection(Direction dir, int speed = 1) {
    pos.setDirection(dir, speed);
  }
  void addKey() { keyCount++; }
  bool useKey();
  void incrementScore(int points) {
    int newScore = score + points;
    newScore >= 0 ? score = newScore : score = 0;
  }

  // Drawing
  void draw(Room *room = nullptr);
  void erase(Room *room);

  // Life management
  void loseLife(Room *room);
  void setLives(int l) { lives = l; }
  void decreaseLives();
  void respawn(Room *room);
  void startRespawn();
  void kill() { alive = false; }



  // Movement & interaction
  bool move(Room *room, class Riddle **activeRiddle = nullptr,
            Player **activePlayer = nullptr, Player *otherPlayer = nullptr);
  bool pickupItem(GameObject *item);
  Point dropItem(Room *room);
  void performAction(Action action, Room *room = nullptr);
  bool checkWallCollision(int nextX, int nextY, Room *room);
  bool checkObjectInteraction(int nextX, int nextY, Room *room,
                              class Riddle **activeRiddle = nullptr,
                              Player **activePlayer = nullptr);

  // Movement helpers
  Direction getCurrentDirection() const;
  Direction actionToDirection(Action action) const;
  int calculateForce() const;

private:
  void clearInventory();
  void copyInventoryFrom(const Player &other);

  // Interaction helpers
  void clearDoorState();
  bool handleRiddleInteraction(class Riddle *riddle, int nextX, int nextY,
                               Room *room, class Riddle **activeRiddle,
                               Player **activePlayer);
  bool handleSwitchInteraction(class Switch *sw, Room *room);
  bool handleSpringInteraction(class SpringLink *link, Room *room);
  bool handlePickableInteraction(GameObject *obj, int nextX, int nextY,
                                 Room *room);
  void handleDoorInteraction(class Door *door);
  bool handleObstacleInteraction(class ObstacleBlock *block, Room *room);

  // Launch control helpers
  bool canApplyInputDuringLaunch(Direction inputDir) const;
  Direction getLaunchDirection() const;
  void applyPerpendicularVelocity(Direction perpendicularDir);

  // Bresenham algorithm helper
  void calculateNextBresenhamPoint(int &x, int &y, int &err, int absDX,
                                   int absDY, int sx, int sy) const;

  // Multi-step movement helper
  bool moveMultiStep(Room *room, Riddle **activeRiddle, Player **activePlayer,
                     Player *otherPlayer);

  // Single-step movement helper
  bool singleStep(int nextX, int nextY, Room *room, Riddle **activeRiddle,
                  Player **activePlayer, Player *otherPlayer);

  // Collision prediction helpers
  bool isCellBlocking(int x, int y, Room *room) const;
  void stopAtPosition(int x, int y);
  void transferMomentumTo(Player *otherPlayer);

  // Movement validation helpers
  bool isStationary() const;
  bool isWithinAbsoluteBounds(int x, int y) const;
  bool canMoveToBoundaryPosition(int x, int y, Room *room) const;

  // Movement action helpers
  void haltAndRedraw(Room *room);
  void updatePosition(int nextX, int nextY, Room *room);

  // Debug helpers
  void logLaunchState() const;
};