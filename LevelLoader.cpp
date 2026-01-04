//////////////////////////////////////       INCLUDES & FORWARDS
/////////////////////////////////////////////

#include "LevelLoader.h"

//////////////////////////////////////////     getScreenFilename
/////////////////////////////////////////////

std::string LevelLoader::getScreenFilename(int number) {
  std::string filename = "adv-world";
  filename += static_cast<char>('0' + (number / 10)); // tens digit
  filename += static_cast<char>('0' + (number % 10)); // ones digit
  filename += ".screen.txt";
  return filename;
}

//////////////////////////////////////////      loadScreenFile
/////////////////////////////////////////////

Screen *LevelLoader::loadScreenFile(int fileNumber, RoomMetadata &metadata) {
  std::string filename = getScreenFilename(fileNumber);
  std::ifstream file(filename);

  if (!file.is_open()) {
    return nullptr; // File doesn't exist
  }

  // Read 25 lines for layout
  std::string layout[MAX_Y];
  for (int y = 0; y < MAX_Y; y++) {
    if (!std::getline(file, layout[y])) {
      layout[y] = std::string(MAX_X, ' ');
    }
    // Ensure exactly 80 characters per line
    while (layout[y].length() < static_cast<size_t>(MAX_X)) {
      layout[y] += ' ';
    }
  }

  // Parse metadata section using >> operator
  std::string key;
  while (file >> key) {
    if (key == "SPAWN") {
      int x, y;
      file >> x >> y;
      metadata.spawnPoint = Point(x, y);
    } else if (key == "SPAWN_PREV") {
      int x, y;
      file >> x >> y;
      metadata.spawnPointFromNext = Point(x, y);
    } else if (key == "NEXT_ROOM") {
      file >> metadata.nextRoomId;
    } else if (key == "PREV_ROOM") {
      file >> metadata.prevRoomId;
    } else if (key == "DOOR") {
      int id, keys, switches;
      file >> id >> keys >> switches;
      metadata.doorConfigs.push_back(std::make_tuple(id, keys, switches));
    } else if (key == "DARK_ZONE") {
      int x1, y1, x2, y2;
      file >> x1 >> y1 >> x2 >> y2;
      metadata.darkZones.push_back(DarkZone(x1, y1, x2, y2));
    }
    // Skip unknown keys (including ---METADATA---)
  }

  file.close();
  return new Screen(layout);
}

//////////////////////////////////////////       loadRiddleFile
/////////////////////////////////////////////

int LevelLoader::loadRiddleFile() {
  std::ifstream file("riddle.txt");
  if (!file.is_open()) {
    return 0; // File not found, will use hardcoded riddles
  }

  RiddleDatabase::clearRiddles();

  int count = 0;
  std::string line;

  while (std::getline(file, line)) {
    if (line.find("---RIDDLE---") != std::string::npos) {
      std::string question;
      std::string options[4];
      int answer;

      // Read question
      std::getline(file, question);

      // Read 4 options
      for (int i = 0; i < 4; i++) {
        std::getline(file, options[i]);
      }

      // Read answer (1-4)
      file >> answer;
      file.ignore(); // Skip newline after answer

      // Add riddle (answer - 1 converts from 1-4 to 0-3 index)
      RiddleDatabase::addRiddle(
          RiddleData(count, question, options, answer - 1));
      count++;
    }
  }

  file.close();
  return count;
}
