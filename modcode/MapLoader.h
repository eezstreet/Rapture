#pragma once
#include "MapFramework.h"
#include "Tile.h"
#include "PresetFileData.h"
#include "Warp.h"
#include "Local.h"

/////////////////////
//
//  MapLoader
//	The MapLoader is responsible for loading
//	tiles, PFDs, etc. and keeping them for use
//	in the dungeon manager.
//
/////////////////////

class MapLoader {
private:
	// Tile/map loading
	vector<Tile> vTiles;
	vector<MapFramework> vMapData;
	vector<Warp> vWarpData;
	unordered_map<string, vector<Tile>::iterator> mTileResolver;
	unordered_map<string, vector<Map>::iterator> mMapResolver;
	unordered_map<string, vector<Warp>::iterator> mWarpResolver;

	void LoadTile(void* file, const char* filename);
	
	// Preset loading
	unordered_map<string, PresetFileData*> mPfd;
	void LoadPresets(const char* path);
	void LoadPreset(const string& path);
public:
	MapLoader(const char* presetsPath, const char* tilePath);
	~MapLoader();

	// Map loading
	MapFramework* FindMapFrameworkByName(const char* name);

	// Tile loading
	Tile* FindTileByName(const string& str);

	// Preset loading
	PresetFileData* FindPresetByName(const string& str);
};