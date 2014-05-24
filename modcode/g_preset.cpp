#include "g_local.h"

unordered_map<string, PresetFileData*> presets;

static void MP_LoadTilePreset(const string& path) {
	if(path.size() <= 0) {
		return;
	}

	PresetFileData* pfd = (PresetFileData*)malloc(sizeof(PresetFileData));
	void* file = trap->OpenFile(path.c_str(), "rb");
	if(!file) {
		R_Error("Directory failure in levels/preset: %s", path.c_str());
		free(pfd);
		return; // ? shouldn't happen
	}

	// Take care of the (constant) header
	trap->ReadBinary(file, (unsigned char*)&pfd->head, sizeof(pfd->head), false);
	if(strnicmp(pfd->head.header, "DRLG.BDF", sizeof(pfd->head.header))) {
		R_Printf("WARNING: BDF file (%s) contains invalid header! (found: \"%s\", expected \"DRLG.BDF\")\n",
			path.c_str(), pfd->head.header);
		free(pfd);
		return;
	}

	if(pfd->head.version != 1) { // TODO: use proper compare
		R_Printf("WARNING: BDF file (%s) uses invalid version number! (%i)\n",
			path.c_str(), pfd->head.version);
		free(pfd);
		return;
	}
	R_Printf("DEBUG: Loading BDF (%s): %s\n", path.c_str(), pfd->head.lookup);

#ifndef BIG_ENDIAN
	// read: i'm an idiot.
	eswap(pfd->head.numEntities);
	eswap(pfd->head.numTiles);
	eswap(pfd->head.reserved1);
	eswap(pfd->head.reserved2);
	eswap(pfd->head.reserved3);
	eswap(pfd->head.sizeX);
	eswap(pfd->head.sizeY);
#endif // BIG_ENDIAN

	size_t fileSize = sizeof(pfd->head);
	size_t tileSize = sizeof(*pfd->tileBlocks)*pfd->head.numTiles;
	size_t entSize = sizeof(*pfd->entities)*pfd->head.numEntities;
	unsigned char* binaryFile = new unsigned char[tileSize + entSize];	// FIXME: RAII compliant pls
	trap->ReadBinary(file, binaryFile, tileSize + entSize, true);

	/*if(trap->GetFileSize(file)-sizeof(pfd->head) < tileSize + entSize) {
		R_Printf("WARNING: BDF file (%s) is incomplete!\n", path.c_str());
		delete binaryFile;
		return;
	}*/

	if(pfd->head.numTiles > 0) {
		PresetFileData::LoadedTile* tiles = new PresetFileData::LoadedTile[pfd->head.numTiles];
		memcpy(tiles, (void*)binaryFile, tileSize);
		fileSize += tileSize;
		pfd->tileBlocks = tiles;
	} else {
		pfd->tileBlocks = NULL;
	}
	
	if(pfd->head.numEntities > 0) {
		PresetFileData::LoadedEntity* ents = new PresetFileData::LoadedEntity[pfd->head.numEntities];
		memcpy(ents, (void*)(binaryFile+tileSize), entSize);
#ifndef BIG_ENDIAN
		// see above
		for(int i = 0; i < pfd->head.numEntities; i++) {
			eswap(pfd->entities[i].spawnflags);
			eswap(pfd->entities[i].reserved1);
		}
#endif // BIG_ENDIAN
		pfd->entities = ents;
	} else {
		pfd->entities = NULL;
	}
	delete binaryFile;
	trap->CloseFile(file);

	presets[pfd->head.lookup] = pfd;
	R_Printf("DEBUG: Loaded %s\n", pfd->head.lookup);
}

void MP_LoadTilePresets() {
	vector<string> paths;
	int numFiles = trap->ListFilesInDir("levels/preset", paths, ".bdf");
	if(paths.size() <= 0) {
		R_Error("Could not load presets in levels/preset (none?)");
		return;
	}

	for(auto it = paths.begin(); it != paths.end(); ++it) {
		MP_LoadTilePreset(*it);
	}

	R_Printf("Loaded %i presets\n", paths.size());
}

void MP_AddToMap(const char* pfdName, Map& map) {
	if(!pfdName) {
		R_Printf("WARNING: MP_AddToMap: NULL pfdName\n");
		return;
	}

	const PresetFileData* pfd = MP_GetPreset(pfdName);
	if(map.bIsPreset) {
		for(int i = 0; i < pfd->head.numTiles; i++) {
			auto tile = pfd->tileBlocks[i];
			TileNode s;
			s.x = tile.x; s.y = tile.y;
			s.ptTile = maps->FindTileByName(tile.lookup);
			map.tiles.push_back(s);
		}
	} else {
		// TODO: DRLG
	}
}

PresetFileData* MP_GetPreset(const string& sName) {
	auto it = presets.find(sName);
	if(it == presets.end()) {
		return NULL;
	}
	return it->second;
}