#pragma once
#include "Map.h"

enum {
	ALGO_DRUNKEN_STAGGER,
	ALGO_SATURATION,
	ALGO_MAX
};

enum RoomType_e {
	ROOM_FILLER,
	ROOM_SPECIAL_0,		// Generic special rooms, these can
	ROOM_SPECIAL_1,		// range from stuff like treasure areas,
	ROOM_SPECIAL_2,		// boss chambers and also rooms which
	ROOM_SPECIAL_3,		// contain quest-specific stuff.
	ROOM_SPECIAL_4,
	ROOM_SPECIAL_5,
	ROOM_SPECIAL_6,
	ROOM_SPECIAL_7,
	ROOM_SHRINE,
	ROOM_ENTRY,
	ROOM_EXIT
};

struct MazeFramework {
	char tileSet[MAX_HANDLE_STRING];
	char name[MAX_HANDLE_STRING];
	int roomSizeX;
	int roomSizeY;
	int algorithm;
};

struct AlwaysRoomPlace {
	RoomType_e type;
};