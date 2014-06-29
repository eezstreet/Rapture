#pragma once
#include <sys_shared.h>
#include "../json/cJSON.h"
#include "QuadTree.h"

extern gameImports_s* trap;
#define R_Printf trap->printf
#define R_Error trap->error

#define MAX_HANDLE_STRING	64

// cg
void RegisterMedia();
void DrawViewportInfo();

extern int currentMouseX;
extern int currentMouseY;

// cg_fps.cpp
void InitFPS();
void FPSFrame();
float GetGameFPS();
unsigned int GetGameFrametime();

// cg_hud.cpp
void InitHUD();
void ShutdownHUD();
void HUD_EnterArea(const char* areaName);
void HUD_DrawLabel(const char* labelText);
void HUD_HideLabels();

// g_shared.cpp
void eswap(unsigned short &x);
void eswap(unsigned int &x);
string genuuid();


// JSON parser structs
typedef function<void (cJSON*, void*)> jsonParseFunc;
bool JSON_ParseFieldSet(cJSON* root, const unordered_map<const char*, jsonParseFunc>& parsers, void* output);
bool JSON_ParseFile(char *filename, const unordered_map<const char*, jsonParseFunc>& parsers, void* output);

////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  MAP SYSTEM
//
////////////////////////////////////////////////////////////////////////////////////////////////////////
#define MAX_MAP_LINKS	8
typedef signed int coords_t[2];

/////////////////////
//
//  Tiles
//
/////////////////////

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

struct Tile {
	// Basic info
	char name[MAX_HANDLE_STRING];

	// Subtile flags
	unsigned short lowmask;		// Mask of subtiles which block the player (but can be jumped over)
	unsigned short highmask;	// Mask of subtiles which block the player (and cannot be jumped over)
	unsigned short lightmask;	// Mask of subtiles which block light
	unsigned short vismask;		// Mask of subtiles which block visibility

	// Material
	Material* materialHandle;	// Pointer to material in memory
	bool bResourcesLoaded;		// Have the resources (images/shaders) been loaded?

	// Depth
	bool bAutoTrans;
	int iAutoTransX, iAutoTransY, iAutoTransW, iAutoTransH;
	float fDepthScoreOffset;

	Tile() {
		name[0] = '\0';
		lowmask = highmask = lightmask = vismask = 0;
		fDepthScoreOffset = 0.0f;
		iAutoTransX = iAutoTransY = iAutoTransW = iAutoTransH = 0;
		materialHandle = NULL;
		bResourcesLoaded = bAutoTrans = false;
	}
};

class TileNode : public QTreeNode<unsigned int> {
public:
	Tile* ptTile;
	tileRenderType_e rt;

	TileNode() {
		w = h = 1;
	}
};


/////////////////////
//
//  Entities
//	An entity is an object in the
//	world, such as a monster or a player.
//	These entities may also be objects
//	without AI, such as chests.
//
/////////////////////

class Entity : public QTreeNode<float> {
protected:
	string uuid;
	int spawnflags;
public:
	virtual void render() = 0;
	virtual void think() = 0;
	virtual void spawn() = 0;
	unsigned int iAct;

	bool bShouldWeRender;
	bool bShouldWeThink;
	bool bShouldWeCollide;

	int GetSpawnflags() { return spawnflags; }
	string classname;
	QuadTree<Entity, float>* ptContainingTree;
friend struct Worldspace;
};

// Actors have visible representations, whereas regular entities do not.
class Actor : public Entity {
public:
	float GetPreviousX() const { return pX; }
	float GetPreviousY() const { return pY; }
protected:
	Material* materialHandle;
	float pX, pY;
friend struct Worldspace;
};

// Players are...players.
class Player : public Actor {
	unsigned char playerNum;
	void MoveToScreenspace(int sx, int sy, bool bStopAtDestination);
public:
	virtual void render();
	virtual void think();
	virtual void spawn();
	Player(float x, float y);

	void MouseUpEvent(int sX, int sY);
	void MouseDownEvent(int sX, int sY);
	void MouseMoveEvent(int sX, int sY);

	static Entity* spawnme(float x, float y, int spawnflags, int act);
};

// Other entities..
class info_player_start : public Entity {
public:
	enum {
		SPAWNFLAG_VIS0 = 1,
		SPAWNFLAG_VIS1 = 2,
		SPAWNFLAG_VIS2 = 4,
		SPAWNFLAG_VIS3 = 8,
		SPAWNFLAG_VIS4 = 16,
		SPAWNFLAG_VIS5 = 32,
		SPAWNFLAG_VIS6 = 64,
		SPAWNFLAG_VIS7 = 128,
		SPAWNFLAG_PORTAL = 256,
		SPAWNFLAG_PLAYSPAWN = 512
	};

	virtual void render() { }
	virtual void think();
	virtual void spawn();

	static Entity* spawnme(float x, float y, int spawnflags, int act);
};

/////////////////////
//
//  MapFramework
//	Each map has its own MapFramework.
//	These serve as the ground rules for
//	the map to be generated
//
/////////////////////

struct MapFramework {
	char name[MAX_HANDLE_STRING];
	bool bIsPreset;
	int nDungeonLevel;
	string sLink[MAX_MAP_LINKS];

	char entryPreset[MAX_HANDLE_STRING];
	
	int iWorldspaceX;
	int iWorldspaceY;
	int iSizeX;
	int iSizeY;
	int iAct;
};


/////////////////////
//
//  Map
//	Each level is a "map". They
//	can be either randomly generated
//	or entirely preset, based on their
//	framework.
//
/////////////////////

struct Map : public QTreeNode<int> {
	char name[MAX_HANDLE_STRING];
	bool bIsPreset;					// Is this map a preset level? (Y/N)

	int iAct;

	QuadTree<TileNode, int> qtTileTree;
	QuadTree<Entity, float> qtEntTree;

	Map(int _x, int _y, int _w, int _h, int act, int depth) :
	qtTileTree(QuadTree<TileNode, int>(_x, _y, _w, _h, 0, depth, NULL)),
	qtEntTree(QuadTree<Entity, float>((float)_x, (float)_y, (float)_w, (float)_h, 0, depth+1, NULL)) {
		x = _x;
		y = _y;
		w = _w;
		h = _h;
		iAct = act;
	}

	// Just to shut the compiler up:
	Map() :
	qtTileTree(QuadTree<TileNode, int>(0, 0, 0, 0, 0, 0, nullptr)),
		qtEntTree(QuadTree<Entity, float>(0, 0, 0, 0, 0, 0, nullptr)) {};

	vector<Entity*> FindEntities(const string& classname);
};


/////////////////////
//
//  PFD
//	The PFDs get loaded from .bdf files
//	and these in turn get put onto the map
//	in various locations
//
/////////////////////

struct PresetFileData {
	// Header data
	struct PFDHeader {
		char header[8];				// 'DRLG.BDF'
		unsigned short version;		// 1 - current version
		char lookup[MAX_HANDLE_STRING];
		unsigned long reserved1;		// Not used.
		unsigned long reserved2;		// Not used.
		unsigned short reserved3;	// Not used.
		unsigned long sizeX;
		unsigned long sizeY;
		unsigned long numTiles;
		unsigned long numEntities;
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
	unordered_map<string, vector<Tile>::iterator> mTileResolver;
	unordered_map<string, vector<Map>::iterator> mMapResolver;

	void LoadTile(void* file, const char* filename);
	
	// Preset loading
	unordered_map<string, PresetFileData*> mPfd;
	void LoadPresets();
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


/////////////////////
//
//  Worldspace
//	The worldspace contains everything for one whole act.
//
/////////////////////

struct Worldspace {
public:
	Worldspace();
	~Worldspace();
	QuadTree<Map, int>* qtMapTree;

	void InsertInto(Map* theMap);
	void SpawnEntity(Entity* ent, bool bShouldRender, bool bShouldThink, bool bShouldCollide);
	void AddPlayer(Player* ptPlayer);
	Player* GetFirstPlayer(); // temphack

	void Run();
	void Render();

	static float WorldPlaceToScreenSpaceFX(float x, float y);
	static float WorldPlaceToScreenSpaceFY(float x, float y);
	static int WorldPlaceToScreenSpaceIX(int x, int y);
	static int WorldPlaceToScreenSpaceIY(int x, int y);
	static float ScreenSpaceToWorldPlaceX(int x, int y, Player* ptPlayer);
	static float ScreenSpaceToWorldPlaceY(int x, int y, Player* ptPlayer);

	float PlayerOffsetX(Player* ptPlayer);
	float PlayerOffsetY(Player* ptPlayer);

	void UpdateEntities();
	void ActorMoved(Actor* ptActor);
private:
	unordered_map<string, Entity*> mRenderList;
	unordered_map<string, Entity*> mThinkList;
	unordered_map<string, Entity*> mCollideList;

	vector<Player*> vPlayers;
	vector<Actor*> vActorsMoved;
};

/////////////////////
//
//	DungeonManager
//	The dungeon manager is responsible for the logic
//	behind dungeon building. This includes random
//	generation.
//
/////////////////////

#define NUM_ACTS	4
class DungeonManager {
private:
	typedef Entity* (*spawnFunc_t)(float x, float y, int spawnflags, int act);

	MapLoader* ptMapLoader;
	Worldspace wActs[NUM_ACTS];

	unordered_map<string, Map*> mHaveWeAlreadyBuilt;
	vector<Map> vMaps[NUM_ACTS];

	void Construct(const MapFramework* ptFramework);
	void PresetGeneration(const MapFramework* ptFramework, Map& in);
	
	unordered_map<string, spawnFunc_t> mSpawnFuncs;
	Entity* GenerateEntity(const char* entName, float x, float y, int spawnflags, int act);
public:
	DungeonManager();
	~DungeonManager();

	void StartBuild(const string& sDungeonName);
	Worldspace* GetWorld(unsigned int iAct);

	void SpawnPlayer(const string& sDungeonName);
	Map* FindProperMap(int iAct, float x, float y);
	Map* FindProperMap(int iAct, int x, int y);
};
extern DungeonManager* ptDungeonManager;