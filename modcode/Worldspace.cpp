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
	ent->spawn();
}

void Worldspace::DrawBackground() {
	auto tiles = qtTileTree->NodesAt(1, 1);
	for(auto it = tiles.begin(); it != tiles.end(); ++it) {
		TileNode* tile = *it;
		trap->RenderMaterial(tile->ptTile->materialHandle, WorldPlaceToScreenSpaceIX(tile->x, tile->y) + PlayerOffsetX(), WorldPlaceToScreenSpaceIY(tile->x, tile->y) + PlayerOffsetY());
	}
}

void Worldspace::DrawEntities() {
	auto ents = qtEntTree->NodesAt(1, 1);
	for(auto it = ents.begin(); it != ents.end(); ++it) {
		auto spatialEnt = *it;
		auto ent = mRenderList.find(spatialEnt->uuid);
		if(ent != mRenderList.end()) {
			ent->second->think();
		}
	}
}

void Worldspace::UpdateEntities() {
	for(auto it = vActorsMoved.begin(); it != vActorsMoved.end(); ++it) {
		auto ptActor = *it;
		if(ptActor->ptContainingTree == qtEntTree->ContainingTree(ptActor->pX, ptActor->pY)) {
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