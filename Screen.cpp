#include "Screen.h"
#include "Point.h"

char Screen::getCharAt(const Point& p) const {
	return getCharAt(p.getX(), p.getY());
}

bool Screen::isWall(const Point& p) const {
	return getCharAt(p) == 'W';
}

bool Screen::isObject(const Point& p) const {
	char c = getCharAt(p);
	if (c != ' ' && c != 'W') {
		return true;
	}
	return false;
}

ObjectType Screen::objectIs(const Point& p) const {
	char ch = getCharAt(p);
	switch (ch)
	{
	case '#':
		return ObjectType::SPRING;
	case '*':
		return ObjectType::OBSTACLE;
	case '!':
		return ObjectType::TORCH;
	case '@':
		return ObjectType::BOMB;
	case 'K':
		return ObjectType::KEY;
	case '\\':
		return ObjectType::SWITCH_OFF;
	case '/':
		return ObjectType::SWITCH_ON;
	case '?':
		return ObjectType::RIDDLE;
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
		return ObjectType::DOOR;
	default:
		return ObjectType::AIR;
	}
}
