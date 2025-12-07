#pragma once
#include <iostream>
#include "Console.h"
#include "Constants.h"

class Point;

using std::cout, std::endl;

class Screen
{
	const char* screen[MAX_Y];  // מערך של מצביעים למחרוזות קבועות

public:
	// Constructor that takes a screen layout
	Screen(const char* layout[MAX_Y]) {
		for (int i = 0; i < MAX_Y; i++) {
			screen[i] = layout[i];
		}
	}

	// Default constructor - empty screen
	Screen() {
		for (int i = 0; i < MAX_Y; i++) {
			screen[i] = nullptr;
		}
	}

	// Get character at position (x, y)
	char getCharAt(int x, int y) const {
		if (x >= 0 && x < MAX_X && y >= 0 && y < MAX_Y && screen[y] != nullptr) {
			return screen[y][x];
		}
		return 'W';  // Return wall for out of bounds
	}

	// Get character at Point
	char getCharAt(const Point& p) const;

	// Draw entire screen
	void draw() const {
		clrscr();
		gotoxy(0, 0);
		for (int i = 0; i < MAX_Y - 1; ++i) {
			if (screen[i] != nullptr) {
				cout << screen[i] << endl;
			}
		}
		if (screen[MAX_Y - 1] != nullptr) {
			cout << screen[MAX_Y - 1];
		}
		cout.flush();
	}

	// Check if position is a wall
	bool isWall(int x, int y) const {
		return getCharAt(x, y) == 'W';
	}

	// Check if position is a wall (Point version)
	bool isWall(const Point& p) const;

	// Check if there's an object at position
	bool isObject(const Point& p) const;

	// Get object type at position
	ObjectType objectIs(const Point& p) const;
};
