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

void InitTileParseFields() {
#define SUBTILE_PARSER(x) [](cJSON* j, void* p) -> void { ParseSubtile(j, p, x); }
#define INT_PARSER(x) [](cJSON* j, void* p) -> void { Tile* t = (Tile*)p; t->##x## = cJSON_ToInteger(j); }
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
}

// LEVEL PARSING
static unordered_map<const char*, jsonParseFunc> levelParseFields;
void InitLevelParseFields() {
#define NAME_PARSER [](cJSON* j, void* p) -> void { MapFramework* t = (MapFramework*)p; strcpy(t->name, cJSON_ToString(j)); }
#define FIRST_PARSER [](cJSON* j, void* p) -> void { MapFramework* t = (MapFramework*)p; strcpy(t->entryPreset, cJSON_ToString(j)); }
#define LEVEL_PARSER [](cJSON* j, void* p) -> void { MapFramework* t = (MapFramework*)p; t->nDungeonLevel = cJSON_ToInteger(j); }
#define VIS_PARSER(x) [](cJSON* j, void*p) -> void { MapFramework* t = (MapFramework*)p; t->iLink[x] = cJSON_ToInteger(j); }
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
	levelParseFields["vis8"] = VIS_PARSER(8);
	levelParseFields["vis9"] = VIS_PARSER(9);
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

void BuildMapFromFramework(const MapFramework& crtMapFramework, Map& rtMap) {
	// With a preset map, it's easy. Just add the entry preset to the map and it's good to go.
	if(crtMapFramework.bIsPreset) {
		rtMap.bIsPreset = true;
		MP_AddToMap(crtMapFramework.entryPreset, rtMap);
		return;
	}

}

MapLoader::MapLoader(const string& presetsPath, const string& tilePath) {
	// Load the tiles.
	InitTileParseFields();
	vector<string> paths;
	int numFiles = trap->ListFilesInDir(tilePath.c_str(), paths, ".json");
	if(numFiles <= 0) {
		R_Error("MapLoader could not load tiles (missing?)");
	}
	for(auto it = paths.begin(); it != paths.end(); ++it) {
		Tile t;
		JSON_ParseFile((char*)it->c_str(), tileParseFields, &t);
		vTiles.push_back(t);
		mTileResolver.insert(make_pair(t.name, vTiles.end()-1));
	}

	// Load the Levels.json
	InitLevelParseFields();
	JSON_ParseMultifile<MapFramework>("levels/Levels.json", levelParseFields, vMapData);

	// Load the presets.
	MP_LoadTilePresets();

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
	Map survivorCamp;
	BuildMapFromFramework(*x, survivorCamp);
	world.InsertInto(&survivorCamp);
	R_Printf("done.");
}

void ShutdownLevels() {
	delete maps;
}

void DrawBackground() {
	auto tiles = world.qtTileTree->NodesAt(0, 0);
	for(auto it = tiles.begin(); it != tiles.end(); ++it) {
		TileNode* tile = *it;
		
	}
}