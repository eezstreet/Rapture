#pragma once
#include "sys_local.h"

class MapManager {
private:
	unsigned short nAct;
	unsigned short nCurrentLevel;

	void LoadLevelData();
	void LoadIndividualLevel(const string& levelPath);

	void InitMapParsers();
public:
	MapManager(unsigned short act = 0, unsigned short levelId = 0);
	~MapManager();
	MapManager& operator= (MapManager& other);
	MapManager(MapManager& other);
};