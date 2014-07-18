#pragma once
#include "Local.h"
#include "MapFramework.h"
#include "MazeFramework.h"
#include "DungeonManager.h"

enum RoomConnections_e {
	ROOM_N,
	ROOM_W,
	ROOM_S,
	ROOM_E
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

struct Room {
	bool					bAreYouReallyReal;
	int						iConnectionFlags;
	RoomType_e				eType;
	int						x, y;

	Room() { eType = ROOM_FILLER; iConnectionFlags = 0; bAreYouReallyReal = true; }
	Room(RoomType_e eTypex) : eType(eTypex), bAreYouReallyReal(true), iConnectionFlags(0) { }
};

struct RoomGrid {
	int sizeX;
	int sizeY;

	Room* roomArray; // Make sure to free!
};

void RandomizeDungeon(MazeFramework* ptFramework, Map& rtMap, DungeonManager* ptDungeonManager);

typedef bool (*randomizeDungeonFunc)(MazeFramework* ptFramework, Map& rtMap, RoomGrid& roomGrid);

extern randomizeDungeonFunc randomizeDungeonFuncs[ALGO_MAX];