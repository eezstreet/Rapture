#include "Server.h"

Server* ptServer = nullptr;
Client* ptClient = nullptr;

Server::Server() {
	ptTheClient = new Client();
	ptDungeonManager = new DungeonManager();
	ptClient = ptTheClient;
}

Client* Server::GetClient() {
	return ptTheClient;
}