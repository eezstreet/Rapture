#pragma once
#include "Map.h"

enum {
	ALGO_DRUNKEN_STAGGER,
	ALGO_SATURATION,
	ALGO_MAX
};

struct MazeFramework {
	char tileSet[MAX_HANDLE_STRING];
	char name[MAX_HANDLE_STRING];
	int roomSizeX;
	int roomSizeY;
	int algorithm;
};