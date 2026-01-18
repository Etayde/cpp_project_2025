#pragma once

//////////////////////////////////////////       INCLUDES & FORWARDS       /////////////////////////////////////////////

#include "Point.h"
#include "RiddleDatabase.h"
#include "Room.h"
#include "Screen.h"
#include <fstream>
#include <string>
#include <tuple>
#include <vector>

//////////////////////////////////////////        RoomMetadata       /////////////////////////////////////////////

// Refactored to support better room connections using AI
struct RoomMetadata
{
  std::vector<Point> spawnPoints;
  std::vector<Point> spawnPointsFromNext;
  int nextRoomId;
  int prevRoomId;
  std::vector<std::tuple<int, int, int, int>> doorConfigs; // doorId, keys, switches, targetRoom
  std::vector<DarkZone> darkZones;

  RoomMetadata()
      : nextRoomId(-1), prevRoomId(-1) 
      {
          spawnPoints = {{3, 5}};
          spawnPointsFromNext = {{75, 17}};
      }
};

//////////////////////////////////////////        LevelLoader       /////////////////////////////////////////////

class LevelLoader
{
public:
  static Screen *loadScreenFile(const std::string& filename, RoomMetadata &metadata);

  static int loadRiddleFile();

  static std::vector<std::string> discoverLevelFiles();
};
