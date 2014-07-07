#pragma once
#include "Worldspace.h"
#include "MapFramework.h"
#include "Entity.h"

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
	void MovePlayerToVis(int iAct, int iPlayerNum, int iNextVis);

	Map* FindProperMap(int iAct, float x, float y);
	Map* FindProperMap(int iAct, int x, int y);
	string FindNextMapStr(int iAct, int iPlayerNum, int nextVis);

	vector<Entity*> GetEntsAt(float x, float y, int act);
};
extern DungeonManager* ptDungeonManager;