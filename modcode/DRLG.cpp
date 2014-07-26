#include "DRLG.h"
#include "DungeonManager.h"
#include "MapFramework.h"
#include "PresetFileData.h"
#include "Entity.h"
#include "RVector.h"

static Seed drlgSeed;

// These functions just generate the rooms, we need to actually set up connections etc
bool Do_DrunkenStagger(MazeFramework* ptFramework, Map& rtMap, RoomGrid& rtRoomGrid) {
	int maxX = rtMap.w / ptFramework->roomSizeX;
	int maxY = rtMap.h / ptFramework->roomSizeY;
	int startX = drlgSeed.GenerateRandom(0, maxX);
	int startY = drlgSeed.GenerateRandom(0, maxY);
	int i = 0;
	rtRoomGrid.sizeX = maxX;
	rtRoomGrid.sizeY = maxY;
	rtRoomGrid.roomArray = (Room*)trap->Zone_Alloc(maxX * maxY * sizeof(Room), "roomgrid");	// FIXME: magically invent hunk memory
	memset(rtRoomGrid.roomArray, 0, maxX * maxY * sizeof(Room));

	RVec2<int> currentPos(startX, startY);
	Room entry(ROOM_ENTRY);
	entry.x = startX;
	entry.y = startY;
	rtRoomGrid.roomArray[startY * maxX + startX] = entry;

	vector<int> vWalkableDirections;
	const MapFramework* ptMapFramework = (const MapFramework*)rtMap.ptFramework;

	function<void(int, bool)> walk = [&](int max, bool bIgnoreBuiltRooms) -> void {
		i = 0;
		while(i < ptMapFramework->parm[0]) {
			// Pick one direction and go with it
			int numWalkableDirections = 4;
			int xInc = 0;
			int yInc = 0;
			vWalkableDirections.clear();
			vWalkableDirections.push_back(0);
			vWalkableDirections.push_back(1);
			vWalkableDirections.push_back(2);
			vWalkableDirections.push_back(3);
			int direction = -1;

			if(currentPos.tComponents[1] <= 0) {
				VectorErase<int>(vWalkableDirections, 0);
				numWalkableDirections--;
			}
			if(currentPos.tComponents[0] <= 0) {
				VectorErase<int>(vWalkableDirections, 1);
				numWalkableDirections--;
			}
			if(currentPos.tComponents[1] >= maxY-1) {
				VectorErase<int>(vWalkableDirections, 2);
				numWalkableDirections--;
			}
			if(currentPos.tComponents[0] >= maxX-1) {
				VectorErase<int>(vWalkableDirections, 3);
				numWalkableDirections--;
			}
			if(numWalkableDirections <= 0) {
				R_Error("Maze with grid size 0");
				break;
			}

			direction = drlgSeed.GenerateRandom(0, vWalkableDirections.size());
			direction = vWalkableDirections.at(direction);

			// NWSE
			switch(direction) {
				case 0:
					yInc--;
					break;
				case 1:
					xInc--;
					break;
				case 2:
					yInc++;
					break;
				case 3:
					xInc++;
					break;
			}

			currentPos.tComponents[0] += xInc;
			currentPos.tComponents[1] += yInc;
			Room& thisRoom = rtRoomGrid.roomArray[currentPos.tComponents[1] * maxX + currentPos.tComponents[0]];
			if(thisRoom.bAreYouReallyReal == false && bIgnoreBuiltRooms == false) {
				i++;
			} else if(bIgnoreBuiltRooms == true) {
				i++;
			}
			if(thisRoom.bAreYouReallyReal == false) {
				// No room here = increment and add a new room
				// TODO: eliminate code reuse
				thisRoom.bAreYouReallyReal = true;
				thisRoom.eType = ROOM_FILLER;
				thisRoom.x = currentPos.tComponents[0];
				thisRoom.y = currentPos.tComponents[1];
			}
		}
		vWalkableDirections.clear();
	};

	walk(ptMapFramework->parm[0], false);
	for(auto it = ptMapFramework->vAlwaysPlace.begin(); it != ptMapFramework->vAlwaysPlace.end(); ++it) {
		currentPos.tComponents[0] = startX;
		currentPos.tComponents[1] = startY;
		walk(ptMapFramework->parm[1], true);
		walk(ptMapFramework->parm[2], false);
		// Now that we've figured out where we need to end up...let's build the room that we need.
		// TODO: eliminate code reuse
		int numWalkableDirections = 4;
		int xInc = 0;
		int yInc = 0;
		int direction = -1;
		vWalkableDirections.clear();
		vWalkableDirections.push_back(0);
		vWalkableDirections.push_back(1);
		vWalkableDirections.push_back(2);
		vWalkableDirections.push_back(3);
		if(currentPos.tComponents[1] <= 0) {
			VectorErase<int>(vWalkableDirections, 0);
			numWalkableDirections--;
		}
		if(currentPos.tComponents[0] <= 0) {
			VectorErase<int>(vWalkableDirections, 1);
			numWalkableDirections--;
		}
		if(currentPos.tComponents[1] >= maxY-1) {
			VectorErase<int>(vWalkableDirections, 2);
			numWalkableDirections--;
		}
		if(currentPos.tComponents[0] >= maxX-1) {
			VectorErase<int>(vWalkableDirections, 3);
			numWalkableDirections--;
		}
		if(numWalkableDirections <= 0) {
			R_Error("Maze with grid size 0");
			break;
		}

		direction = drlgSeed.GenerateRandom(0, vWalkableDirections.size());
		direction = vWalkableDirections.at(direction);

		// NWSE
		switch(direction) {
			case 0:
				yInc--;
				break;
			case 1:
				xInc--;
				break;
			case 2:
				yInc++;
				break;
			case 3:
				xInc++;
				break;
		}

		currentPos.tComponents[0] += xInc;
		currentPos.tComponents[1] += yInc;
		Room& thisRoom = rtRoomGrid.roomArray[currentPos.tComponents[1] * maxX + currentPos.tComponents[0]];
		if(thisRoom.bAreYouReallyReal == false) {
			// No room here = increment and add a new room
			thisRoom.bAreYouReallyReal = true;
			thisRoom.eType = it->type;
			thisRoom.x = currentPos.tComponents[0];
			thisRoom.y = currentPos.tComponents[1];
		}
	}

	return true;
}

bool Do_Saturation(MazeFramework* ptFramework, Map& rtMap, RoomGrid& rtRoomGrid ) {
	return true;
}

// Linking
bool Do_DrunkenStaggerSaturationLink(MazeFramework* ptFramework, Map& rtMap, RoomGrid& rtRoomGrid) {
	// Find the entrance.
	int i;
	vector<int> vConnectables;
	for(i = 0; i != rtRoomGrid.sizeX * rtRoomGrid.sizeY; i++) {
		if(!rtRoomGrid.roomArray[i].bAreYouReallyReal) {
			continue;
		}
		if(rtRoomGrid.roomArray[i].eType == ROOM_ENTRY) {
			break;
		}
	}
	if(i == rtRoomGrid.sizeX * rtRoomGrid.sizeY) {
		return false; // Couldn't find the entrance
	}
	
	// Assign random links to everything until we run out of orphaned rooms
	for(int j = 0; j < rtRoomGrid.sizeX * rtRoomGrid.sizeY; j++) {
		Room *ptRoom = &rtRoomGrid.roomArray[j];
		if(!ptRoom->bAreYouReallyReal) {
			continue;
		}
		if(ptRoom->iConnectionFlags == 0) {
			// Give us a random connection
			vConnectables.clear();
			vConnectables.push_back(0);
			vConnectables.push_back(1);
			vConnectables.push_back(2);
			vConnectables.push_back(3);
			if(ptRoom->y <= 0 || rtRoomGrid.roomArray[(ptRoom->y - 1) * rtRoomGrid.sizeX + ptRoom->x].bAreYouReallyReal == false) {
				VectorErase<int>(vConnectables, 0);
			}
			if(ptRoom->x <= 0 || rtRoomGrid.roomArray[ptRoom->y * rtRoomGrid.sizeX + ptRoom->x - 1].bAreYouReallyReal == false) {
				VectorErase<int>(vConnectables, 1);
			}
			if(ptRoom->y >= rtRoomGrid.sizeY || rtRoomGrid.roomArray[(ptRoom->y + 1) * rtRoomGrid.sizeX + ptRoom->x].bAreYouReallyReal == false) {
				VectorErase<int>(vConnectables, 2);
			}
			if(ptRoom->x >= rtRoomGrid.sizeX || rtRoomGrid.roomArray[ptRoom->y * rtRoomGrid.sizeX + ptRoom->x + 1].bAreYouReallyReal == false) {
				VectorErase<int>(vConnectables, 3);
			}
			if(vConnectables.size() <= 0) {
				ptRoom->bAreYouReallyReal = false; // Just kill the room
				continue;
			}

			int connection = drlgSeed.GenerateRandom(0, vConnectables.size());
			connection = vConnectables.at(connection);

			// Set our connection
			Room* otherRoom;
			switch(connection) {
				default:
				case 0:
					otherRoom = &rtRoomGrid.roomArray[(ptRoom->y-1) * rtRoomGrid.sizeX + ptRoom->x];
					ptRoom->iConnectionFlags |= (1 << ROOM_N);
					otherRoom->iConnectionFlags |= (1 << ROOM_S);
					break;
				case 1:
					otherRoom = &rtRoomGrid.roomArray[ptRoom->y * rtRoomGrid.sizeX + ptRoom->x - 1];
					ptRoom->iConnectionFlags |= (1 << ROOM_W);
					otherRoom->iConnectionFlags |= (1 << ROOM_E);
					break;
				case 2:
					otherRoom = &rtRoomGrid.roomArray[(ptRoom->y+1) * rtRoomGrid.sizeX + ptRoom->x];
					ptRoom->iConnectionFlags |= (1 << ROOM_S);
					otherRoom->iConnectionFlags |= (1 << ROOM_N);
					break;
				case 3:
					otherRoom = &rtRoomGrid.roomArray[ptRoom->y * rtRoomGrid.sizeX + ptRoom->x + 1];
					ptRoom->iConnectionFlags |= (1 << ROOM_E);
					otherRoom->iConnectionFlags |= (1 << ROOM_W);
					break;
			}

		}
	}

	// Determine if we're able to reach the entrance, first by working from the entrance's block
	RVec2<int> currentPos(rtRoomGrid.roomArray[i].x, rtRoomGrid.roomArray[i].y);
	size_t connectionSize = sizeof(bool) * rtRoomGrid.sizeX * rtRoomGrid.sizeY;
	vector<bool> bConnects(connectionSize, false);
	function<void(RVec2<int>&)> f = [&](RVec2<int>& c) -> void {
		int index = c.tComponents[1] * rtRoomGrid.sizeX + c.tComponents[0];
		Room& room = rtRoomGrid.roomArray[index];
		if(bConnects[index] == true) {
			return;
		}
		bConnects[index] = true;
		if(room.iConnectionFlags & (1 << ROOM_N)) {
			f(RVec2<int>(c.tComponents[0], c.tComponents[1]-1));
		}
		if(room.iConnectionFlags & (1 << ROOM_W)) {
			f(RVec2<int>(c.tComponents[0]-1, c.tComponents[1]));
		}
		if(room.iConnectionFlags & (1 << ROOM_S)) {
			f(RVec2<int>(c.tComponents[0], c.tComponents[1]+1));
		}
		if(room.iConnectionFlags & (1 << ROOM_E)) {
			f(RVec2<int>(c.tComponents[0]+1, c.tComponents[1]));
		}
	};
	f(currentPos);

	// Lastly, walk from the entrance and link up rooms which can't connect to us.
	vector<bool> bRanLastStage(connectionSize, false);
	// TODO: eliminate code reuse
	function<void(RVec2<int>&)> g = [&](RVec2<int>& c) -> void {
		int index = c.tComponents[1] * rtRoomGrid.sizeX + c.tComponents[0];
		int nextIndex;
		Room* ptNextRoom;
		Room* ptRoom = &rtRoomGrid.roomArray[index];
		if(bRanLastStage[index] == true) {
			return;
		}
		bRanLastStage[index] = true;
		if(c.tComponents[1] > 0) {
			nextIndex = index - rtRoomGrid.sizeX;
			ptNextRoom = &rtRoomGrid.roomArray[nextIndex];
			if(ptNextRoom->bAreYouReallyReal && !bConnects[nextIndex]) {
				ptRoom->iConnectionFlags |= (1 << ROOM_N);
				ptNextRoom->iConnectionFlags |= (1 << ROOM_S);
				f(RVec2<int>(ptNextRoom->x, ptNextRoom->y));
				g(RVec2<int>(ptNextRoom->x, ptNextRoom->y));
			} else if(bConnects[nextIndex] && !bRanLastStage[nextIndex]) {
				g(RVec2<int>(ptNextRoom->x, ptNextRoom->y));
			}
		}
		if(c.tComponents[0] > 0) {
			nextIndex = index - 1;
			ptNextRoom = &rtRoomGrid.roomArray[nextIndex];
			if(ptNextRoom->bAreYouReallyReal && !bConnects[nextIndex]) {
				ptRoom->iConnectionFlags |= (1 << ROOM_W);
				ptNextRoom->iConnectionFlags |= (1 << ROOM_E);
				f(RVec2<int>(ptNextRoom->x, ptNextRoom->y));
				g(RVec2<int>(ptNextRoom->x, ptNextRoom->y));
			} else if(bConnects[nextIndex] && !bRanLastStage[nextIndex]) {
				g(RVec2<int>(ptNextRoom->x, ptNextRoom->y));
			}
		}
		if(c.tComponents[1] < rtRoomGrid.sizeY - 1) {
			nextIndex = index + rtRoomGrid.sizeX;
			ptNextRoom = &rtRoomGrid.roomArray[nextIndex];
			if(ptNextRoom->bAreYouReallyReal && !bConnects[nextIndex]) {
				ptRoom->iConnectionFlags |= (1 << ROOM_S);
				ptNextRoom->iConnectionFlags |= (1 << ROOM_N);
				f(RVec2<int>(ptNextRoom->x, ptNextRoom->y));
				g(RVec2<int>(ptNextRoom->x, ptNextRoom->y));
			} else if(bConnects[nextIndex] && !bRanLastStage[nextIndex]) {
				g(RVec2<int>(ptNextRoom->x, ptNextRoom->y));
			}
		}
		if(c.tComponents[0] < rtRoomGrid.sizeX - 1) {
			nextIndex = index + 1;
			ptNextRoom = &rtRoomGrid.roomArray[nextIndex];
			if(ptNextRoom->bAreYouReallyReal && !bConnects[nextIndex]) {
				ptRoom->iConnectionFlags |= (1 << ROOM_E);
				ptNextRoom->iConnectionFlags |= (1 << ROOM_W);
				f(RVec2<int>(ptNextRoom->x, ptNextRoom->y));
				g(RVec2<int>(ptNextRoom->x, ptNextRoom->y));
			} else if(bConnects[nextIndex] && !bRanLastStage[nextIndex]) {
				g(RVec2<int>(ptNextRoom->x, ptNextRoom->y));
			}
		}
	};
	g(currentPos);
	vConnectables.clear();
	return true;
}

void Maze_Build(MazeFramework* ptFramework, Map& rtMap, RoomGrid& rtRoomGrid, DungeonManager* ptDungeonManager) {
	int iTilesDone = 0;
	for(int i = 0; i < rtRoomGrid.sizeX * rtRoomGrid.sizeY; i++) {
		Room* ptRoom = &rtRoomGrid.roomArray[i];
		if(!ptRoom->bAreYouReallyReal) {
			continue;
		}
		string preset = ptFramework->tileSet;
		if(ptRoom->eType != ROOM_FILLER) {
			switch(ptRoom->eType) {
			case ROOM_SPECIAL_0:
			case ROOM_SPECIAL_1:
			case ROOM_SPECIAL_2:
			case ROOM_SPECIAL_3:
			case ROOM_SPECIAL_4:
			case ROOM_SPECIAL_5:
			case ROOM_SPECIAL_6:
			case ROOM_SPECIAL_7:
				preset += "spc";
				preset += ptRoom->eType - ROOM_SPECIAL_0;
				break;
			case ROOM_SHRINE:
				preset += "shr";
				break;
			case ROOM_ENTRY:
				preset += "etr";
				break;
			case ROOM_EXIT:
				preset += "exi";
				break;
			}
		}
		if(ptRoom->iConnectionFlags & (1 << ROOM_N)) {
			preset += "N";
		}
		if(ptRoom->iConnectionFlags & (1 << ROOM_W)) {
			preset += "W";
		}
		if(ptRoom->iConnectionFlags & (1 << ROOM_S)) {
			preset += "S";
		}
		if(ptRoom->iConnectionFlags & (1 << ROOM_E)) {
			preset += "E";
		}
		iTilesDone++;

		PresetFileData* pfd = ptDungeonManager->GetPFD(preset);
		if(!pfd) {
			R_Message(PRIORITY_WARNING, "WARNING: couldn't find pfd: %s\n", preset.c_str());
			continue;
		}
		R_Message(PRIORITY_DEBUG, "DRLG: requesting pfd resource %s\n", preset.c_str());
		TileNode* tiles = (TileNode*)trap->Zone_Alloc(sizeof(TileNode)*pfd->head.numTiles, "tiles");
		for(int j = 0; j < pfd->head.numTiles; j++) {
			auto tile = pfd->tileBlocks[j];
			int roomOffsetX = (i % rtRoomGrid.sizeX) + (ptRoom->x * rtRoomGrid.sizeX);
			int roomOffsetY = ((int)floor((float)i / rtRoomGrid.sizeX)) + (ptRoom->y * rtRoomGrid.sizeY);
			tiles[j].x = tile.x + rtMap.x + roomOffsetX - ptRoom->x;
			tiles[j].y = tile.y + rtMap.y + roomOffsetY - ptRoom->y;
			tiles[j].w = tiles[j].h = 1;
			tiles[j].ptTile = ptDungeonManager->GetTileByName(tile.lookup);
			if(tiles[j].ptTile == nullptr) {
				R_Message(PRIORITY_WARNING, "WARNING: couldn't find tile %s in pfd %s\n", tile.lookup, preset.c_str());
				continue;
			}
			tiles[j].rt = (tileRenderType_e)tile.rt;
			rtMap.qtTileTree.AddNode(&(tiles[j]));
		}

		for(int j = 0; j < pfd->head.numEntities; j++) {
			auto loadedEnt = pfd->entities[j];
			float roomOffsetX = (i % rtRoomGrid.sizeX) + (ptRoom->x * rtRoomGrid.sizeX) - ptRoom->x;
			float roomOffsetY = floor((float)i / rtRoomGrid.sizeX) + (ptRoom->y * rtRoomGrid.sizeY) - ptRoom->y;
			Entity* ent = ptDungeonManager->GenerateEntity(loadedEnt.lookup, loadedEnt.x + rtMap.x + roomOffsetX, 
				loadedEnt.y + rtMap.y + roomOffsetY, loadedEnt.spawnflags, rtMap.iAct);
			Worldspace* ptWorld = ptDungeonManager->GetWorld(rtMap.iAct);
			ptWorld->SpawnEntity(ent, ent->bShouldWeRender, ent->bShouldWeThink, ent->bShouldWeCollide);
			ent->ptContainingTree = rtMap.qtEntTree.AddNode(ent);
		}
	}
}

// Construct rooms
randomizeDungeonFunc randomizeDungeonFuncs[] = {
	Do_DrunkenStagger,					// parm0 = how many rooms to walk, parm1 = special room walk, parm2 = special room corridor walk
	Do_Saturation
};

// Set up links
randomizeDungeonFunc linkDungeonFuncs[] = {
	Do_DrunkenStaggerSaturationLink,
	Do_DrunkenStaggerSaturationLink
};

void RandomizeDungeon(MazeFramework* ptFramework, Map& rtMap, DungeonManager* ptDungeonManager) {
	RoomGrid rg;
	randomizeDungeonFuncs[ptFramework->algorithm](ptFramework, rtMap, rg);
	linkDungeonFuncs[ptFramework->algorithm](ptFramework, rtMap, rg);
	Maze_Build(ptFramework, rtMap, rg, ptDungeonManager);
	trap->Zone_FastFree(rg.roomArray, "roomgrid");
}