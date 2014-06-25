#include "g_json.h"

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

// LEVEL PARSING
static unordered_map<const char*, jsonParseFunc> levelParseFields;
void InitLevelParseFields() {
#define NAME_PARSER [](cJSON* j, void* p) -> void { MapFramework* t = (MapFramework*)p; strcpy(t->name, cJSON_ToString(j)); }
#define FIRST_PARSER [](cJSON* j, void* p) -> void { MapFramework* t = (MapFramework*)p; strcpy(t->entryPreset, cJSON_ToString(j)); }
#define LEVEL_PARSER [](cJSON* j, void* p) -> void { MapFramework* t = (MapFramework*)p; t->nDungeonLevel = cJSON_ToInteger(j); }
#define VIS_PARSER(x) [](cJSON* j, void*p) -> void { MapFramework* t = (MapFramework*)p; t->sLink[x] = cJSON_ToString(j); }
#define P_PARSER [](cJSON* j, void* p) -> void { MapFramework* t = (MapFramework*)p; t->bIsPreset = cJSON_ToBoolean(j); }
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
}

Worldspace world;
vector<MapFramework> vMapData;

MapFramework* FindMapFrameworkByName(const char* name) {
	for(auto it = vMapData.begin(); it != vMapData.end(); ++it) {
		if(!stricmp(it->name, name)) {
			return &(*it);
		}
	}
	return NULL;
}

unordered_map<string, bool> mHaveWeAlreadyBuilt;
static vector<Map> levels;

void BuildMapFromFramework(const MapFramework& crtMapFramework, Map& rtMap) {
	if(mHaveWeAlreadyBuilt.find(crtMapFramework.name) != mHaveWeAlreadyBuilt.end()) {
		return; // We've already built this map. Don't build it again.
	}
	mHaveWeAlreadyBuilt[crtMapFramework.name] = true;

	// With a preset map, it's easy. Just add the entry preset to the map and it's good to go.
	if(crtMapFramework.bIsPreset) {
		rtMap.bIsPreset = true;
		MP_AddToMap(crtMapFramework.entryPreset, rtMap);
	} else {
		// DRLG generation
	}

	for(int i = 0; i < 8; i++) {
		// Create any maps which we link to
		if(crtMapFramework.sLink[i].length() > 0) {
			MapFramework* ptFramework = FindMapFrameworkByName(crtMapFramework.sLink[i].c_str());
			if(ptFramework != NULL) {
				Map tehMap;
				BuildMapFromFramework(*ptFramework, tehMap);
				levels.push_back(tehMap);
			}
		}
	}
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

	// Load the presets.
	MP_LoadTilePresets();

	trap->FreeFileList(paths, numFiles);
}

MapLoader::~MapLoader() {
	mTileResolver.clear();
	vTiles.clear();
	tileParseFields.clear();
}

MapLoader* maps;
void InitLevels() {
	maps = new MapLoader("levels/preset", "levels/tiles");

	// Finally, start loading the maps which are important.
	/*** TEST ***/
	// Savegame data isn't done yet, so let's just hardcode the Survivor's Camp as the starting area
	R_Printf("building world...");
	MapFramework* x = FindMapFrameworkByName("Survivor's Camp");
	Player* play = new Player(1, 3);
	Map survivorCamp;
	BuildMapFromFramework(*x, survivorCamp);
	levels.push_back(survivorCamp);
	for(auto it = levels.begin(); it != levels.end(); ++it) {
		world.InsertInto(&(*it));
	}
	world.SpawnEntity(play, true, true, true);
	R_Printf("done.\n");
}

void ShutdownLevels() {
	delete maps;
}