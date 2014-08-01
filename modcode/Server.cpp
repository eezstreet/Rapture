#include "Server.h"

Server* ptServer = nullptr;
Client* ptClient = nullptr;

Server::Server() {
	ptTheClient = new Client();
	ptDungeonManager = new DungeonManager();
	ptQuestManager = new QuestManager();
	ptClient = ptTheClient;
}

Server::~Server() {
	delete ptDungeonManager;
	delete ptQuestManager;
	delete ptTheClient;
}

Client* Server::GetClient() {
	return ptTheClient;
}