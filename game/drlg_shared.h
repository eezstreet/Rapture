#pragma once
#include "sys_local.h"
#include "../json/cJSON.h"

class MapManager {
public:
	struct Level {
		unsigned int nLevelId;
		unsigned char nAct;
		unsigned int mTileMask;
		string sLevelType;
		string sIdentifier;
	};

	typedef void (*jParseFunction)(Level&, cJSON*);
	static unordered_map<const char*, function<void (Level&, cJSON*)>> levelParseFuncs;

	MapManager(unsigned short act = 0, unsigned short levelId = 0);
	~MapManager();
	MapManager& operator= (MapManager& other);
	MapManager(MapManager& other);
private:
	unordered_map<string, Level*> levels;

	unsigned short nAct;
	unsigned short nCurrentLevel;

	void LoadLevelData();
	void LoadIndividualLevel(const string& levelPath);


	void InitMapParsers();
};