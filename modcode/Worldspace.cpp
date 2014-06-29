#include "g_local.h"
#include "QuadTree.h"

#define MAX_WORLDSPACE_SIZE	/*32768*/	256
#define WORLDSPACE_LOG2		/*15*/		2

Worldspace::Worldspace() {
	qtMapTree = new QuadTree<Map, int>(0, 0, MAX_WORLDSPACE_SIZE, MAX_WORLDSPACE_SIZE, 0, WORLDSPACE_LOG2, NULL);
};

Worldspace::~Worldspace() {
	delete qtMapTree;
};

void Worldspace::InsertInto(Map* theMap) {
	qtMapTree->AddNode(theMap);
};

void Worldspace::SpawnEntity(Entity* ent, bool bShouldRender, bool bShouldThink, bool bShouldCollide) {
	if(bShouldRender) {
		mRenderList[ent->uuid] = ent;
	}
	if(bShouldThink) {
		mThinkList[ent->uuid] = ent;
	}
	if(bShouldCollide) {
		mCollideList[ent->uuid] = ent;
	}
	ent->spawn();
}

void Worldspace::Render() {
	struct RenderObject {
		union {
			Entity* entData;
			TileNode* tileData;
		};
		float fDepthScore;
		bool bIsTile;
	};
	vector<RenderObject> sortedObjects;
	auto maps = qtMapTree->NodesAt(GetFirstPlayer()->x, GetFirstPlayer()->y);
	if(maps.size() <= 0) {
		return;
	}
	// Determine which map we're on
	auto player = GetFirstPlayer();
	Map* theMap = nullptr;
	if(player == nullptr) {
		return;
	}
	for(auto it = maps.begin(); it != maps.end(); ++it) {
		auto thisMap = *it;
		if(player->x >= thisMap->x && player->x < thisMap->x + thisMap->w &&
			player->y >= thisMap->y && player->y < thisMap->y + thisMap->h) {
				theMap = thisMap;
		}
	}
	if(theMap == nullptr) {
		return;
	}
	int screenWidth, screenHeight;

	trap->CvarIntVal("r_width", &screenWidth);
	trap->CvarIntVal("r_height", &screenHeight);

	screenWidth /= 2;
	screenHeight /= 2;

	auto mapTiles = theMap->qtTileTree.NodesAt(player->x, player->y);
	auto mapEnts = theMap->qtEntTree.NodesAt(player->x, player->y);
	// Chuck entities into the list of stuff that needs sorted
	for(auto it = mapEnts.begin(); it != mapEnts.end(); ++it) {
		auto ent = *it;
		auto foundEnt = mRenderList.find(ent->uuid);
		if(foundEnt != mRenderList.end()) {
			RenderObject obj;
			obj.bIsTile = false;
			obj.entData = ent;
			obj.fDepthScore = 10 * WorldPlaceToScreenSpaceFY(foundEnt->second->x, foundEnt->second->y);
			sortedObjects.push_back(obj);
		}
	}

	// Now chuck tiles into the pot too
	for(auto it = mapTiles.begin(); it != mapTiles.end(); ++it) {
		TileNode* tile = *it;
		RenderObject obj;
		obj.bIsTile = true;
		obj.tileData = tile;
		obj.fDepthScore = 10.0f * (WorldPlaceToScreenSpaceFY(tile->x, tile->y) - 96.0f - tile->ptTile->fDepthScoreOffset);
		sortedObjects.push_back(obj);
	}

	// Now we need to sort the stuff that's being rendered.
	sort(sortedObjects.begin(), sortedObjects.end(), [](RenderObject a, RenderObject b) -> bool {
		if(a.fDepthScore < b.fDepthScore) {
			return true;
		}
		return false;
	});

	bool bHaveWeRenderedPlayer = false;
	// Lastly we need to render the actual stuff
	for(auto it = sortedObjects.begin(); it != sortedObjects.end(); ++it) {
		RenderObject obj = *it;
		if(obj.bIsTile) {
			TileNode* tile = obj.tileData;
			int renderX = WorldPlaceToScreenSpaceIX(tile->x, tile->y) + (int)floor(PlayerOffsetX(player));
			int renderY = WorldPlaceToScreenSpaceIY(tile->x, tile->y) + (int)floor(PlayerOffsetY(player));
			if(bHaveWeRenderedPlayer) {
				// Does this tile use autotrans?
				if(tile->ptTile->bAutoTrans) {
					if(screenWidth > renderX + tile->ptTile->iAutoTransX
						&& screenWidth < renderX + tile->ptTile->iAutoTransX + tile->ptTile->iAutoTransW
						&& screenHeight > renderY + tile->ptTile->iAutoTransY
						&& screenHeight < renderY + tile->ptTile->iAutoTransY + tile->ptTile->iAutoTransH) {
							trap->RenderMaterialTrans(tile->ptTile->materialHandle, renderX, renderY);
							continue;
					}
				}
			}
			trap->RenderMaterial(tile->ptTile->materialHandle, renderX, renderY);
		} else {
			// Each entity has its own render function, so call that
			auto ent = mRenderList.find(obj.entData->uuid);
			if(ent != mRenderList.end()) {
				if(ent->second->uuid == player->uuid) {
					bHaveWeRenderedPlayer = true;
				}
				ent->second->render();
			}
		}
	}
}

void Worldspace::Run() {
	// Run entity think functions
	auto player = GetFirstPlayer();
	if(player == nullptr) {
		return;
	}
	auto maps = qtMapTree->NodesAt(player->x, player->y); // FIXME
	if(maps.size() <= 0) {
		return;
	}
	Map* theMap = nullptr;
	for(auto it = maps.begin(); it != maps.end(); ++it) {
		auto thisMap = *it;
		if(player->x >= thisMap->x && player->x < thisMap->x + thisMap->w &&
			player->y >= thisMap->y && player->y < thisMap->y + thisMap->h) {
				theMap = thisMap;
		}
	}
	auto ents = theMap->qtEntTree.NodesAt(player->x, player->y); // FIXME
	for(auto it = ents.begin(); it != ents.end(); ++it) {
		auto spatialEnt = *it;
		auto ent = mThinkList.find(spatialEnt->uuid);
		if(ent != mThinkList.end()) {
			ent->second->think();
		}
	}
	UpdateEntities();
}

void Worldspace::UpdateEntities() {
	// FIXME
	for(auto it = vActorsMoved.begin(); it != vActorsMoved.end(); ++it) {
		auto ptActor = *it;
		auto containingMap = qtMapTree->NodesAt(ptActor->x, ptActor->y);
		Map* theMap = nullptr;
		for(auto it = containingMap.begin(); it != containingMap.end(); ++it) {
			if(ptActor->x >= (*it)->x && ptActor->x < (*it)->x + (*it)->w &&
				ptActor->y >= (*it)->y && ptActor->y < (*it)->y + (*it)->h) {
					theMap = *it;
			}
		}
		if(theMap == nullptr) {
			continue;
		}

		auto containingTree = theMap->qtEntTree.ContainingTree(ptActor->x, ptActor->y);
		if(containingTree != ptActor->ptContainingTree) {
			// The tree that we've physically moved on is no longer the same, so we need to update it
			ptActor->ptContainingTree->RemoveNode(ptActor);
			containingTree->AddNode(ptActor);
			ptActor->ptContainingTree = containingTree;
		}
	}
	vActorsMoved.clear();
}

void Worldspace::AddPlayer(Player* ptPlayer) {
	vPlayers.push_back(ptPlayer);
}

Player* Worldspace::GetFirstPlayer() {
	if(vPlayers.size() > 0) {
		return vPlayers.front();
	}
	return NULL;
}

void Worldspace::ActorMoved(Actor* ptActor) {
	vActorsMoved.push_back(ptActor);
}

float Worldspace::WorldPlaceToScreenSpaceFX(float x, float y) {
	return (96.0f * x) + (96.0f * y);
		
}

float Worldspace::WorldPlaceToScreenSpaceFY(float x, float y) {
	return (48.0f * y) - (48.0f * x);
}

int Worldspace::WorldPlaceToScreenSpaceIX(int x, int y) {
	return (96 * x) + (96 * y);
}

int Worldspace::WorldPlaceToScreenSpaceIY(int x, int y) {
	return (48 * y) - (48 * x);
}

float Worldspace::PlayerOffsetX(Player* ptPlayer) {
	int screenWidth = 0;

	trap->CvarIntVal("r_width", &screenWidth);

	return (screenWidth / 2.0f) - (96.0f * ptPlayer->x) - (96.0f * ptPlayer->y);
}

float Worldspace::PlayerOffsetY(Player* ptPlayer) {
	int screenHeight = 0;

	trap->CvarIntVal("r_height", &screenHeight);

	return (screenHeight / 2.0f) - (48.0f * ptPlayer->y) + (48.0f * ptPlayer->x);
}

float Worldspace::ScreenSpaceToWorldPlaceX(int x, int y, Player* ptPlayer) {
	float plyX = ptPlayer->x;
	int screenWidth, screenHeight;

	trap->CvarIntVal("r_width", &screenWidth);
	trap->CvarIntVal("r_height", &screenHeight);

	return (x/192.0f) - (y/96.0f) + plyX - (screenWidth/384.0f) + (screenHeight/192.0f) + 0.5f;
}

float Worldspace::ScreenSpaceToWorldPlaceY(int x, int y, Player* ptPlayer) {
	float plyY = ptPlayer->y;
	int screenWidth, screenHeight;

	trap->CvarIntVal("r_width", &screenWidth);
	trap->CvarIntVal("r_height", &screenHeight);
	
	return (y/96.0f) + (x/192.0f) - (screenWidth/384.0f) + plyY - (screenHeight/192.0f) - 0.5f;
}