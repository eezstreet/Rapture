#pragma once
#include "QuadTree.h"
#include "Tile.h"
#include "Entity.h"
#include "Seed.h"

#define MAX_MAP_LINKS		8
#define MAX_HANDLE_STRING	64

/////////////////////
//
//  Map
//	Each level is a "map". They
//	can be either randomly generated
//	or entirely preset, based on their
//	framework.
//
/////////////////////

struct Map : public QTreeNode<int> {
	char name[MAX_HANDLE_STRING];
	bool bIsPreset;					// Is this map a preset level? (Y/N)

	int iAct;

	QuadTree<TileNode, signed int> qtTileTree;
	QuadTree<Entity, float> qtEntTree;

	Map(int _x, int _y, int _w, int _h, int act, int depth) :
	qtTileTree(QuadTree<TileNode, int>(_x, _y, _w, _h, 0, depth, NULL)),
	qtEntTree(QuadTree<Entity, float>((float)_x, (float)_y, (float)_w, (float)_h, 0, depth+1, NULL)) {
		x = _x;
		y = _y;
		w = _w;
		h = _h;
		iAct = act;
	}

	// Just to shut the compiler up:
	Map() :
	qtTileTree(QuadTree<TileNode, int>(0, 0, 0, 0, 0, 0, nullptr)),
		qtEntTree(QuadTree<Entity, float>(0, 0, 0, 0, 0, 0, nullptr)) {};

	vector<Entity*> FindEntities(const string& classname);

	char links[MAX_MAP_LINKS][MAX_HANDLE_STRING];

	const void* ptFramework;
};