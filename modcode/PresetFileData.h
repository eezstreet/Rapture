#pragma once
#include "Map.h"

/////////////////////
//
//  PFD
//	The PFDs get loaded from .bdf files
//	and these in turn get put onto the map
//	in various locations
//
/////////////////////

struct PresetFileData {
	// Header data
	struct PFDHeader {
		char header[8];				// 'DRLG.BDF'
		unsigned short version;		// 1 - current version
		char lookup[MAX_HANDLE_STRING];
		unsigned long reserved1;		// Not used.
		unsigned long reserved2;		// Not used.
		unsigned short reserved3;	// Not used.
		unsigned long sizeX;
		unsigned long sizeY;
		unsigned long numTiles;
		unsigned long numEntities;
	};
	PFDHeader head;

	// Tiles
#pragma pack(1)
	struct LoadedTile {
		char lookup[MAX_HANDLE_STRING];
		signed int x;
		signed int y;
		unsigned char rt;
		signed char layerOffset;
	};
	LoadedTile* tileBlocks;

	// Entities
#pragma pack(1)
	struct LoadedEntity {
		char lookup[MAX_HANDLE_STRING];
		float x;
		float y;
		unsigned int spawnflags;
		unsigned int reserved1;
	};
	LoadedEntity* entities;
};