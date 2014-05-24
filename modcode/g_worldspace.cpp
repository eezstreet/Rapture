#include "g_local.h"
#include "QuadTree.h"

#define MAX_WORLDSPACE_SIZE	/*32768*/	256
#define WORLDSPACE_LOG2		/*15*/		2

Worldspace::Worldspace() {
	qtTileTree = new CTileTree(0, 0, MAX_WORLDSPACE_SIZE, MAX_WORLDSPACE_SIZE, 0, WORLDSPACE_LOG2);
	qtEntTree = new CEntTree(0, 0, MAX_WORLDSPACE_SIZE, MAX_WORLDSPACE_SIZE, 0, WORLDSPACE_LOG2 + 2);
};

Worldspace::~Worldspace() {
	delete qtTileTree;
	delete qtEntTree;
};

void Worldspace::InsertInto(Map* theMap) {
	for(auto it = theMap->tiles.begin(); it != theMap->tiles.end(); ++it) {
		qtTileTree->AddNode(&(*it));
	}
	for(auto it = theMap->ents.begin(); it != theMap->ents.end(); ++it) {
		qtEntTree->AddNode(&(*it));
	}
};