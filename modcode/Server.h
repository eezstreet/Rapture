#pragma once
#include "Local.h"
#include "DungeonManager.h"
#include "QuestManager.h"

struct Server {
public:
	DungeonManager* ptDungeonManager;
	QuestManager* ptQuestManager;

	Client* GetClient();
	Server();
	~Server();
private:
	Client* ptTheClient; // NETWORKING FIXME - we don't store raw pointers to clients over the internet!
};

extern Server* ptServer;