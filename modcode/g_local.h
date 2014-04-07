#pragma once
#include <sys_shared.h>
#include "../json/cJSON.h"

extern gameImports_s* trap;
#define R_Printf trap->printf
#define R_Error trap->error

// JSON parser structs
typedef function<void (cJSON*, void*)> jsonParseFunc;
bool JSON_ParseFieldSet(cJSON* root, const unordered_map<const char*, jsonParseFunc>& parsers, void* output);
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

	Tile() {
		name[0] = lowmask = highmask = lightmask = vismask = '\0';
		materialHandle = NULL;
	}
};

// Each map belongs to the worldspace, consuming a portion of it.
class Map {

};

// The worldspace resembles an entire act's maps, all stacked onto a gigantic grid
class Worldspace {
};

// The map loader gets initialized by the game first, and then the worldspace grabs map data from it
class MapLoader {
private:
	vector<Tile> vTiles;
	unordered_map<string, vector<Tile>::iterator> mTileResolver;

	void LoadTile(void* file, const char* filename);
public:
	MapLoader(const string& presetsPath, const string& tilePath);
	~MapLoader();
};

void InitLevels();
void ShutdownLevels();