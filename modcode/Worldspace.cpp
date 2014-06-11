#include "g_local.h"
#include "QuadTree.h"

#define MAX_WORLDSPACE_SIZE	/*32768*/	256
#define WORLDSPACE_LOG2		/*15*/		2

Worldspace::Worldspace() {
	qtTileTree = new QuadTree<TileNode, unsigned int>(0, 0, MAX_WORLDSPACE_SIZE, MAX_WORLDSPACE_SIZE, 0, WORLDSPACE_LOG2, NULL);
	qtEntTree = new QuadTree<SpatialEntity, float>(0, 0, MAX_WORLDSPACE_SIZE, MAX_WORLDSPACE_SIZE, 0, WORLDSPACE_LOG2 + 2, NULL);
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

void Worldspace::SpawnEntity(Entity* ent, bool bShouldRender, bool bShouldThink, bool bShouldCollide) {
	SpatialEntity *s = new SpatialEntity(ent);
	if(bShouldRender) {
		mRenderList[s->uuid] = ent;
	}
	if(bShouldThink) {
		mThinkList[s->uuid] = ent;
	}
	if(bShouldCollide) {
		mCollideList[s->uuid] = ent;
	}
	ent->ptContainingTree = qtEntTree->AddNode(s);
	ent->ptSpatialEntity = s;
	ent->spawn();
}

void Worldspace::Render() {
	struct RenderObject {
		union {
			SpatialEntity* entData;
			TileNode* tileData;
		};
		float fDepthScore;
		bool bIsTile;
	};
	vector<RenderObject> sortedObjects;
	auto ents = qtEntTree->NodesAt(1, 1); // FIXME
	auto tiles = qtTileTree->NodesAt(1, 1);

	// Chuck entities into the list of stuff that needs sorted
	for(auto it = ents.begin(); it != ents.end(); ++it) {
		auto spatialEnt = *it;
		auto ent = mRenderList.find(spatialEnt->uuid);
		if(ent != mRenderList.end()) {
			RenderObject obj;
			obj.bIsTile = false;
			obj.entData = spatialEnt;
			obj.fDepthScore = 10 * WorldPlaceToScreenSpaceFY(ent->second->x, ent->second->y);
			sortedObjects.push_back(obj);
		}
	}

	// Now chuck tiles into the pot too
	for(auto it = tiles.begin(); it != tiles.end(); ++it) {
		TileNode* tile = *it;
		RenderObject obj;
		obj.bIsTile = true;
		obj.tileData = tile;
		obj.fDepthScore = 10 * (WorldPlaceToScreenSpaceFY(tile->x, tile->y) - 96 - tile->ptTile->fDepthScoreOffset);
		sortedObjects.push_back(obj);
	}

	// Now we need to sort the stuff that's being rendered.
	sort(sortedObjects.begin(), sortedObjects.end(), [](RenderObject a, RenderObject b) -> bool {
		if(a.fDepthScore < b.fDepthScore) {
			return true;
		}
		/*if(a.bIsTile) {
			TileNode* aT = a.tileData;
			int aY = WorldPlaceToScreenSpaceIY(aT->x, aT->y);
			if(b.bIsTile) {
				TileNode* bT = b.tileData;
				int bY = WorldPlaceToScreenSpaceIY(bT->x, bT->y);
				if(aY - 96 < bY - 96) {
					return true;
				}
			} else {
				SpatialEntity* bE = b.entData;
				int bY = WorldPlaceToScreenSpaceFY(bE->x, bE->y);
				if(aT->rt > TRT_FLOOR3) {
					return false;
				}
				if(aY - 96 < bY) {
					return true;
				}
			}
		} else {
			SpatialEntity* aE = a.entData;
			int aY = WorldPlaceToScreenSpaceFY(aE->x, aE->y);
			if(b.bIsTile) {
				TileNode* bT = b.tileData;
				int bY = WorldPlaceToScreenSpaceIY(bT->x, bT->y);
				if(bT->rt > TRT_FLOOR3) {
					return false;
				}
				if(aY < bY - 96) {
					return true;
				}
			} else {
				SpatialEntity* bE = b.entData;
				int bY = WorldPlaceToScreenSpaceFY(bE->x, bE->y);
				if(aY < bY) {
					return true;
				}
			}
		}*/
		return false;
	});

	auto player = GetFirstPlayer();
	bool bHaveWeRenderedPlayer = false;
	// Lastly we need to render the actual stuff
	for(auto it = sortedObjects.begin(); it != sortedObjects.end(); ++it) {
		RenderObject obj = *it;
		if(obj.bIsTile) {
			TileNode* tile = obj.tileData;
			int renderX = WorldPlaceToScreenSpaceIX(tile->x, tile->y) + PlayerOffsetX();
			int renderY = WorldPlaceToScreenSpaceIY(tile->x, tile->y) + PlayerOffsetY();
			if(bHaveWeRenderedPlayer) {
				// Does this tile use autotrans?
				if(tile->ptTile->bAutoTrans) {
					if(player->y < tile->y &&
						player->y > tile->y - tile->ptTile->fAutoTransY &&
						player->x > tile->x &&
						player->x < tile->x - tile->ptTile->fAutoTransX) {
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
	auto ents = qtEntTree->NodesAt(1, 1); // FIXME
	for(auto it = ents.begin(); it != ents.end(); ++it) {
		auto spatialEnt = *it;
		auto ent = mThinkList.find(spatialEnt->uuid);
		if(ent != mThinkList.end()) {
			ent->second->think();
		}
	}
}

void Worldspace::UpdateEntities() {
	for(auto it = vActorsMoved.begin(); it != vActorsMoved.end(); ++it) {
		auto ptActor = *it;
		if(ptActor->ptContainingTree == qtEntTree->ContainingTree(ptActor->pX, ptActor->pY)) {
			// Even if it's still in the same tree, we need to update the
			// coordinates in that tree (otherwise depth etc get fucked up)
			ptActor->ptSpatialEntity->x = ptActor->x;
			ptActor->ptSpatialEntity->y = ptActor->y;
			continue;
		}
		ptActor->ptContainingTree->RemoveNode(ptActor);
		qtEntTree->AddNode(ptActor);
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
	return (96.0 * x) + (96.0 * y);
}

float Worldspace::WorldPlaceToScreenSpaceFY(float x, float y) {
	return (48.0 * y) - (48.0 * x);
}

int Worldspace::WorldPlaceToScreenSpaceIX(int x, int y) {
	return (96 * x) + (96 * y);
}

int Worldspace::WorldPlaceToScreenSpaceIY(int x, int y) {
	return (48 * y) - (48 * x);
}

float Worldspace::PlayerOffsetX() {
	int screenWidth = 0;
	Player* ply = GetFirstPlayer();

	trap->CvarIntVal("r_width", &screenWidth);

	return (screenWidth / 2) - (96.0 * ply->x) - (96.0 * ply->y);
}

float Worldspace::PlayerOffsetY() {
	int screenHeight = 0;
	Player* ply = GetFirstPlayer();

	trap->CvarIntVal("r_height", &screenHeight);

	return (screenHeight / 2) - (48.0 * ply->y) + (48.0 * ply->x);
}

float Worldspace::ScreenSpaceToWorldPlaceX(int x, int y) {
	Player* ply = world.GetFirstPlayer();
	float plyX = ply->x;
	int screenWidth, screenHeight;

	trap->CvarIntVal("r_width", &screenWidth);
	trap->CvarIntVal("r_height", &screenHeight);

	return (x/192.0f) - (y/96.0f) + plyX - (screenWidth/384.0f) + (screenHeight/192.0f) + 0.5;
}

float Worldspace::ScreenSpaceToWorldPlaceY(int x, int y) {
	Player* ply = world.GetFirstPlayer();
	float plyY = ply->y;
	int screenWidth, screenHeight;

	trap->CvarIntVal("r_width", &screenWidth);
	trap->CvarIntVal("r_height", &screenHeight);
	
	return (y/96.0f) + (x/192.0f) - (screenWidth/384.0f) + plyY - (screenHeight/192.0f) - 0.5;
}