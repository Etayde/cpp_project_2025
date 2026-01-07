//////////////////////////////////////       INCLUDES & FORWARDS       /////////////////////////////////////////////

#include "RiddleDatabase.h"
#include <fstream>

//////////////////////////////////////////    Static Initialization       /////////////////////////////////////////////

std::vector<RiddleData> RiddleDatabase::riddles;
bool RiddleDatabase::isActive = false;

//////////////////////////////////////////          initialize       /////////////////////////////////////////////

void RiddleDatabase::initialize()
{
  if (isActive)
    return;

  std::ifstream file("riddle.txt");
  if (!file.is_open())
  {
    isActive = true;
    return;
  }

  std::string line;
  int riddleId = 0;

  while (std::getline(file, line))
  {
    if (line.find("---RIDDLE---") != std::string::npos)
    {
      std::string question;
      if (!std::getline(file, question))
        break;

      std::string opts[4];
      for (int i = 0; i < 4; i++)
      {
        if (!std::getline(file, opts[i]))
          break;
      }

      std::string answerLine;
      if (!std::getline(file, answerLine))
        break;

      int correctAnswer = std::stoi(answerLine) - 1;

      riddles.push_back(RiddleData(riddleId, question, opts, correctAnswer));
      riddleId++;
    }
  }

  file.close();
  isActive = true;
}

//////////////////////////////////////////          getRiddle       /////////////////////////////////////////////

const RiddleData *RiddleDatabase::getRiddle(int riddleId)
{
  initialize();

  for (const RiddleData &r : riddles)
  {
    if (r.riddleId == riddleId)
      return &r;
  }

  return nullptr;
}

//////////////////////////////////////////       getTotalRiddles       /////////////////////////////////////////////

int RiddleDatabase::getTotalRiddles()
{
  initialize();
  return riddles.size();
}

//////////////////////////////////////////          addRiddle       /////////////////////////////////////////////

void RiddleDatabase::addRiddle(const RiddleData &riddle)
{
  riddles.push_back(riddle);
  isActive = true;
}

//////////////////////////////////////////        clearRiddles       /////////////////////////////////////////////

void RiddleDatabase::clearRiddles()
{
  riddles.clear();
  isActive = false;
}
