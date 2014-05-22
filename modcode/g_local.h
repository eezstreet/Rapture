#pragma once
#include <sys_shared.h>
#include "../json/cJSON.h"

extern gameImports_s* trap;
#define R_Printf trap->printf
#define R_Error trap->error

#define MAX_HANDLE_STRING	64

// g_shared.cpp
void eswap(unsigned short &x);
void eswap(unsigned int &x);

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
typedef signed int coords_t[2];
enum tileRenderType_e {
	TRT_FLOOR1,
	TRT_FLOOR2,
	TRT_FLOOR3,
	TRT_WALL1,
	TRT_WALL2,
	TRT_WALL3,
	TRT_SHADOW,		// FIXME: not needed?
	TRT_SPECIAL
};

// Each tile is placed onto a map. The tiles contain 16 subtiles
struct Tile {
	// Basic info
	char name[MAX_HANDLE_STRING];

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

// Provides a framework for the Map to be built.
struct MapFramework {
	char name[MAX_HANDLE_STRING];
	bool bIsPreset;
	int nDungeonLevel;
	int iLink[MAX_MAP_LINKS];

	char entryPreset[MAX_HANDLE_STRING];
};
extern vector<MapFramework> vMapData;

// Each map belongs to the worldspace, consuming a portion of it.
struct Map {
	bool bIsPreset;					// Is this map a preset level? (Y/N)
};

// g_preset.cpp
struct PresetFileData {
	// Header data
	struct PFDHeader {
		char header[8];				// 'DRLG.BDF'
		unsigned short version;		// 1 - current version
		char lookup[MAX_HANDLE_STRING];
		unsigned int reserved1;		// Not used.
		unsigned int reserved2;		// Not used.
		unsigned short reserved3;	// Not used.
		unsigned int sizeX;
		unsigned int sizeY;
		unsigned int numTiles;
		unsigned int numEntities;
	};
	PFDHeader head;

	// Tiles
#pragma pack(1)
	struct LoadedTile {
		char lookup[MAX_HANDLE_STRING];
		signed int x;
		signed int y;
		unsigned char rt;
		signed char layerOffset;
	};
	LoadedTile* tileBlocks;

	// Entities
#pragma pack(1)
	struct LoadedEntity {
		char lookup[MAX_HANDLE_STRING];
		float x;
		float y;
		unsigned int spawnflags;
		unsigned int reserved1;
	};
	LoadedEntity* entities;
};
void MP_LoadTilePresets();
PresetFileData* MP_GetPreset(const string& sName);
void MP_AddToMap(const char* pfdName, Map& map);

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