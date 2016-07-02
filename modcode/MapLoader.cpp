#include "MapLoader.h"
#include "Json.h"

MapLoader::MapLoader(const char* presetsPath, const char* tilePath) {
}

MapLoader::~MapLoader() {
}

Tile* MapLoader::FindTileByName(const string& str) {
	return nullptr;
}

MapFramework* MapLoader::FindMapFrameworkByName(const char* name) {
	return nullptr;
}

PresetFileData* MapLoader::FindPresetByName(const string& str) {
	return nullptr;
}

MazeFramework* MapLoader::FindMazeFrameworkByName(const char* name) {
	return nullptr;
}