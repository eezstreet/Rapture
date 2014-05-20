#pragma once
#include <sys_shared.h>
#include "../json/cJSON.h"

extern gameImports_s* trap;
#define R_Printf trap->printf
#define R_Error trap->error

// JSON parser structs
typedef function<void (cJSON*, void*)> jsonParseFunc;
bool JSON_ParseFieldSet(cJSON* root, const unordered_map<const char*, jsonParseFunc>& parsers, cJSON* rootNode, void* output);
bool JSON_ParseFile(char *filename, const unordered_map<const char*, jsonParseFunc>& parsers, void* output);

// Generic Cache class
template<class T>
class Cache {
private:
	size_t elementsAllocated;
	size_t elementsUsed;
	T* cache;

	vector<int> ivInUse;
public:
	void Flush(bool bFlushInUseOnly = false);
	void Insert(T* element, bool bInUse = false);
	T* GetElement(const int at) { if(at >= elementsUsed) return NULL; return &cache[at] }
	Cache(const string& zoneTag);
};

// Shader element types
enum ShaderMapType_e {
	SMT_DIFFUSE,
	SMT_SPECULAR,
	SMT_NORMAL
};

////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  MAP SYSTEM
//
////////////////////////////////////////////////////////////////////////////////////////////////////////
#define MAX_MAP_LINKS	8
typedef unsigned int coords_t[2];

// Each tile is placed onto a map. The tiles contain 16 subtiles
struct Tile {
	// Basic info
	char name[64];

	// Subtile flags
	unsigned short lowmask;		// Mask of subtiles which block the player (but can be jumped over)
	unsigned short highmask;	// Mask of subtiles which block the player (and cannot be jumped over)
	unsigned short lightmask;	// Mask of subtiles which block light
	unsigned short vismask;		// Mask of subtiles which block visibility

	// Material
	void* materialHandle;		// Pointer to material in memory
	bool bResourcesLoaded;		// Have the resources (images/shaders) been loaded?

	Tile() {
		name[0] = lowmask = highmask = lightmask = vismask = '\0';
		materialHandle = NULL;
		bResourcesLoaded = false;
	}
};

// MapDatum provides a framework for the Map to be built.
struct MapDatum {
	char name[64];
	bool bIsPreset;
	int nDungeonLevel;
	int iLink[MAX_MAP_LINKS];
};
extern vector<MapDatum> vMapData;

// Each map belongs to the worldspace, consuming a portion of it.
struct Map {
	struct MapData {
		virtual void Load() = 0;
	};

	struct PresetData : public MapData {
		map<coords_t, Tile*> placedTiles;

		void Load();
	};

	struct DLRGData : public MapData {
		bool bIsGenerated;		// Has DLRG data been generated? (Y/N)

		void Load();
	};

	union {
		PresetData* ptPData;
		DLRGData* ptDData;
	};
	Map* links[MAX_MAP_LINKS];
	bool bIsPreset;					// Is this map a preset level? (Y/N)
	bool bIsLoaded;					// Are resources loaded? (Y/N)
	const unsigned int iHiSeed;		// High seed
	const unsigned int iLoSeed;		// Low seed
	const unsigned int iLIndex;		// LevelData index
	const char nDLevel;				// Dungeon level

	MapData* GetMapData() {
		if(bIsPreset) {
			return ptPData;
		} else {
			return ptDData;
		}
	}
};

struct PresetFileData {
	struct PFDHeader {
		char name[64];
		unsigned short version;
		unsigned int sizeX;
		unsigned int sizeY;
		unsigned int numTileBlocks;
	};
	PFDHeader head;
	struct LoadedTile {
		char name[64];
		unsigned int pos[1024][2];
	};
	LoadedTile* tileBlocks;
};

// The worldspace resembles an entire act's maps, all crammed into a gigantic grid
class Worldspace {
	map<coords_t, Tile*> tiles;

	const Map* firstLink;
};

extern Worldspace world;

// The map loader gets initialized by the game first, and then the worldspace grabs map data from it
class MapLoader {
private:
	vector<Tile> vTiles;
	unordered_map<string, vector<Tile>::iterator> mTileResolver;
	vector<Map> vMaps;
	unordered_map<string, vector<Map>::iterator> mMapResolver;

	void LoadTile(void* file, const char* filename);
public:
	MapLoader(const string& presetsPath, const string& tilePath);
	void BeginLoad(unsigned int levelId);
	~MapLoader();
};

void InitLevels();
void ShutdownLevels();