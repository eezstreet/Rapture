#include "drlg_local.h"
#include "../json/cJSON.h"

typedef void (*jParseFunction)(Level&, cJSON*);
static unordered_map<const char*, function<void (Level&, cJSON*)>> levelParseFuncs;

#define INTPARSER(x) levelParseFuncs["##x##"] = [](Level& lvl, cJSON* json) -> void { lvl.##x = cJSON_ToInteger(json); }
void MapManager::InitMapParsers() {
	INTPARSER(nLevelId);
	INTPARSER(nAct);
	INTPARSER(mTileMask);
}

void ProcessLevel(Level& out, const string& text) {
	const char *cText = text.c_str();
	char error[1024];
	cJSON* json = cJSON_ParsePooled(cText, error, 1024);
	if(!json)
		return;
	for(cJSON* x = cJSON_GetFirstItem(json); x; x = cJSON_GetNextItem(json)) {
		auto it = levelParseFuncs.find(cJSON_GetItemKey(x));
		if(it != levelParseFuncs.end())
			it->second(out, x);
	}
}

void MapManager::LoadIndividualLevel(const string& levelPath) {
	// Load the file from FS. We don't need to check that the file is NULL because we are grabbing this from the FS list.
	File* f = File::Open(levelPath, "r");
	string& text = f->ReadPlaintext();
	// Load the map's info from JSON
	Level lvl;
	ProcessLevel(lvl, text);
}

void MapManager::LoadLevelData() {
	vector<string> s;
	string dir = "tiles/act" + (nAct+1);
	dir += "/data";
	FS::FileSystem::ListFiles(dir, s, ".json");
	for(auto it = s.begin(); it != s.end(); ++it)
		LoadIndividualLevel(*it);
}

MapManager::MapManager(unsigned short act /* = 0 */, unsigned short levelId /* = 0 */) :
	nAct(act),
	nCurrentLevel(levelId) {
	// Load all the files in this act's data
	InitMapParsers();
	LoadLevelData();
}

MapManager::~MapManager() {
}

MapManager::MapManager(MapManager& other) : nAct(other.nAct), nCurrentLevel(other.nCurrentLevel) {
}

MapManager& MapManager::operator=(MapManager& other) {
	nAct = other.nAct;
	nCurrentLevel = other.nCurrentLevel;
	return *this;
}