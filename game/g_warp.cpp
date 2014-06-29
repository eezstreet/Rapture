#include "g_local.h"

static bool bMouseIsOnWarp = false;
static bool bHaveWeNotifiedHUD = false;
static Map* ptMap = NULL;

// Gets run every frame.
static bool WarpThink() {
	float fMouseX = Worldspace::ScreenSpaceToWorldPlaceX(currentMouseX, currentMouseY);
	float fMouseY = Worldspace::ScreenSpaceToWorldPlaceY(currentMouseX, currentMouseY);
	int mouseX = floor(fMouseX);
	int mouseY = floor(fMouseY);

	bMouseIsOnWarp = false;

	auto tiles = world.qtTileTree->NodesAt(mouseX != 0 ? mouseX : 1, mouseY != 0 ? mouseY : 1);
	if(tiles.size() <= 0) {
		// No tiles on our cursor's sector
		return false;
	}

	// First loop - find out which tiles are on our cursor and whether there's a VIS tile there
	vector<TileNode*> vRelevantTiles;
	bool bDoWeHaveVis = false;
	int visNum;
	for(auto it = tiles.begin(); it != tiles.end(); ++it) {
		auto tile = *it;
		if(tile->x == mouseX && tile->y == mouseY) {
			for(int i = 0; i < 8; i++) {
				stringstream ss;
				ss << "VIS" << i;
				if(!stricmp(tile->ptTile->name, ss.str().c_str())) {
					bDoWeHaveVis = true;
					visNum = i;
					break;
				}
			}
			if(tile->ptTile->vismask) {
				vRelevantTiles.push_back(tile);
			}
		}
	}

	if(vRelevantTiles.size() <= 0) {
		// No tiles on our cursor.
		return false;
	} else if(!bDoWeHaveVis) {
		// No vis = no care
		return false;
	}

	int subtileX = floor((fMouseX - mouseX) * 4);
	int subtileY = floor((fMouseY - mouseY) * 4);
	// Figure out whether the subtile we're pointing on is part of the vismask
	for(auto it = vRelevantTiles.begin(); it != vRelevantTiles.end(); ++it) {
		TileNode* node = *it;
		if(node->ptTile->vismask & (1 << ((subtileX*4)+subtileY))) {
			bMouseIsOnWarp = true;
		}
	}

	if(!bMouseIsOnWarp) {
		return false;
	}

	return true;
}

void WarpFrame() {
	if(WarpThink() == false) {
		if(bHaveWeNotifiedHUD) {
			// Tell HUD UI to stop drawing label
			HUD_HideLabels();
		}
	} else {
		if(!bHaveWeNotifiedHUD) {
			// Figure out the map we're looking for
			if(ptMap != NULL) {
				HUD_DrawLabel(ptMap->name);
			}
		}
	}
}