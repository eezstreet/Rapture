#pragma once
#include "Server.h"

class SaveGame {
	SaveGame(const Server* ptServer, const Client* ptClient, const Player* ptPlayer);

public:
	static SaveGame* NewSingleplayerSave(const Server* ptServer, const Client* ptClient);
	static SaveGame* LoadSingleplayerSave(const string& sSaveName);
};