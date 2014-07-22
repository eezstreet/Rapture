#include "MapLoader.h"
#include "Json.h"

// LEVEL PARSING
static unordered_map<const char*, jsonParseFunc> levelParseFields;

void ParseAlwaysRooms(cJSON* j, void* p) {
	MapFramework* t = (MapFramework*)p;
	int max = cJSON_GetArraySize(j);
	for(int i = 0; i < max; i++) {
		cJSON* node = cJSON_GetArrayItem(j, i);
		cJSON* childNode = cJSON_GetObjectItem(node, "type");
		if(!childNode) {
			continue;
		}
		const char* str = cJSON_ToString(childNode);
		AlwaysRoomPlace ap;
		if(!stricmp(str, "ROOM_FILLER")) {
			ap.type = ROOM_FILLER;
		} else if(!stricmp(str, "ROOM_SHRINE")) {
			ap.type = ROOM_SHRINE;
		} else if(!stricmp(str, "ROOM_ENTRY")) {
			ap.type = ROOM_ENTRY;
		} else if(!stricmp(str, "ROOM_EXIT")) {
			ap.type = ROOM_EXIT;
		} else if(!strncmp(str, "ROOM_SPECIAL_", 13)) {
			int num = atoi(str+13);
			ap.type = (RoomType_e)(ROOM_SPECIAL_0 + num);
		}

		t->vAlwaysPlace.push_back(ap);
	}
}

void InitLevelParseFields() {
#define NAME_PARSER [](cJSON* j, void* p) -> void { MapFramework* t = (MapFramework*)p; strcpy(t->name, cJSON_ToString(j)); }
#define FIRST_PARSER [](cJSON* j, void* p) -> void { MapFramework* t = (MapFramework*)p; strcpy(t->dataPreset.preset, cJSON_ToString(j)); }
#define LEVEL_PARSER [](cJSON* j, void* p) -> void { MapFramework* t = (MapFramework*)p; t->nDungeonLevel = cJSON_ToInteger(j); }
#define VIS_PARSER(x) [](cJSON* j, void*p) -> void { MapFramework* t = (MapFramework*)p; t->sLink[x] = cJSON_ToString(j); }
#define PARM_PARSER(x) [](cJSON* j, void* p) -> void { MapFramework* t = (MapFramework*)p; t->parm[x] = cJSON_ToInteger(j); }
#define P_PARSER [](cJSON* j, void* p) -> void { MapFramework* t = (MapFramework*)p; t->bIsPreset = cJSON_ToBoolean(j); }
#define INT_PARSER(x) [](cJSON* j, void* p) -> void { MapFramework* t = (MapFramework*)p; t->##x## = cJSON_ToInteger(j); }
#define TO_PARSER [](cJSON* j, void* p) -> void { MapFramework* t = (MapFramework*)p; strcpy(t->toName, cJSON_ToString(j)); }
#define MAZE_PARSER [](cJSON* j, void* p) -> void { MapFramework* t = (MapFramework*)p; strncpy(t->mazeName, cJSON_ToString(j), sizeof(t->mazeName)); }
	levelParseFields["name"] = NAME_PARSER;
	levelParseFields["toName"] = TO_PARSER;
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
	levelParseFields["maze"] = MAZE_PARSER;
	levelParseFields["parm0"] = PARM_PARSER(0);
	levelParseFields["parm1"] = PARM_PARSER(1);
	levelParseFields["parm2"] = PARM_PARSER(2);
	levelParseFields["parm3"] = PARM_PARSER(3);
	levelParseFields["parm4"] = PARM_PARSER(4);
	levelParseFields["parm5"] = PARM_PARSER(5);
	levelParseFields["parm6"] = PARM_PARSER(6);
	levelParseFields["parm7"] = PARM_PARSER(7);
	levelParseFields["alwaysPlaceRooms"] = ParseAlwaysRooms;
#undef NAME_PARSER
#undef INT_PARSER
}

// WARP PARSING
static unordered_map<const char*, jsonParseFunc> warpParseFields;
void InitWarpParseFields() {
#define NAME_PARSER [](cJSON* j, void* p) -> void { Warp* t = (Warp*)p; strcpy(t->sName, cJSON_ToString(j)); };
#define HIGH_PARSER [](cJSON* j, void* p) -> void { Warp* t = (Warp*)p; strcpy(t->sHighMaterial, cJSON_ToString(j)); t->ptHighMaterial = trap->RegisterMaterial(t->sHighMaterial); };
#define DOWN_PARSER [](cJSON* j, void* p) -> void { Warp* t = (Warp*)p; strcpy(t->sDownMaterial, cJSON_ToString(j)); t->ptDownMaterial = trap->RegisterMaterial(t->sDownMaterial); };
#define INT_PARSER(x) [](cJSON* j, void* p) -> void { Warp* t = (Warp*)p; t->##x## = cJSON_ToInteger(j); }
	warpParseFields["name"] = NAME_PARSER;
	warpParseFields["baseMaterial"] = DOWN_PARSER;
	warpParseFields["highMaterial"] = HIGH_PARSER;
	warpParseFields["x"] = INT_PARSER(x);
	warpParseFields["y"] = INT_PARSER(y);
	warpParseFields["w"] = INT_PARSER(w);
	warpParseFields["h"] = INT_PARSER(h);
#undef INT_PARSER
#undef NAME_PARSER
}

// MAZE PARSING
static unordered_map<const char*, jsonParseFunc> mazeParseFields;

void Maze_AlgorithmParser(cJSON* j, void* p) {
	MazeFramework* t = (MazeFramework*)p;
	string str = cJSON_ToString(j);

	if(str == "ALGO_DRUNKEN_STAGGER") {
		t->algorithm = ALGO_DRUNKEN_STAGGER;
	} else if(str == "ALGO_SATURATION") {
		t->algorithm = ALGO_SATURATION;
	}
}
void InitMazeParseFields() {
#define INT_PARSER(x) [](cJSON* j, void* p) -> void { MazeFramework* t = (MazeFramework*)p; t->##x## = cJSON_ToInteger(j); }
#define SET_PARSER [](cJSON* j, void* p) -> void { MazeFramework* t = (MazeFramework*)p; strncpy(t->tileSet, cJSON_ToString(j), sizeof(t->tileSet)); }
#define NAME_PARSER [](cJSON* j, void* p) -> void { MazeFramework* t = (MazeFramework*)p; strncpy(t->name, cJSON_ToString(j), sizeof(t->name)); }
	mazeParseFields["name"] = NAME_PARSER;
	mazeParseFields["mazeSet"] = SET_PARSER;
	mazeParseFields["roomSizeX"] = INT_PARSER(roomSizeX);
	mazeParseFields["roomSizeY"] = INT_PARSER(roomSizeY);
	mazeParseFields["algorithm"] = Maze_AlgorithmParser;
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
#define WARP_PARSER [](cJSON* j, void* p) -> void { Tile* t = (Tile*)p; strcpy(t->sWarp, cJSON_ToString(j)); t->bWarpTile = true; }

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
	tileParseFields["material"] = MAT_PARSER; // FIXME: load this stuff dynamially!
	tileParseFields["depthoffset"] = FLOAT_PARSER(fDepthScoreOffset);
	tileParseFields["autotrans"] = AutoTransParser;
	tileParseFields["warp"] = WARP_PARSER;
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
		R_Message(PRIORITY_WARNING, "WARNING: BDF file (%s) is empty!\n", path.c_str());
		trap->CloseFile(file);
		trap->Zone_FastFree(pfd, "pfd");
		return;
	}

	// Take care of the (constant) header
	trap->ReadBinary(file, (unsigned char*)&pfd->head, sizeof(pfd->head), false);
	if(strnicmp(pfd->head.header, "DRLG.BDF", sizeof(pfd->head.header))) {
		R_Message(PRIORITY_WARNING, "WARNING: BDF file (%s) contains invalid header! (found: \"%s\", expected \"DRLG.BDF\")\n",
			path.c_str(), pfd->head.header);
		trap->CloseFile(file);
		trap->Zone_FastFree(pfd, "pfd");
		return;
	}

	if(pfd->head.version != 1) { // TODO: use proper compare
		R_Message(PRIORITY_WARNING, "WARNING: BDF file (%s) uses invalid version number! (%i)\n",
			path.c_str(), pfd->head.version);
		trap->CloseFile(file);
		trap->Zone_FastFree(pfd, "pfd");
		return;
	}

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
		R_Message(PRIORITY_WARNING, "WARNING: no entities or tiles in %s..not loading\n", path.c_str());
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
}

void MapLoader::LoadPresets(const char* path) {
	int numFiles = 0;
	char** paths = trap->ListFilesInDir(path, ".bdf", &numFiles);
	if(numFiles <= 0) {
		R_Error("Could not load presets in levels/preset (none?)");
		return;
	}

	// We use a special zone tag for presets, we should add that now
	trap->Zone_NewTag("pfd");

	// We also use a special zone tag for DRLG allocations
	trap->Zone_NewTag("roomgrid");

	for(int i = 0; i < numFiles; i++) {
		LoadPreset(paths[i]);
	}

	R_Message(PRIORITY_NOTE, "Loaded %i presets\n", numFiles);
	trap->FreeFileList(paths, numFiles);
}

MapLoader::MapLoader(const char* presetsPath, const char* tilePath) {
	// Load warps. We'll need this info for tiles.
	InitWarpParseFields();
	JSON_ParseMultifile<Warp>("levels/LevelWarp.json", warpParseFields, vWarpData);
	for(auto it = vWarpData.begin(); it != vWarpData.end(); ++it) {
		mWarpResolver[it->sName] = it;
	}

	// Load mazes
	InitMazeParseFields();
	JSON_ParseMultifile<MazeFramework>("levels/LevelMaze.json", mazeParseFields, vMazeData);
	for(auto it = vMazeData.begin(); it != vMazeData.end(); ++it) {
		mMazeResolver[it->name] = it;
	}

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
		if(it->bWarpTile) {
			// One extra step...we need to set up the linkage of the warp tile
			auto found = mWarpResolver.find(it->sWarp);
			if(found != mWarpResolver.end()) {
				it->ptiWarp = found->second;
			}
		}
		mTileResolver[it->name] = it;
	}

	// Load the Levels.json
	InitLevelParseFields();
	JSON_ParseMultifile<MapFramework>("levels/Levels.json", levelParseFields, vMapData);

	// Special: We need to copy data from the maze framework if needed
	for(auto it = vMapData.begin(); it != vMapData.end(); ++it) {
		if(!it->bIsPreset) {
			if(it->mazeName[0] != '\0') {
				auto maze = mMazeResolver.find(it->mazeName);
				if(maze == mMazeResolver.end()) {
					continue;
				}
				memcpy(&it->dataDRLG, &(*maze->second), sizeof(it->dataDRLG));
			}
		}
	}

	// Load presets.
	LoadPresets(presetsPath);

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

MazeFramework* MapLoader::FindMazeFrameworkByName(const char* name) {
	auto it = mMazeResolver.find(name);
	if(it == mMazeResolver.end()) {
		return nullptr;
	}
	return &(*(it->second));
}