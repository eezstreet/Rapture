#pragma once
#include "Local.h"
#include "DungeonManager.h"

struct Server {
public:
	DungeonManager* ptDungeonManager;
	Client* GetClient();

	Server();
private:
	Client* ptTheClient; // NETWORKING FIXME - we don't store raw pointers to clients over the internet!
};

extern Server* ptServer;