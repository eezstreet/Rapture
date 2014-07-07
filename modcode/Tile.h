#pragma once
#include "Map.h"
#include "QuadTree.h"
#include "Warp.h"

#define MAX_HANDLE_STRING	64

/////////////////////
//
//  Tiles
//
/////////////////////

enum tileRenderType_e {
	TRT_FLOOR1,
	TRT_FLOOR2,
	TRT_FLOOR3,
	TRT_WALL1,
	TRT_WALL2,
	TRT_WALL3,
	TRT_SHADOW,		// FIXME: not needed?
	TRT_SPECIAL
};

struct Tile {
	// Basic info
	char name[MAX_HANDLE_STRING];

	// Subtile flags
	unsigned short lowmask;		// Mask of subtiles which block the player (but can be jumped over)
	unsigned short highmask;	// Mask of subtiles which block the player (and cannot be jumped over)
	unsigned short lightmask;	// Mask of subtiles which block light
	unsigned short vismask;		// Mask of subtiles which block visibility

	// Material
	Material* materialHandle;	// Pointer to material in memory
	bool bResourcesLoaded;		// Have the resources (images/shaders) been loaded?

	// Depth
	bool bAutoTrans;
	int iAutoTransX, iAutoTransY, iAutoTransW, iAutoTransH;
	float fDepthScoreOffset;

	// Warp information
	bool bWarpTile;
	vector<Warp>::iterator ptiWarp;
	char sWarp[MAX_HANDLE_STRING];

	Tile() {
		name[0] = '\0';
		lowmask = highmask = lightmask = vismask = 0;
		fDepthScoreOffset = 0.0f;
		iAutoTransX = iAutoTransY = iAutoTransW = iAutoTransH = 0;
		materialHandle = nullptr;
		bResourcesLoaded = bAutoTrans = bWarpTile = false;
	}
};

class TileNode : public QTreeNode<signed int> {
public:
	Tile* ptTile;
	tileRenderType_e rt;

	TileNode() {
		w = h = 1;
	}
};