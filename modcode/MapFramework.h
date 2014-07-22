#pragma once
#include "Map.h"
#include "Local.h"
#include "MazeFramework.h"

#define MAX_PARMS	8

/////////////////////
//
//  PresetFramework
//
/////////////////////

struct PresetFramework {
	char preset[MAX_HANDLE_STRING];
};

/////////////////////
//
//  MapFramework
//	Each map has its own MapFramework.
//	These serve as the ground rules for
//	the map to be generated
//
/////////////////////

struct MapFramework {
	char name[MAX_HANDLE_STRING];
	char toName[MAX_HANDLE_STRING];
	bool bIsPreset;
	int nDungeonLevel;
	string sLink[MAX_MAP_LINKS];

	char mazeName[MAX_HANDLE_STRING];
	union {
		PresetFramework dataPreset;
		MazeFramework dataDRLG;
	};
	
	int iWorldspaceX;
	int iWorldspaceY;
	int iSizeX;
	int iSizeY;
	int iAct;

	// These parm things are for DRLG...but maybe they can find a use in preset dungeons too?
	int parm[MAX_PARMS];
	vector<AlwaysRoomPlace> vAlwaysPlace;
};