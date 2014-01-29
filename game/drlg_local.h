#pragma once
#include "drlg_shared.h"

struct LevelTypeHeader {
	unsigned char nNumTiles;
	vector<string&> tileSets;
};

struct Level {
	unsigned char sSignature[4]; // Four bytes - 'RLVL'
	unsigned char nLevelType;
	unsigned int mTileMask;
};