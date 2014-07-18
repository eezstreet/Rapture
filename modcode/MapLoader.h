#pragma once
#include "MapFramework.h"
#include "MazeFramework.h"
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
	vector<MazeFramework> vMazeData;
	unordered_map<string, vector<Tile>::iterator> mTileResolver;
	unordered_map<string, vector<Map>::iterator> mMapResolver;
	unordered_map<string, vector<Warp>::iterator> mWarpResolver;
	unordered_map<string, vector<MazeFramework>::iterator> mMazeResolver;

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

	// Maze loading
	MazeFramework* FindMazeFrameworkByName(const char* name);
};