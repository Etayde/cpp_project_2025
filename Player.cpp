//////////////////////////////////////       INCLUDES & FORWARDS       /////////////////////////////////////////////

#include "Player.h"
#include "Bomb.h"
#include "Game.h"
#include "Renderer.h"

#include "Door.h"
#include "Obstacle.h"
#include "Riddle.h"
#include "Room.h"
#include "Spring.h"
#include "SpringLink.h"
#include "Switch.h"

//////////////////////////////////////////     Player Constructors       /////////////////////////////////////////////

Player::Player()
    : inventory(nullptr), playerId(0), sprite(' '), atDoor(false), doorId(-1),
      alive(true), keyCount(0), lives(3), score(0), waitingAtDoor(false),
      requestPause(false), springMomentum(Momentum()), respawnTimer(0)
{
  pos = Point(1, 1, 0, 0, ' ');
}

Player::Player(int id, int startX, int startY, char playerSprite)
    : inventory(nullptr), playerId(id), sprite(playerSprite), atDoor(false),
      doorId(-1), alive(true), keyCount(0), lives(3), score(0),
      waitingAtDoor(false), requestPause(false), springMomentum(Momentum()), respawnTimer(0)
{
  pos = Point(startX, startY, 0, 0, playerSprite);
}

//////////////////////////////////////////      Player Destructor       /////////////////////////////////////////////

Player::~Player() { clearInventory(); }

//////////////////////////////////////////   Player Copy Constructor       /////////////////////////////////////////////

Player::Player(const Player &other)
    : pos(other.pos), inventory(nullptr), playerId(other.playerId),
      sprite(other.sprite), atDoor(other.atDoor), doorId(other.doorId),
      alive(other.alive), keyCount(other.keyCount), lives(other.lives),
      score(other.score), waitingAtDoor(other.waitingAtDoor),
      requestPause(other.requestPause), springMomentum(other.springMomentum), respawnTimer(other.respawnTimer)
{
  copyInventoryFrom(other);
}

//////////////////////////////////////////  Player Assignment Operator       /////////////////////////////////////////////

Player &Player::operator=(const Player &other)
{
  if (this != &other)
  {
    clearInventory();

    pos = other.pos;
    playerId = other.playerId;
    sprite = other.sprite;
    atDoor = other.atDoor;
    doorId = other.doorId;
    alive = other.alive;
    keyCount = other.keyCount;
    lives = other.lives;
    score = other.score;
    waitingAtDoor = other.waitingAtDoor;
    requestPause = other.requestPause;
    springMomentum = other.springMomentum;
    respawnTimer = other.respawnTimer;

    copyInventoryFrom(other);
  }
  return *this;
}

//////////////////////////////////////////       Private Helpers       /////////////////////////////////////////////

void Player::clearInventory()
{
  delete inventory;
  inventory = nullptr;
}

void Player::copyInventoryFrom(const Player &other)
{
  if (other.inventory != nullptr) inventory = other.inventory->clone();
}

//////////////////////////////////////////     Movement Helpers       /////////////////////////////////////////////

bool Player::isStationary() const{ return (pos.getDiffX() == 0 && pos.getDiffY() == 0); }

bool Player::isWithinAbsoluteBounds(int x, int y) const{ return (x >= 0 && x < MAX_X && y >= 1 && y < MAX_Y - 1); }

bool Player::canMoveToBoundaryPosition(int x, int y, Room *room) const{
  bool atBoundaryColumn = (x < 1 || x >= MAX_X - 1);

  if (!atBoundaryColumn) return true;

  GameObject *obj = room->getObjectAt(x, y);
  return (obj != nullptr && obj->getType() == ObjectType::DOOR);
}

void Player::erase(Room *room)
{
  if (room == nullptr)
  {
    Renderer::printAt(pos.getX(), pos.getY(), ' ');
    return;
  }

  ObjectType currentType = room->getObjectTypeAt(pos.getX(), pos.getY());
  GameObject *obj = room->getObjectAt(pos.getX(), pos.getY());

  char restoreChar;
  if (obj != nullptr && obj->getType() == ObjectType::DOOR) restoreChar = obj->getSprite();
  else
  {
    char currentChar = static_cast<char>(currentType);
    restoreChar = (currentChar == ' ' || currentChar == sprite) ? ' ' : currentChar;
  }

  Renderer::printAt(pos.getX(), pos.getY(), restoreChar);
}

//////////////////////////////////////////  Respawn and Death Helpers       /////////////////////////////////////////////

void Player::startRespawn(){ respawnTimer = PlayerConstants::RESPAWN_DURATION_FRAMES; }

void Player::respawn(Room *room){

  erase(room);

  pos.stopMovement();
  springMomentum.resetMomentum();
  if (room)
  {
    if (getInventory() != nullptr) dropItem(room);
    
    Point spawn = room->getSpawnPoint(playerId);
    
    if (playerId == 2) spawn.setY(spawn.getY() + 1);
    pos = spawn;
  }

  draw(room);
  startRespawn();
}

void Player::loseLife(Room *room, Game *game)
{
  decreaseLives();
  respawn(room);
  if (game != nullptr) game->reportLifeLost(playerId);
}

void Player::decreaseLives()
{
  if (lives > 0) lives--;
  
  if (lives == 0) kill();

  return;
}

void Player::fallBack(Room *room)
{
  if (room == nullptr) return;

  erase(room);

  int targetX = pos.getX();
  int targetY = pos.getY();
  Direction dir = getCurrentDirection();
  if (springMomentum.isActive()) springMomentum.resetMomentum();

  switch (dir)
  {
  case Direction::UP:
    targetY += 1;
    break;
  case Direction::DOWN:
    targetY -= 1;
    break;
  case Direction::LEFT:
    targetX += 1;
    break;
  case Direction::RIGHT:
    targetX -= 1;
    break;
  case Direction::STAY:
  case Direction::HORIZONTAL:
  case Direction::VERTICAL:
    break;
  }

  bool spotVacant = room->isVacantSpot(targetX, targetY);

  if (spotVacant) setPosition(targetX, targetY);

  draw(room);

  return;
}

//////////////////////////////////////////           move       /////////////////////////////////////////////

bool Player::move(Room *room, Riddle **activeRiddle, Player **activePlayer,
                  Player *otherPlayer, Game *game)
{
  if (room == nullptr) return false;

  if (isRespawning())
  {
    respawnTimer--;
    draw(room);
    return false;
  }

  erase(room);

  if (!springMomentum.isActive())
  {
    GameObject *currentObj = room->getObjectAt(pos.getX(), pos.getY());
    if (currentObj != nullptr &&
        currentObj->getType() == ObjectType::SPRING_LINK)
    {
      SpringLink *link = static_cast<SpringLink *>(currentObj);
      Spring *spring = link->getParentSpring();

      if (spring != nullptr && spring->isCompressed())
      {
        Direction myDirection = getCurrentDirection();
        Direction springDirection = spring->getCompressionDir();

        if (myDirection != springDirection)
        {
          Spring::InteractionResult result =
              spring->handlePlayerInteraction(link, this, room);
          if (result.launched)
          {
            this->springMomentum = result.momentum;
            this->springMomentum.setActive(true);
          }
          draw(room);
          return true;
        }
      }
    }
  }

  bool success;

  if (!springMomentum.isActive())
  {
    int nextX = pos.getX() + pos.getDiffX();
    int nextY = pos.getY() + pos.getDiffY();

    success = singleStep(nextX, nextY, room, activeRiddle, activePlayer, otherPlayer, game);
  }
  else
    success = moveMultiStep(room, activeRiddle, activePlayer, otherPlayer, game);

  draw(room);

  return success;
}

//////////////////////////////////////////           draw            /////////////////////////////////////////////

void Player::draw(Room *room)
{
  Renderer::gotoxy(pos.getX(), pos.getY());

  if (waitingAtDoor)
  {
    if (doorId != room->getPrevRoomId()) { erase(room); return; }
  }

  if (isRespawning() && respawnTimer % BLINK_RATE != 0) Renderer::print(' ');
  
  else Renderer::print(sprite);
}

//////////////////////////////////////////        pickupItem       /////////////////////////////////////////////

bool Player::pickupItem(GameObject *item)
{
  if (item == nullptr || hasItem()) return false;
  if (!item->isPickable()) return false;

  if (item->getType() == ObjectType::KEY) keyCount++;

  inventory = item->clone();
  item->setActive(false);

  return true;
}

//////////////////////////////////////////         dropItem       /////////////////////////////////////////////

Point Player::dropItem(Room *room)
{
  Point dropPos(-1, -1);

  if (!hasItem() || room == nullptr) return dropPos;

  const int checkOffsets[][2] = {{0, 1}, {0, -1}, {1, 0}, {-1, 0}, {1, 1}, {1, -1}, {-1, 1}, {-1, -1}};

  int dropX = -1, dropY = -1;
  bool found = false;

  for (int i = 0; i < 8 && !found; i++)
  {
    int testX = pos.getX() + checkOffsets[i][0];
    int testY = pos.getY() + checkOffsets[i][1];

    if (testX >= 1 && testX < MAX_X - 1 && testY >= 1 &&
        testY < MAX_Y - 1)
    {
      ObjectType type = room->getObjectTypeAt(testX, testY);
      GameObject *existing = room->getObjectAt(testX, testY);

      if ((type == ObjectType::AIR || static_cast<char>(type) == '.') &&
          existing == nullptr)
      {
        dropX = testX;
        dropY = testY;
        found = true;
      }
    }
  }

  if (!found) return dropPos;

  GameObject *droppedItem = inventory->clone();
  droppedItem->setPosition(dropX, dropY);
  droppedItem->setActive(true);

  if (droppedItem->getType() == ObjectType::BOMB)
  {
    Bomb *bomb = static_cast<Bomb *>(droppedItem);
    bomb->activate(room);
  }

  if (room->addObject(droppedItem))
  {
    dropPos.setX(dropX);
    dropPos.setY(dropY);

    if (inventory->getType() == ObjectType::KEY) keyCount--;

    clearInventory();
    clearInventory();

    Renderer::printAt(dropX, dropY, droppedItem->getSprite());
  }

  else delete droppedItem;

  return dropPos;
}

//////////////////////////////////////////       performAction       /////////////////////////////////////////////

void Player::performAction(Action action, Room *room)
{
  int frames = springMomentum.getLaunchFramesRemaining();

  if (frames > 0)
  {
    Direction inputDir = actionToDirection(action);
    if (canApplyInputDuringLaunch(inputDir)) applyPerpendicularVelocity(inputDir);
    
    return;
  }

  switch (action)
  {
  case Action::MOVE_UP:
    pos.setDirection(Direction::UP);
    break;
  case Action::MOVE_DOWN:
    pos.setDirection(Direction::DOWN);
    break;
  case Action::MOVE_LEFT:
    pos.setDirection(Direction::LEFT);
    break;
  case Action::MOVE_RIGHT:
    pos.setDirection(Direction::RIGHT);
    break;
  case Action::STAY:
    pos.setDirection(Direction::STAY);
    break;
  case Action::DROP_ITEM:
    dropItem(room);
    break;
  default:
    pos.setDirection(Direction::STAY);
    break;
  }
}

//////////////////////////////////////////           useKey       /////////////////////////////////////////////

bool Player::useKey()
{
  if (keyCount > 0)
  {
    keyCount--;

    if (keyCount == 0 && hasKey()) clearInventory();
    return true;
  }
  return false;
}

//////////////////////////////////////////    Movement Direction Helpers       /////////////////////////////////////////////

Direction Player::getCurrentDirection() const
{
  if (springMomentum.isActive()) return springMomentum.getLaunchDir();

  if (pos.getDiffX() == 0 && pos.getDiffY() == 0) return Direction::STAY;
  if (pos.getDiffY() < 0) return Direction::UP;
  if (pos.getDiffY() > 0) return Direction::DOWN;
  if (pos.getDiffX() < 0) return Direction::LEFT;
  if (pos.getDiffX() > 0) return Direction::RIGHT;
  return Direction::STAY;
}

Direction Player::actionToDirection(Action action) const
{
  switch (action)
  {
  case Action::MOVE_UP:
    return Direction::UP;
  case Action::MOVE_DOWN:
    return Direction::DOWN;
  case Action::MOVE_LEFT:
    return Direction::LEFT;
  case Action::MOVE_RIGHT:
    return Direction::RIGHT;
  case Action::STAY:
    return Direction::STAY;
  default:
    return Direction::STAY;
  }
}

//////////////////////////////////////////    checkWallCollision       /////////////////////////////////////////////

bool Player::checkWallCollision(int nextX, int nextY, Room *room)
{
  if (room == nullptr) return false;
  return room->isWallAt(nextX, nextY);
}

//////////////////////////////////////////  checkObjectInteraction       /////////////////////////////////////////////

bool Player::checkObjectInteraction(int nextX, int nextY, Room *room,
                                    Riddle **activeRiddle,
                                    Player **activePlayer, Game *game)
{
  if (room == nullptr) return false;

  GameObject *obj = room->getObjectAt(nextX, nextY);

  if (obj == nullptr || !obj->isActive())
  {
    clearDoorState();
    return false;
  }

  ObjectType objType = obj->getType();

  switch (objType)
  {
  case ObjectType::RIDDLE:
  {
    Riddle *riddle = dynamic_cast<Riddle *>(obj);
    if (riddle != nullptr)
      return handleRiddleInteraction(riddle, nextX, nextY, room, activeRiddle,
                                     activePlayer, game);
    break;
  }

  case ObjectType::SWITCH_OFF:
  case ObjectType::SWITCH_ON:
  {
    Switch *sw = dynamic_cast<Switch *>(obj);
    if (sw != nullptr) return handleSwitchInteraction(sw, room);
    break;
  }

  case ObjectType::SPRING_LINK:
  {
    SpringLink *link = dynamic_cast<SpringLink *>(obj);
    if (link != nullptr) return handleSpringInteraction(link, room);
    break;
  }

  case ObjectType::DOOR:
  {
    Door *door = dynamic_cast<Door *>(obj);
    if (door != nullptr) handleDoorInteraction(door);
    return false;
  }

  case ObjectType::OBSTACLE_BLOCK:
  {
    ObstacleBlock *obstacle = dynamic_cast<ObstacleBlock *>(obj);
    if (obstacle != nullptr) return handleObstacleInteraction(obstacle, room);
    break;
  }

  default:
    clearDoorState();

    if (obj->isBlocking()) return true;

    if (obj->isPickable() && !hasItem()) handlePickableInteraction(obj, nextX, nextY, room);

    break;
  }

  return false;
}

//////////////////////////////////////////   clearDoorState       /////////////////////////////////////////////

void Player::clearDoorState()
{
  atDoor = false;
  doorId = -1;
}

//////////////////////////////////////////   handleRiddleInteraction       /////////////////////////////////////////////

bool Player::handleRiddleInteraction(Riddle *riddle, int nextX, int nextY,
                                     Room *room, Riddle **activeRiddle,
                                     Player **activePlayer, Game *game)
{
  if (activeRiddle != nullptr && activePlayer != nullptr)
  {
    *activeRiddle = riddle;
    *activePlayer = this;
    riddle->setSolvingPlayer(*this);
  }

  RiddleResult result = riddle->enterRiddle(room, this, game);

  if (result == RiddleResult::NO_RIDDLE)
  {
    room->removeObjectAt(nextX, nextY);
    if (activeRiddle != nullptr)
      *activeRiddle = nullptr;
    if (activePlayer != nullptr)
      *activePlayer = nullptr;
    return false;
  }
  else if (result == RiddleResult::SOLVED)
  {
    room->removeObjectAt(nextX, nextY);
    if (activeRiddle != nullptr)
      *activeRiddle = nullptr;
    if (activePlayer != nullptr)
      *activePlayer = nullptr;
    return false;
  }
  else if (result == RiddleResult::ESCAPED)
  {
    requestPause = true;
    return true;
  }
  else
  {
    if (activeRiddle != nullptr)
      *activeRiddle = nullptr;
    if (activePlayer != nullptr)
      *activePlayer = nullptr;
    return true;
  }
}

//////////////////////////////////////////   handleSwitchInteraction       /////////////////////////////////////////////

bool Player::handleSwitchInteraction(Switch *sw, Room *room)
{
  sw->toggle();
  room->setCharAt(sw->getX(), sw->getY(), sw->getSprite());
  Renderer::printAt(sw->getX(), sw->getY(), sw->getSprite());
  room->updatePuzzleState();
  return true;
}

//////////////////////////////////////////   handleSpringInteraction       /////////////////////////////////////////////

bool Player::handleSpringInteraction(SpringLink *link, Room *room)
{
  Spring *spring = link->getParentSpring();
  if (spring == nullptr) return false;

  Spring::InteractionResult result =
      spring->handlePlayerInteraction(link, this, room);

  if (result.launched)
  {
    this->springMomentum = result.momentum;
    this->springMomentum.setActive(true);
  }

  return false;
}

//////////////////////////////////////////   handlePickableInteraction       /////////////////////////////////////////////

bool Player::handlePickableInteraction(GameObject *obj, int nextX, int nextY,
                                       Room *room)
{
  if (obj->getType() == ObjectType::KEY)
    keyCount++;

  inventory = obj->clone();
  obj->setActive(false);
  room->setCharAt(nextX, nextY, ' ');

  return false;
}

//////////////////////////////////////////   handleDoorInteraction       /////////////////////////////////////////////

void Player::handleDoorInteraction(Door *door)
{
  atDoor = true;
  doorId = door->getDoorId();
}

//////////////////////////////////////////   isCellBlocking       /////////////////////////////////////////////

bool Player::isCellBlocking(int x, int y, Room *room) const
{
  if (room == nullptr) return true;

  if (x < 0 || x >= MAX_X || y < 1 || y >= MAX_Y - 1) return true;

  if (room->isWallAt(x, y)) return true;

  GameObject *obj = room->getObjectAt(x, y);
  if (obj != nullptr && obj->isActive() && obj->isBlocking())
    return true;

  return false;
}

//////////////////////////////////////////   canApplyInputDuringLaunch       /////////////////////////////////////////////

bool Player::canApplyInputDuringLaunch(Direction inputDir) const
{
  if (inputDir == Direction::STAY) return false;

  Direction oppositeDir;
  Direction launchDir = springMomentum.getLaunchDir();

  switch (launchDir)
  {
  case Direction::UP:
    oppositeDir = Direction::DOWN;
    break;
  case Direction::DOWN:
    oppositeDir = Direction::UP;
    break;
  case Direction::LEFT:
    oppositeDir = Direction::RIGHT;
    break;
  case Direction::RIGHT:
    oppositeDir = Direction::LEFT;
    break;
  default:
    return false;
  }

  if (inputDir == oppositeDir) return false;

  if (inputDir == launchDir) return false;

  return true;
}

//////////////////////////////////////////   applyPerpendicularVelocity       /////////////////////////////////////////////

void Player::applyPerpendicularVelocity(Direction perpendicularDir)
{
  Direction launchDir = springMomentum.getLaunchDir();

  if (launchDir == Direction::LEFT || launchDir == Direction::RIGHT)
  {
    if (perpendicularDir == Direction::UP) springMomentum.incrementDY(-1);
    else if (perpendicularDir == Direction::DOWN) springMomentum.incrementDY(1);
  }
  else if (launchDir == Direction::UP || launchDir == Direction::DOWN)
  {
    if (perpendicularDir == Direction::LEFT) springMomentum.incrementDX(-1);
    else if (perpendicularDir == Direction::RIGHT) springMomentum.incrementDX(1);
  }
}

//////////////////////////////////////////   calculateNextBresenhamPoint       /////////////////////////////////////////////

void Player::calculateNextBresenhamPoint(int &x, int &y, int &err, int absDX,
                                         int absDY, int sx, int sy) const
{
  int e2 = 2 * err;

  if (e2 > -absDY)
  {
    err -= absDY;
    x += sx;
  }
  if (e2 < absDX)
  {
    err += absDX;
    y += sy;
  }
}

//////////////////////////////////////////   moveMultiStep       /////////////////////////////////////////////

bool Player::moveMultiStep(Room *room, Riddle **activeRiddle,
                           Player **activePlayer, Player *otherPlayer, Game *game)
{
  int dx = springMomentum.getDX();
  int dy = springMomentum.getDY();

  if (dx == 0 && dy == 0)
  {
    springMomentum.resetMomentum();
    return false;
  }

  int targetX = pos.getX() + dx;
  int targetY = pos.getY() + dy;

  // Bresenham's Line Algorithm setup
  int absDX = abs(dx);
  int absDY = abs(dy);
  int sx = (dx > 0) ? 1 : -1;
  int sy = (dy > 0) ? 1 : -1;
  int err = absDX - absDY;

  int currentX = pos.getX();
  int currentY = pos.getY();

  while (currentX != targetX || currentY != targetY)
  {
    int nextX = currentX;
    int nextY = currentY;
    calculateNextBresenhamPoint(nextX, nextY, err, absDX, absDY, sx, sy);

    bool moveSucceeded =
        singleStep(nextX, nextY, room, activeRiddle, activePlayer, otherPlayer, game);

    if (!moveSucceeded) break;

    currentX = nextX;
    currentY = nextY;
  }

  if (springMomentum.getLaunchFramesRemaining() == 0)
  {
    springMomentum.resetMomentum();
    pos.stopMovement();
  }

  else
  {
    int remaining = springMomentum.getLaunchFramesRemaining() - 1;
    springMomentum.setLaunchFramesRemaining(remaining);
  }

  return true;
}

//////////////////////////////////////////   singleStep       /////////////////////////////////////////////

bool Player::singleStep(int nextX, int nextY, Room *room, Riddle **activeRiddle,
                        Player **activePlayer, Player *otherPlayer, Game *game)
{
  if (!isWithinAbsoluteBounds(nextX, nextY)) return false;

  if (!canMoveToBoundaryPosition(nextX, nextY, room)) return false;

  if (checkWallCollision(nextX, nextY, room)) return false;

  if (springMomentum.isActive())
  {
    if (otherPlayer != nullptr && otherPlayer->isAlive() &&
        otherPlayer->getX() == nextX && otherPlayer->getY() == nextY)
    {
      transferMomentumTo(otherPlayer);
      return false;
    }
  }

  if (checkObjectInteraction(nextX, nextY, room, activeRiddle, activePlayer, game)) return false;

  pos.setX(nextX);
  pos.setY(nextY);
  return true;
}

//////////////////////////////////////////   stopAtPosition       /////////////////////////////////////////////

void Player::stopAtPosition(int x, int y)
{
  pos.setX(x);
  pos.setY(y);

  pos.stopMovement();

  springMomentum.setLaunchFramesRemaining(0);
}

//////////////////////////////////////////   transferMomentumTo       /////////////////////////////////////////////

void Player::transferMomentumTo(Player *otherPlayer)
{
  if (otherPlayer == nullptr || !otherPlayer->isAlive()) return;

  otherPlayer->springMomentum = this->springMomentum;

  otherPlayer->setDirection(getCurrentDirection());
  springMomentum.setLaunchFramesRemaining(0);
}

////////////////////////////////////////////   calculateForce       /////////////////////////////////////////////

int Player::calculateForce() const
{
  int dx, dy;
  if (springMomentum.isActive())
  {
    dx = abs(springMomentum.getDX());
    dy = abs(springMomentum.getDY());
  }
  else
  {
    dx = abs(pos.getDiffX());
    dy = abs(pos.getDiffY());
  }
  return dx > dy ? dx : dy;
}

////////////////////////////////////////////   handleObstacleInteraction       /////////////////////////////////////////////

bool Player::handleObstacleInteraction(class ObstacleBlock *block, Room *room)
{
  Obstacle *obstacle = block->getParent();
  if (obstacle == nullptr) return false;

  int force = calculateForce();
  Direction pushDir = getCurrentDirection();

  bool obstacleMoved = obstacle->tryPush(pushDir, force, room, this);

  return !obstacleMoved;
}
