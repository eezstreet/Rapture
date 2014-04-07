#include "g_local.h"

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
	/*tileParseFields.insert(make_pair("subtile0", SUBTILE_PARSER(0)));
	tileParseFields.insert(make_pair("subtile1", SUBTILE_PARSER(1)));
	tileParseFields.insert(make_pair("subtile2", SUBTILE_PARSER(2)));
	tileParseFields.insert(make_pair("subtile3", SUBTILE_PARSER(3)));
	tileParseFields.insert(make_pair("subtile4", SUBTILE_PARSER(4)));
	tileParseFields.insert(make_pair("subtile5", SUBTILE_PARSER(5)));
	tileParseFields.insert(make_pair("subtile6", SUBTILE_PARSER(6)));
	tileParseFields.insert(make_pair("subtile7", SUBTILE_PARSER(7)));
	tileParseFields.insert(make_pair("subtile8", SUBTILE_PARSER(8)));
	tileParseFields.insert(make_pair("subtile9", SUBTILE_PARSER(9)));
	tileParseFields.insert(make_pair("subtile10", SUBTILE_PARSER(10)));
	tileParseFields.insert(make_pair("subtile11", SUBTILE_PARSER(11)));
	tileParseFields.insert(make_pair("subtile12", SUBTILE_PARSER(12)));
	tileParseFields.insert(make_pair("subtile13", SUBTILE_PARSER(13)));
	tileParseFields.insert(make_pair("subtile14", SUBTILE_PARSER(14)));
	tileParseFields.insert(make_pair("subtile15", SUBTILE_PARSER(15)));
	tileParseFields.insert(make_pair("lowmask", INT_PARSER(lowmask)));
	tileParseFields.insert(make_pair("highmask", INT_PARSER(highmask)));
	tileParseFields.insert(make_pair("lightmask", INT_PARSER(lightmask)));
	tileParseFields.insert(make_pair("vismask", INT_PARSER(vismask)));
	tileParseFields.insert(make_pair("name", NAME_PARSER));
	tileParseFields.insert(make_pair("material", MAT_PARSER));*/
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
}

MapLoader::~MapLoader() {
	mTileResolver.clear();
	vTiles.clear();
	tileParseFields.clear();
}

MapLoader* maps;
void InitLevels() {
	maps = new MapLoader("levels/preset", "levels/tiles");
}

void ShutdownLevels() {
	delete maps;
}