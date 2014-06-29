#include "g_json.h"

// LEVEL PARSING
static unordered_map<const char*, jsonParseFunc> levelParseFields;
void InitLevelParseFields() {
#define NAME_PARSER [](cJSON* j, void* p) -> void { MapFramework* t = (MapFramework*)p; strcpy(t->name, cJSON_ToString(j)); }
#define FIRST_PARSER [](cJSON* j, void* p) -> void { MapFramework* t = (MapFramework*)p; strcpy(t->entryPreset, cJSON_ToString(j)); }
#define LEVEL_PARSER [](cJSON* j, void* p) -> void { MapFramework* t = (MapFramework*)p; t->nDungeonLevel = cJSON_ToInteger(j); }
#define VIS_PARSER(x) [](cJSON* j, void*p) -> void { MapFramework* t = (MapFramework*)p; t->sLink[x] = cJSON_ToString(j); }
#define P_PARSER [](cJSON* j, void* p) -> void { MapFramework* t = (MapFramework*)p; t->bIsPreset = cJSON_ToBoolean(j); }
#define INT_PARSER(x) [](cJSON* j, void* p) -> void { MapFramework* t = (MapFramework*)p; t->##x## = cJSON_ToInteger(j); }
	levelParseFields["name"] = NAME_PARSER;
	levelParseFields["level"] = LEVEL_PARSER;
	levelParseFields["first"] = FIRST_PARSER;
	levelParseFields["preset"] = P_PARSER;
	levelParseFields["vis0"] = VIS_PARSER(0);
	levelParseFields["vis1"] = VIS_PARSER(1);
	levelParseFields["vis2"] = VIS_PARSER(2);
	levelParseFields["vis3"] = VIS_PARSER(3);
	levelParseFields["vis4"] = VIS_PARSER(4);
	levelParseFields["vis5"] = VIS_PARSER(5);
	levelParseFields["vis6"] = VIS_PARSER(6);
	levelParseFields["vis7"] = VIS_PARSER(7);
	levelParseFields["worldX"] = INT_PARSER(iWorldspaceX);
	levelParseFields["worldY"] = INT_PARSER(iWorldspaceY);
	levelParseFields["sizeX"] = INT_PARSER(iSizeX);
	levelParseFields["sizeY"] = INT_PARSER(iSizeY);
	levelParseFields["act"] = INT_PARSER(iAct);
#undef NAME_PARSER
#undef INT_PARSER
}


// TILE PARSING
static unordered_map<const char*, jsonParseFunc> tileParseFields;

void ParseSubtile(cJSON* json, void* ptTile, int tileNum) {
	Tile* t = (Tile*)ptTile;
	if(!t) {
		return;
	}
	cJSON* jsonNode = cJSON_GetObjectItem(json, "low");
	if(jsonNode) {
		t->lowmask |= (1 << tileNum);
	}

	jsonNode = cJSON_GetObjectItem(json, "high");
	if(jsonNode) {
		t->highmask |= (1 << tileNum);
	}

	jsonNode = cJSON_GetObjectItem(json, "light");
	if(jsonNode) {
		t->lightmask |= (1 << tileNum);
	}

	jsonNode = cJSON_GetObjectItem(json, "vis");
	if(jsonNode) {
		t->vismask |= (1 << tileNum);
	}
}

void AutoTransParser(cJSON* j, void* p) {
	Tile* t = (Tile*)p;
	t->bAutoTrans = true;

	cJSON* node = cJSON_GetObjectItem(j, "x");
	t->iAutoTransX = cJSON_ToInteger(node);
	node = cJSON_GetObjectItem(j, "y");
	t->iAutoTransY = cJSON_ToInteger(node);
	node = cJSON_GetObjectItem(j, "w");
	t->iAutoTransW = cJSON_ToInteger(node);
	node = cJSON_GetObjectItem(j, "h");
	t->iAutoTransH = cJSON_ToInteger(node);
}

void InitTileParseFields() {
#define SUBTILE_PARSER(x) [](cJSON* j, void* p) -> void { ParseSubtile(j, p, x); }
#define INT_PARSER(x) [](cJSON* j, void* p) -> void { Tile* t = (Tile*)p; t->##x## = cJSON_ToInteger(j); }
#define FLOAT_PARSER(x) [](cJSON* j, void* p) -> void { Tile* t = (Tile*)p; t->##x## = cJSON_ToNumber(j); }
#define NAME_PARSER [](cJSON* j, void* p) -> void { Tile* t = (Tile*)p; strcpy(t->name, cJSON_ToString(j)); }
#define MAT_PARSER \
	[](cJSON* j, void* p) -> void { Tile* t = (Tile*)p; t->materialHandle = trap->RegisterMaterial(cJSON_ToString(j)); }

	tileParseFields["subtile0"] = SUBTILE_PARSER(0);
	tileParseFields["subtile1"] = SUBTILE_PARSER(1);
	tileParseFields["subtile2"] = SUBTILE_PARSER(2);
	tileParseFields["subtile3"] = SUBTILE_PARSER(3);
	tileParseFields["subtile4"] = SUBTILE_PARSER(4);
	tileParseFields["subtile5"] = SUBTILE_PARSER(5);
	tileParseFields["subtile6"] = SUBTILE_PARSER(6);
	tileParseFields["subtile7"] = SUBTILE_PARSER(7);
	tileParseFields["subtile8"] = SUBTILE_PARSER(8);
	tileParseFields["subtile9"] = SUBTILE_PARSER(9);
	tileParseFields["subtile10"] = SUBTILE_PARSER(10);
	tileParseFields["subtile11"] = SUBTILE_PARSER(11);
	tileParseFields["subtile12"] = SUBTILE_PARSER(12);
	tileParseFields["subtile13"] = SUBTILE_PARSER(13);
	tileParseFields["subtile14"] = SUBTILE_PARSER(14);
	tileParseFields["subtile15"] = SUBTILE_PARSER(15);
	tileParseFields["lowmask"] = INT_PARSER(lowmask);
	tileParseFields["highmask"] = INT_PARSER(highmask);
	tileParseFields["lightmask"] = INT_PARSER(lightmask);
	tileParseFields["vismask"] = INT_PARSER(vismask);
	tileParseFields["name"] = NAME_PARSER;
	tileParseFields["material"] = MAT_PARSER;
	tileParseFields["depthoffset"] = FLOAT_PARSER(fDepthScoreOffset);
	tileParseFields["autotrans"] = AutoTransParser;
}
#undef NAME_PARSER
#undef INT_PARSER

void MapLoader::LoadPreset(const string& path) {
	if(path.size() <= 0) {
		return;
	}

	PresetFileData* pfd = (PresetFileData*)trap->Zone_Alloc(sizeof(PresetFileData), "pfd");
	if(!pfd) {
		R_Error("Out of memory.");
		return;
	}

	File* file = trap->OpenFile(path.c_str(), "rb");
	if(!file) {
		R_Error("Directory failure in levels/preset: %s", path.c_str());
		trap->Zone_FastFree(pfd, "pfd");
		return; // ? shouldn't happen
	}

	if(trap->GetFileSize(file) <= 0) {
		R_Printf("WARNING: BDF file (%s) is empty!\n", path.c_str());
		trap->CloseFile(file);
		trap->Zone_FastFree(pfd, "pfd");
		return;
	}

	// Take care of the (constant) header
	trap->ReadBinary(file, (unsigned char*)&pfd->head, sizeof(pfd->head), false);
	if(strnicmp(pfd->head.header, "DRLG.BDF", sizeof(pfd->head.header))) {
		R_Printf("WARNING: BDF file (%s) contains invalid header! (found: \"%s\", expected \"DRLG.BDF\")\n",
			path.c_str(), pfd->head.header);
		trap->CloseFile(file);
		trap->Zone_FastFree(pfd, "pfd");
		return;
	}

	if(pfd->head.version != 1) { // TODO: use proper compare
		R_Printf("WARNING: BDF file (%s) uses invalid version number! (%i)\n",
			path.c_str(), pfd->head.version);
		trap->CloseFile(file);
		trap->Zone_FastFree(pfd, "pfd");
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
		trap->Zone_FastFree(pfd, "pfd");
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
		memcpy(ents, (void*)(binaryFile + tileSize), entSize);
#ifdef BIG_ENDIAN
		// see above
		for(int i = 0; i < pfd->head.numEntities; i++) {
			eswap(ents[i].spawnflags);
			eswap(ents[i].reserved1);
		}
#endif // BIG_ENDIAN
		pfd->entities = ents;
	} else {
		pfd->entities = NULL;
	}
	delete[] binaryFile;
	trap->CloseFile(file);

	mPfd[pfd->head.lookup] = pfd;
	R_Printf("DEBUG: Loaded %s\n", pfd->head.lookup);
}

void MapLoader::LoadPresets() {
	int numFiles = 0;
	char** paths = trap->ListFilesInDir("levels/preset", ".bdf", &numFiles);
	if(numFiles <= 0) {
		R_Error("Could not load presets in levels/preset (none?)");
		return;
	}

	// We use a special zone tag for presets, we should add that now
	trap->Zone_NewTag("pfd");

	for(int i = 0; i < numFiles; i++) {
		LoadPreset(paths[i]);
	}

	R_Printf("Loaded %i presets\n", numFiles);
	trap->FreeFileList(paths, numFiles);
}

MapLoader::MapLoader(const char* presetsPath, const char* tilePath) {
	// Load the tiles.
	InitTileParseFields();
	int numFiles = 0; 
	char** paths = trap->ListFilesInDir(tilePath, ".json", &numFiles);
	if(numFiles <= 0) {
		R_Error("MapLoader could not load tiles (missing?)");
	}
	for(int i = 0; i < numFiles; i++) {
		Tile t;
		JSON_ParseFile(paths[i], tileParseFields, &t);
		vTiles.push_back(t);
	}
	for(auto it = vTiles.begin(); it != vTiles.end(); ++it) {
		mTileResolver[it->name] = it;
	}

	// Load the Levels.json
	InitLevelParseFields();
	JSON_ParseMultifile<MapFramework>("levels/Levels.json", levelParseFields, vMapData);

	// Load presets.
	LoadPresets();

	trap->FreeFileList(paths, numFiles);
}

MapLoader::~MapLoader() {
	mTileResolver.clear();
	vTiles.clear();
	tileParseFields.clear();
	mPfd.clear();
	trap->Zone_FreeAll("pfd");
}

Tile* MapLoader::FindTileByName(const string& str) {
	auto it = mTileResolver.find(str);
	if(it == mTileResolver.end()) {
		return NULL;
	}
	return &(*(it->second)); // HACK
}

MapFramework* MapLoader::FindMapFrameworkByName(const char* name) {
	for(auto it = vMapData.begin(); it != vMapData.end(); ++it) {
		if(!stricmp(it->name, name)) {
			return &(*it);
		}
	}
	return NULL;
}

PresetFileData* MapLoader::FindPresetByName(const string& str) {
	auto it = mPfd.find(str);
	if(it == mPfd.end()) {
		return NULL;
	}
	return it->second;
}