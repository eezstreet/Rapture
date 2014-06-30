#pragma once
#include "Local.h"

/////////////////////
//
//  Warp
//	Warps help to define areas where we should
//	transfer to different levels, ie, levels of
//	a sewer can be transversed through a manhole
//	or something else.
//
/////////////////////

struct Warp {
private:
	bool bLoadedMaterials;
public:
	Material* ptDownMaterial;	// Base material
	Material* ptHighMaterial;	// Material which shows when highlighted
	char sDownMaterial[64];
	char sHighMaterial[64];
	char sName[64];

	int x, y, w, h;
friend class MapLoader;
};