#pragma once
#include "drlg_shared.h"

struct LevelTypeHeader {
	unsigned char nNumTiles;
	vector<string&> tileSets;
};

struct Level {
	unsigned int nLevelId;
	unsigned char nAct;
	char sLevelType;
	unsigned int mTileMask;

};