#include "drlg_local.h"

void MapManager::LoadLevelData() {
	vector<string> s;
	string dir = "tiles/act" + (act+1);
	dir += "/data";
	FS::FileSystem::ListFiles(dir, s, ".json");
	for(auto it = s.begin(); it != s.end(); ++it)
		LoadIndividualLevel(*it);
}

MapManager::MapManager(unsigned short act /* = 0 */, unsigned short levelId /* = 0 */) :
	nAct(act),
	nCurrentLevel(levelId) {
	// Load all the files in this act's data
	vector<string> s;
	string dir = "tiles/act" + (act+1);
	dir += "/data";
	FS::FileSystem::ListFiles(dir, s, ".
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