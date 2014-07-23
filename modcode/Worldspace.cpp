#include "Worldspace.h"
#include "DungeonManager.h"

#define MAX_WORLDSPACE_SIZE	/*32768*/	256
#define WORLDSPACE_LOG2		/*15*/		2

#define TILE_WIDTH		192
#define TILE_HEIGHT		96

int visTouching = -1;

Worldspace::Worldspace() {
	qtMapTree = new QuadTree<Map, int>(0, 0, MAX_WORLDSPACE_SIZE, MAX_WORLDSPACE_SIZE, 0, WORLDSPACE_LOG2, nullptr);
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

void Worldspace::Render(Client* ptClient) {
	struct RenderObject {
		union {
			Entity* entData;
			TileNode* tileData;
		};
		float fDepthScore;
		bool bIsTile;
	};
static vector<RenderObject> sortedObjects;	// Static so that we aren't allocating/freeing memory over and over
	sortedObjects.clear();

	Player* ptPlayer = ptClient->ptPlayer;
	if(ptPlayer == nullptr) {
		return;
	}
	Map* theMap = qtMapTree->PreciseNodeAt(ptPlayer->x, ptPlayer->y);
	if(theMap == nullptr) {
		return;
	}

	// mega ultra hack
	float fBoundsX = Worldspace::ScreenSpaceToWorldPlaceX(-TILE_WIDTH, ptClient->screenWidth + TILE_WIDTH, ptClient->ptPlayer);
	float fBoundsY = Worldspace::ScreenSpaceToWorldPlaceY(-TILE_WIDTH, -TILE_WIDTH, ptClient->ptPlayer);
	float fBoundsW = Worldspace::ScreenSpaceToWorldPlaceX(ptClient->screenWidth + TILE_WIDTH, -TILE_WIDTH, ptClient->ptPlayer) - fBoundsX;
	float fBoundsH = Worldspace::ScreenSpaceToWorldPlaceY(ptClient->screenWidth + TILE_WIDTH, ptClient->screenHeight + TILE_WIDTH, ptClient->ptPlayer) - fBoundsY;
static vector<TileNode*> mapTiles;
static vector<Entity*> mapEnts;
	mapTiles.clear();
	mapEnts.clear();
	theMap->qtTileTree.NodesIn(fBoundsX, fBoundsY, fBoundsW, fBoundsH, mapTiles);
	theMap->qtEntTree.NodesIn(fBoundsX, fBoundsY, fBoundsW, fBoundsH, mapEnts);
	//auto mapTiles = theMap->qtTileTree.NodesIn(fBoundsX, fBoundsY, fBoundsW, fBoundsH);
	//auto mapEnts = theMap->qtEntTree.NodesIn(fBoundsX, fBoundsY, fBoundsW, fBoundsH);
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
	struct VisInfo_t {
		int x, y;
		int whichVis;
	};
static vector<VisInfo_t> vis;
	vis.clear();
	for(auto it = mapTiles.begin(); it != mapTiles.end(); ++it) {
		TileNode* tile = *it;
		RenderObject obj;
		obj.bIsTile = true;
		obj.tileData = tile;
		obj.fDepthScore = 10.0f * (WorldPlaceToScreenSpaceFY(tile->x, tile->y) - TILE_HEIGHT - tile->ptTile->fDepthScoreOffset);
		sortedObjects.push_back(obj);

		if(tile->ptTile->name[0] == 'V' && tile->ptTile->name[1] == 'I' && tile->ptTile->name[2] == 'S') {
			if(tile->ptTile->name[3] >= '0' && tile->ptTile->name[3] <= '7') {
				// A vis tile. Let's do this..
				VisInfo_t visData;
				visData.x = tile->x;
				visData.y = tile->y;
				visData.whichVis = tile->ptTile->name[3] - '0';
				vis.push_back(visData);
			}
		}
	}

	// Now we need to sort the stuff that's being rendered.
	sort(sortedObjects.begin(), sortedObjects.end(), [](RenderObject a, RenderObject b) -> bool {
		if(a.fDepthScore < b.fDepthScore) {
			return true;
		}
		return false;
	});

	visTouching = -1;
	bool bHaveWeRenderedPlayer = false;
	// Lastly we need to render the actual stuff
	for(auto itObj = sortedObjects.begin(); itObj != sortedObjects.end(); ++itObj) {
		RenderObject obj = *itObj;
		if(obj.bIsTile) {
			TileNode* tile = obj.tileData;
			int renderX = WorldPlaceToScreenSpaceIX(tile->x, tile->y) + (int)floor(PlayerOffsetX(ptPlayer));
			int renderY = WorldPlaceToScreenSpaceIY(tile->x, tile->y) + (int)floor(PlayerOffsetY(ptPlayer));
			if(bHaveWeRenderedPlayer) {
				// Does this tile use autotrans?
				if(tile->ptTile->bAutoTrans) {
					int halfWidth = ptClient->screenWidth / 2;
					int halfHeight = ptClient->screenHeight / 2;
					if(halfWidth > renderX + tile->ptTile->iAutoTransX
						&& halfWidth < renderX + tile->ptTile->iAutoTransX + tile->ptTile->iAutoTransW
						&& halfHeight > renderY + tile->ptTile->iAutoTransY
						&& halfHeight < renderY + tile->ptTile->iAutoTransY + tile->ptTile->iAutoTransH) {
							trap->RenderMaterialTrans(tile->ptTile->materialHandle, renderX, renderY);
							continue;
					}
				}
			}
			if(tile->ptTile->bWarpTile) {
				// Special logic for warp tiles
				bool bHaveWeAlreadyRendered = false;
				for(auto it = vis.begin(); it != vis.end(); ++it) {
					if(it->x == tile->x && it->y == tile->y) {
						if(ptClient->cursorX >= renderX + tile->ptTile->ptiWarp->x &&
							ptClient->cursorY >= renderY + tile->ptTile->ptiWarp->y &&
							ptClient->cursorX < renderX + tile->ptTile->ptiWarp->x + tile->ptTile->ptiWarp->w &&
							ptClient->cursorY < renderY + tile->ptTile->ptiWarp->y + tile->ptTile->ptiWarp->h) {
								if(visTouching != it->whichVis) {
									// Tell us to start drawing a label here
									string nextMapStr = ptDungeonManager->FindNextMapStr(ptPlayer->iAct, ptPlayer->playerNum, it->whichVis);
									ptClient->StartLabelDraw(nextMapStr.c_str());
								}
								ptClient->bShouldDrawLabels = true;
								visTouching = it->whichVis;
								trap->RenderMaterial(tile->ptTile->ptiWarp->ptHighMaterial, renderX, renderY);
								bHaveWeAlreadyRendered = true;
								break;
						}
					}
				}
				if(bHaveWeAlreadyRendered) {
					continue;
				}
				trap->RenderMaterial(tile->ptTile->ptiWarp->ptDownMaterial, renderX, renderY);
				continue;
			}
			trap->RenderMaterial(tile->ptTile->materialHandle, renderX, renderY);
		} else {
			// Each entity has its own render function, so call that
			auto ent = mRenderList.find(obj.entData->uuid);
			if(ent != mRenderList.end()) {
				if(ent->second->uuid == ptPlayer->uuid) {
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
	if(!theMap) {
		return;
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
	for(auto itActor = vActorsMoved.begin(); itActor != vActorsMoved.end(); ++itActor) {
		auto ptActor = *itActor;
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
	return nullptr;
}

void Worldspace::ActorMoved(Actor* ptActor) {
	vActorsMoved.push_back(ptActor);
}

float Worldspace::WorldPlaceToScreenSpaceFX(float x, float y) {
	return (TILE_HEIGHT * x) + (TILE_HEIGHT * y);
		
}

float Worldspace::WorldPlaceToScreenSpaceFY(float x, float y) {
	return ((TILE_HEIGHT/2.0f) * y) - ((TILE_HEIGHT/2.0f) * x);
}

int Worldspace::WorldPlaceToScreenSpaceIX(int x, int y) {
	return (TILE_HEIGHT * x) + (TILE_HEIGHT * y);
}

int Worldspace::WorldPlaceToScreenSpaceIY(int x, int y) {
	return ((TILE_HEIGHT/2.0f) * y) - ((TILE_HEIGHT/2.0f) * x);
}

float Worldspace::PlayerOffsetX(Player* ptPlayer) {
	return (thisClient->screenWidth / 2.0f) - (TILE_HEIGHT * ptPlayer->x) - (TILE_HEIGHT * ptPlayer->y);
}

float Worldspace::PlayerOffsetY(Player* ptPlayer) {
	return (thisClient->screenHeight / 2.0f) - ((TILE_HEIGHT/2.0f) * ptPlayer->y) + ((TILE_HEIGHT/2.0f) * ptPlayer->x);
}

float Worldspace::ScreenSpaceToWorldPlaceX(int x, int y, Player* ptPlayer) {
	float plyX = ptPlayer->x;
	return (x/TILE_WIDTH) - (y/TILE_HEIGHT) + plyX - (thisClient->screenWidth/(TILE_WIDTH*2.0f)) + (thisClient->screenHeight/TILE_WIDTH) + 0.5f;
}

float Worldspace::ScreenSpaceToWorldPlaceY(int x, int y, Player* ptPlayer) {
	float plyY = ptPlayer->y;
	return (y/TILE_HEIGHT) + (x/TILE_WIDTH) - (thisClient->screenWidth/(TILE_WIDTH*2.0f)) + plyY - (thisClient->screenHeight/TILE_WIDTH) - 0.5f;
}