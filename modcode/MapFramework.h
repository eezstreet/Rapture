#pragma once
#include "Map.h"
#include "Local.h"

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

	char entryPreset[MAX_HANDLE_STRING];
	
	int iWorldspaceX;
	int iWorldspaceY;
	int iSizeX;
	int iSizeY;
	int iAct;
};