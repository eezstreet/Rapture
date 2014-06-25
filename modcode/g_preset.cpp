#include "g_local.h"

unordered_map<string, PresetFileData*> presets;

static void MP_LoadTilePreset(const string& path) {
	if(path.size() <= 0) {
		return;
	}

	PresetFileData* pfd = (PresetFileData*)malloc(sizeof(PresetFileData));
	if(!pfd) {
		R_Error("Out of memory.");
		return;
	}

	File* file = trap->OpenFile(path.c_str(), "rb");
	if(!file) {
		R_Error("Directory failure in levels/preset: %s", path.c_str());
		free(pfd);
		return; // ? shouldn't happen
	}

	if(trap->GetFileSize(file) <= 0) {
		R_Printf("WARNING: BDF file (%s) is empty!\n", path.c_str());
		trap->CloseFile(file);
		free(pfd);
		return;
	}

	// Take care of the (constant) header
	trap->ReadBinary(file, (unsigned char*)&pfd->head, sizeof(pfd->head), false);
	if(strnicmp(pfd->head.header, "DRLG.BDF", sizeof(pfd->head.header))) {
		R_Printf("WARNING: BDF file (%s) contains invalid header! (found: \"%s\", expected \"DRLG.BDF\")\n",
			path.c_str(), pfd->head.header);
		trap->CloseFile(file);
		free(pfd);
		return;
	}

	if(pfd->head.version != 1) { // TODO: use proper compare
		R_Printf("WARNING: BDF file (%s) uses invalid version number! (%i)\n",
			path.c_str(), pfd->head.version);
		trap->CloseFile(file);
		free(pfd);
		return;
	}
	R_Printf("DEBUG: Loading BDF (%s): %s\n", path.c_str(), pfd->head.lookup);

#ifdef BIG_ENDIAN
	// read: i'm an idiot.
	eswap(pfd->head.numEntities);
	eswap(pfd->head.numTiles);
	eswap(pfd->head.reserved1);
	eswap(pfd->head.reserved2);
	eswap(pfd->head.reserved3);
	eswap(pfd->head.sizeX);
	eswap(pfd->head.sizeY);
#endif // BIG_ENDIAN

	if(pfd->head.numTiles == 0 && pfd->head.numEntities == 0) {
		// Don't try to alloc zero memory..
		R_Printf("WARNING: no entities or tiles in %s..not loading\n", path.c_str());
		trap->CloseFile(file);
		free(pfd);
		return;
	}

	size_t fileSize = sizeof(pfd->head);
	size_t tileSize = sizeof(*pfd->tileBlocks)*pfd->head.numTiles;
	size_t entSize = sizeof(*pfd->entities)*pfd->head.numEntities;
	unsigned char* binaryFile = new unsigned char[tileSize + entSize];	// FIXME: RAII compliant pls
	trap->ReadBinary(file, binaryFile, tileSize + entSize, true);

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
	delete[] binaryFile;
	trap->CloseFile(file);

	presets[pfd->head.lookup] = pfd;
	R_Printf("DEBUG: Loaded %s\n", pfd->head.lookup);
}

void MP_LoadTilePresets() {
	int numFiles = 0;
	char** paths = trap->ListFilesInDir("levels/preset", ".bdf", &numFiles);
	if(numFiles <= 0) {
		R_Error("Could not load presets in levels/preset (none?)");
		return;
	}

	for(int i = 0; i < numFiles; i++) {
		MP_LoadTilePreset(paths[i]);
	}

	R_Printf("Loaded %i presets\n", numFiles);
	trap->FreeFileList(paths, numFiles);
}

void MP_AddToMap(const char* pfdName, Map& map) {
	if(!pfdName) {
		R_Printf("WARNING: MP_AddToMap: NULL pfdName\n");
		return;
	}

	const PresetFileData* pfd = MP_GetPreset(pfdName);
	if(!pfd) {
		R_Printf("WARNING: Couldn't find pfd: %s\n", pfdName);
		return;
	}
	if(map.bIsPreset) {
		for(int i = 0; i < pfd->head.numTiles; i++) {
			auto tile = pfd->tileBlocks[i];
			TileNode s;
			s.x = tile.x; s.y = tile.y;
			s.ptTile = maps->FindTileByName(tile.lookup);
			s.rt = (tileRenderType_e)tile.rt;
			if(s.ptTile == NULL) {
				R_Printf("Cannot find tile %s\n", tile.lookup);
				return;
			}
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