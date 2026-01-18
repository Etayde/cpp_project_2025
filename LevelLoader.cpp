//////////////////////////////////////       INCLUDES & FORWARDS       /////////////////////////////////////////////

#include "LevelLoader.h"
#include <algorithm>
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

//////////////////////////////////////////     discoverLevelFiles     /////////////////////////////////////////////

std::vector<std::string> LevelLoader::discoverLevelFiles()
{
  std::vector<std::string> files;
  
  // Use std::filesystem to iterate over the current directory
  // We use error_code to prevent crashing if the directory doesn't exist (though "." always should)
  std::error_code ec;
  for (const auto& entry : fs::directory_iterator(".", ec))
  {
      if (entry.is_regular_file())
      {
          std::string filename = entry.path().filename().string();
          // Check for .screen extension logic
          if (filename.length() >= 7 && 
              filename.substr(filename.length() - 7) == ".screen")
          {
             files.push_back(filename);
          }
          else if (filename.length() >= 11 &&
                   filename.substr(filename.length() - 11) == ".screen.txt")
          {
            files.push_back(filename);
          }
      }
  }

  std::sort(files.begin(), files.end());
  return files;
}

//////////////////////////////////////////      loadScreenFile       /////////////////////////////////////////////

Screen *LevelLoader::loadScreenFile(const std::string& filename, RoomMetadata &metadata)
{
  std::ifstream file(filename);

  if (!file.is_open())
  {
    return nullptr;
  }

  std::string layout[MAX_Y];
  for (int y = 0; y < MAX_Y; y++)
  {
    if (!std::getline(file, layout[y]))
    {
      layout[y] = std::string(MAX_X, ' ');
    }
    while (layout[y].length() < static_cast<size_t>(MAX_X))
    {
      layout[y] += ' ';
    }
  }

  std::string key;
  while (file >> key)
  {
    if (key == "SPAWN")
    {
      int x, y;
      file >> x >> y;
      if (x < 0 || x >= MAX_X || y < 0 || y >= MAX_Y)
      {
        return nullptr;
      }
      metadata.spawnPoints.push_back(Point(x, y));
    }
    else if (key == "SPAWN_PREV")
    {
      int x, y;
      file >> x >> y;
      if (x < 0 || x >= MAX_X || y < 0 || y >= MAX_Y)
      {
        return nullptr;
      }
      metadata.spawnPointsFromNext.push_back(Point(x, y));
    }
    else if (key == "NEXT_ROOM")
    {
      file >> metadata.nextRoomId;
    }
    else if (key == "PREV_ROOM")
    {
      file >> metadata.prevRoomId;
    }
    else if (key == "DOOR")
    {
      int id, keys, switches;
      int targetRoom = -1;
      file >> id >> keys >> switches;
      
      // Check if there is another integer on the same line for targetRoom
      // Logic: Peek next char. If it's a digit or '-', read it.
      while (file.peek() == ' ') file.ignore();
      if (file.peek() != '\n' && file.peek() != EOF)
      {
         char c = file.peek();
         if (isdigit(c) || c == '-')
         {
             file >> targetRoom;
         }
      }

      metadata.doorConfigs.push_back(std::make_tuple(id, keys, switches, targetRoom));
    }
    else if (key == "DARK_ZONE")
    {
      int x1, y1, x2, y2;
      file >> x1 >> y1 >> x2 >> y2;
      if (x1 < 0 || x1 >= MAX_X || y1 < 0 || y1 >= MAX_Y || x2 < 0 || x2 >= MAX_X || y2 < 0 || y2 >= MAX_Y)
      {
        return nullptr;
      }
      metadata.darkZones.push_back(DarkZone(x1, y1, x2, y2));
    }
  }

  file.close();
  return new Screen(layout);
}

//////////////////////////////////////////       loadRiddleFile       /////////////////////////////////////////////

int LevelLoader::loadRiddleFile()
{
  std::ifstream file("riddle.txt");
  if (!file.is_open())
  {
    return 0;
  }

  RiddleDatabase::clearRiddles();

  int count = 0;
  std::string line;

  while (std::getline(file, line))
  {
    if (line.find("---RIDDLE---") != std::string::npos)
    {
      std::string question;
      std::string options[4];
      int answer;

      std::getline(file, question);

      for (int i = 0; i < 4; i++)
      {
        std::getline(file, options[i]);
      }

      file >> answer;
      file.ignore();

      RiddleDatabase::addRiddle(
          RiddleData(count, question, options, answer - 1));
      count++;
    }
  }

  file.close();
  return count;
}
