#include "DungeonManager.h"
#include "MapLoader.h"
#include "PresetFileData.h"

#include "entities\info_player_start.h"

DungeonManager* ptDungeonManager;

#define DEF_SPAWN_FUNC(x)	mSpawnFuncs["" #x ""] = ##x##::spawnme
DungeonManager::DungeonManager() {
	// Initialize the DungeonManager, creating all the spawn funcs and initializing zone memory
	R_Printf("loading dungeons...\n");
	ptMapLoader = new MapLoader("levels/preset", "levels/tiles");

	DEF_SPAWN_FUNC(info_player_start);
	DEF_SPAWN_FUNC(Player);

	trap->Zone_NewTag("tiles");
}

DungeonManager::~DungeonManager() {
	// Clear out stuff we no longer need. // FIXME: memory leaking here?
	delete ptMapLoader;
	trap->Zone_FreeAll("tiles");
}

Worldspace* DungeonManager::GetWorld(unsigned int iAct) {
	// Find the worldspace we're looking for
	if(iAct >= NUM_ACTS) {
		return NULL;
	}
	return &(wActs[iAct]);
}

void DungeonManager::StartBuild(const string& sDungeonName) {
	// Start building from a level. We take that map's framework, and then build everything from that point recursively.
	MapFramework* ptFramework = ptMapLoader->FindMapFrameworkByName(sDungeonName.c_str());
	if(!ptFramework) {
		return;
	}
	Construct(ptFramework);
}

void DungeonManager::PresetGeneration(const MapFramework* ptFramework, Map& in) {
	// Generate a preset level.
	// Locate the pfd in storage
	const PresetFileData* pfd = ptMapLoader->FindPresetByName(ptFramework->entryPreset);
	if(!pfd) {
		R_Printf("WARNING: Couldn't find pfd: %s\n", ptFramework->entryPreset);
		return;
	}

	strcpy(in.name, ptFramework->name); // FIXME: not needed?

	// Add tiles to the map
	if(pfd->head.numTiles > 0) {
		TileNode* v = (TileNode*)trap->Zone_Alloc(sizeof(TileNode)*pfd->head.numTiles, "tiles");
		for(int i = 0; i < pfd->head.numTiles; i++) {
			// Construct the tile from the pfd
			auto tile = pfd->tileBlocks[i];
			v[i].x = tile.x + in.x; v[i].y = tile.y + in.y;
			v[i].w = 1; v[i].h = 1;
			v[i].ptTile = ptMapLoader->FindTileByName(tile.lookup);
			v[i].rt = (tileRenderType_e)tile.rt;
			if(v[i].ptTile == NULL) {
				R_Printf("Cannot find tile %s\n", tile.lookup);
				return;
			}
			// Add the tile to the map
			in.qtTileTree.AddNode(&(v[i]));
		}
	}

	// Add entities to the map
	for(int i = 0; i < pfd->head.numEntities; i++) {
		auto loadedEnt = pfd->entities[i];
		// Spawn an instance of the entity by calling its ::spawnme function
		Entity* ent = GenerateEntity(loadedEnt.lookup, loadedEnt.x + ptFramework->iWorldspaceX, loadedEnt.y + ptFramework->iWorldspaceY, loadedEnt.spawnflags, ptFramework->iAct);
		// Actually call its spawn() function
		wActs[ptFramework->iAct].SpawnEntity(ent, ent->bShouldWeRender, ent->bShouldWeThink, ent->bShouldWeCollide);
		// Lastly, put it in the map that it belongs in
		ent->ptContainingTree = in.qtEntTree.AddNode(ent);
	}
}

void DungeonManager::Construct(const MapFramework* ptFramework) {
	if(!ptFramework) {
		return;
	}

	// Check to see if we've already made this map. This is to prevent generating the same area twice.
	auto found = mHaveWeAlreadyBuilt.find(ptFramework->name);
	if(found != mHaveWeAlreadyBuilt.end()) {
		return; // Already built the map.
	}

	// Create the base map
	Map *theMap = new Map(ptFramework->iWorldspaceX, ptFramework->iWorldspaceY,
		ptFramework->iSizeX, ptFramework->iSizeY, ptFramework->iAct, 1);
	strcpy(theMap->name, ptFramework->name);
	for(int i = 0; i < MAX_MAP_LINKS; i++) {
		if(ptFramework->sLink[i].length() > 0) {
			strcpy(theMap->links[i], ptFramework->sLink[i].c_str());
		} else {
			theMap->links[i][0] = '\0';
		}
	}
	mHaveWeAlreadyBuilt[ptFramework->name] = theMap; // stops infinite recursion below

	// Dungeon generation
	if(ptFramework->bIsPreset) {
		// It's preset, so lets just do that then
		PresetGeneration(ptFramework, *theMap);
	} else {
		// DRLG generation -- currently not done
		return;
	}

	// For each map that this one links to, we generate those maps (recursively)
	for(int i = 0; i < 8; i++) {
		const string& str = ptFramework->sLink[i];
		if(str.length() > 0) {
			// We have a link here, so construct it
			MapFramework* ptLinkFramework = ptMapLoader->FindMapFrameworkByName(str.c_str());
			if(ptLinkFramework) {
				Construct(ptLinkFramework);
			}
		}
	}

	// Insert the map into the worldspace
	GetWorld(ptFramework->iAct)->InsertInto(theMap);
	mHaveWeAlreadyBuilt[ptFramework->name] = theMap; // FIXME: not needed?
}

Entity* DungeonManager::GenerateEntity(const char* name, float x, float y, int spawnflags, int act) {
	// Call the entity's ::spawnme method.
	auto func = mSpawnFuncs.find(name);
	if(func == mSpawnFuncs.end()) {
		return nullptr;
	}

	return func->second(x, y, spawnflags, act);
}

void DungeonManager::SpawnPlayer(const string& sDungeonName) {
	auto theMap = mHaveWeAlreadyBuilt.find(sDungeonName);
	if(theMap == mHaveWeAlreadyBuilt.end()) {
		// Couldn't spawn, that level doesn't exist
		return;
	}

	// Find an info_player_start with the SPAWNFLAG_PLAYSPAWN flag set.
	vector<Entity*> ents = theMap->second->FindEntities("info_player_start");
	if(ents.size() <= 0) {
		return; // no spawn points, should throw an error?
	}

	Entity* ent = nullptr;
	for(auto it = ents.begin(); it != ents.end(); ++it) {
		if((*it)->GetSpawnflags() & info_player_start::SPAWNFLAG_PLAYSPAWN) {
			ent = *it;
			break;
		}
	}
	if(ent == nullptr) {
		// couldn't find an appropriate spawn point. should throw an error/use alternatives?
		return;
	}

	// Generate a player entity, spawn it, and alert the world that there's a new player in the game.
	Entity* playerEnt = GenerateEntity("Player", ent->x, ent->y, 0, theMap->second->iAct);
	Player* ptPlayer = (Player*)playerEnt;
	wActs[theMap->second->iAct].SpawnEntity(playerEnt, playerEnt->bShouldWeRender, playerEnt->bShouldWeThink, playerEnt->bShouldWeCollide);
	wActs[theMap->second->iAct].AddPlayer(ptPlayer);
	playerEnt->ptContainingTree = theMap->second->qtEntTree.AddNode(playerEnt);

	// networking FIXME
	thisClient->ptPlayer = ptPlayer;
	ptPlayer->SignalZoneChange(ent->x, ent->y, sDungeonName.c_str());
}

Map* DungeonManager::FindProperMap(int iAct, float x, float y) {
	if(iAct < 0 || iAct >= NUM_ACTS) {
		return nullptr;
	}
	Worldspace* world = &(wActs[iAct]);
	auto nodes = world->qtMapTree->NodesAt(x, y);
	for(auto it = nodes.begin(); it != nodes.end(); ++it) {
		if(x >= (*it)->x && x < (*it)->x + (*it)->w &&
			y >= (*it)->y && y < (*it)->y + (*it)->h) {
				return *it;
		}
	}
	return nullptr;
}

Map* DungeonManager::FindProperMap(int iAct, int x, int y) {
	return FindProperMap(iAct, (float)x, (float)y);
}

string DungeonManager::FindNextMapStr(int iAct, int iPlayerNum, int iVisNumber) {
	Player* ply = wActs[iAct].GetFirstPlayer();	// FIXME
	Map* ourMap = FindProperMap(iAct, ply->x, ply->y);
	MapFramework* nextMap = ptMapLoader->FindMapFrameworkByName(ourMap->links[iVisNumber]);
	if(!nextMap) {
		return "";
	}
	return nextMap->toName;
}

void DungeonManager::MovePlayerToVis(int iAct, int iPlayerNum, int iVisNumber) {
	Player* ply = wActs[iAct].GetFirstPlayer();
	Map* ourMap = FindProperMap(iAct, ply->x, ply->y);
	MapFramework* ptNextFrame = ptMapLoader->FindMapFrameworkByName(ourMap->links[iVisNumber]);
	if(!ptNextFrame) {
		// Don't move us to the next vis if we can't find the framework for it!
		return;
	}
	int iNextVis = -1;
	for(int i = 0; i < MAX_MAP_LINKS; i++) {
		if(!stricmp(ptNextFrame->sLink[i].c_str(), ourMap->name)) {
			iNextVis = i;
			break;
		}
	}
	if(iNextVis == -1) {
		// This is somewhat of a stupid mechanics restriction, but don't allow us to go into maps
		// which don't link back to us. This prevents the player from getting stuck in zones.
		return;
	}

	Map* nextMap = FindProperMap(iAct, ptNextFrame->iWorldspaceX + (ptNextFrame->iSizeX / 2), 
		ptNextFrame->iWorldspaceY + (ptNextFrame->iSizeY / 2));
	if(!nextMap) {
		// Cannot proceed if we can't find the next map...
		return;
	}

	vector<Entity*> vSpawnPoints = nextMap->FindEntities("info_player_start");
	Entity* ptSpawn = nullptr;
	for(auto it = vSpawnPoints.begin(); it != vSpawnPoints.end(); ++it) {
		if((*it)->GetSpawnflags() & (1 << iNextVis)) {
			ptSpawn = *it;
		}
	}
	if(ptSpawn == nullptr) {
		// FIXME: maybe throw an error? Couldn't find a spawn point?
		return;
	}

	ply->SignalZoneChange(ptSpawn->x, ptSpawn->y, nextMap->name);
}