#pragma once

//////////////////////////////////////////       INCLUDES & FORWARDS
/////////////////////////////////////////////

#include "Point.h"
#include "RiddleDatabase.h"
#include "Room.h"
#include "Screen.h"
#include <fstream>
#include <string>
#include <tuple>
#include <vector>

//////////////////////////////////////////        RoomMetadata
/////////////////////////////////////////////

struct RoomMetadata {
  Point spawnPoint;
  Point spawnPointFromNext;
  int nextRoomId;
  int prevRoomId;
  std::vector<std::tuple<int, int, int>> doorConfigs; // doorId, keys, switches
  std::vector<DarkZone> darkZones;

  RoomMetadata()
      : spawnPoint(3, 5), spawnPointFromNext(75, 17), nextRoomId(-1),
        prevRoomId(-1) {}
};

//////////////////////////////////////////        LevelLoader
/////////////////////////////////////////////

class LevelLoader {
public:
  // Load screen file by number (1, 2, 3...) - returns nullptr if file not found
  static Screen *loadScreenFile(int fileNumber, RoomMetadata &metadata);

  // Load riddles from riddle.txt - returns number of riddles loaded
  static int loadRiddleFile();

  // Generate filename: 1 -> "adv-world01.screen"
  static std::string getScreenFilename(int number);
};
