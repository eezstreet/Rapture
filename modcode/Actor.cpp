#include "Actor.h"
#include "Worldspace.h"
#include "DungeonManager.h"
#include "RVector.h"

void Actor::SetDestinationEnt(Entity* ptEntity) {
	ptDestinationEnt = ptEntity;
}

extern int iMovingToVis;
bool Actor::Move() {
	switch(pathfinding) {
		case PATHFIND_NONE:
		default:
			return Move_NoPathfinding();
		case PATHFIND_ASTAR:
			return Move_AStarPathfinding();
	}
}

bool Actor::Move_NoPathfinding() {
	Worldspace* ptWorld = ptDungeonManager->GetWorld(iAct);
	RVec2<float> nextFramePosition((dir.tComponents[0] * fSpeed * thisClient->GetFrametime())+x, (dir.tComponents[1] * fSpeed * thisClient->GetFrametime())+y);
	if(nextFramePosition.tComponents[0] <= 0 || nextFramePosition.tComponents[1] <= 0) {
		// No moving into negative/zero coords
		return false;
	}

	int bottomedOutX = (int)floor(nextFramePosition.tComponents[0]);
	int bottomedOutY = (int)floor(nextFramePosition.tComponents[1]);
	int bottomedOutSubtileX = (int)floor((nextFramePosition.tComponents[0]-bottomedOutX)*4);
	int bottomedOutSubtileY = (int)floor((nextFramePosition.tComponents[1]-bottomedOutY)*4);

	if(bottomedOutX <= 0 || bottomedOutY <= 0) {
		// can sometimes happen
		return true;
	}
		
	auto nodes = ptDungeonManager->FindProperMap(iAct, bottomedOutX, bottomedOutY)->qtTileTree.NodesAt(bottomedOutX, bottomedOutY);
	if(nodes.size() <= 0) {
		// No nodes in this sector = no movement
		return true;
	}
		
	bool bDoWeHaveNodeHere = false;
	vector<TileNode*> nodesOnThis;
	for(auto it = nodes.begin(); it != nodes.end(); ++it) {
		auto node = (*it);
		if(node->x == bottomedOutX && node->y == bottomedOutY) {
			nodesOnThis.push_back(node);
		}
	}
	if(nodesOnThis.size() <= 0) {
		// No tiles here? NO CAN PASS.
		return true;
	}
	for(auto it = nodesOnThis.begin(); it != nodesOnThis.end(); ++it) {
		auto node = (*it);
		if(node->ptTile->lowmask == 0 && node->ptTile->vismask == 0) {
			// No mask = no worries
			continue;
		}
		if(bIsPlayer) {
			if(node->ptTile->vismask & (1 << (bottomedOutSubtileX * 4)+bottomedOutSubtileY)) {
				Player* ptPlayer = (Player*)this;
				ptPlayer->MoveToNextVis();
				return false;
			}
		}
		if(node->ptTile->lowmask & (1 << (bottomedOutSubtileX * 4)+bottomedOutSubtileY)) {
			// Does this subtile specifically block movement? Don't allow it.
			return true;
		}
	}

	// At this point, we should be allowed to move. We now need to check if there's any entities in our path.
	auto vEntList = ptDungeonManager->FindProperMap(iAct, bottomedOutX, bottomedOutY)->qtEntTree.NodesAt(bottomedOutX, bottomedOutY);
	if(vEntList.size() > 0) {
		for(auto it = vEntList.begin(); it != vEntList.end(); ++it) {
			Entity* ent = *it;
			if(!ent->bShouldWeCollide) {
				continue;
			}
			if(ent->w == 0 || ent->h == 0) {
				continue;
			}
			if(ent->x <= nextFramePosition.tComponents[0] && ent->x + ent->w >= nextFramePosition.tComponents[0] &&
				ent->y <= nextFramePosition.tComponents[1] && ent->y + ent->h >= nextFramePosition.tComponents[1]) {
				if(ptDestinationEnt == ent) {
					ent->interact(this);
				}
				return true;
			}
		}
	}

	x = nextFramePosition.tComponents[0];
	y = nextFramePosition.tComponents[1];
	ptWorld->ActorMoved(this);
	return true;
}

bool Actor::Move_AStarPathfinding() {
	return false;
}