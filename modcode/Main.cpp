#include "Server.h"
#include "Client.h"
#include "Savegame.h"
#include "Simulation.h"

gameImports_s* trap = nullptr;

void Game_InitServer(const char* szSavePath) {
	Simulation::Initialize(szSavePath);
	Server::Initialize();
}

void Game_InitClient(const char* szSavePath) {
	Simulation::Initialize(szSavePath);
	Client::Initialize();
}

void Game_KeyPress(int x) {

}

void Game_MouseDown(int x, int y) {

}

void Game_MouseMove(int x, int y) {

}

void Game_MouseUp(int x, int y) {

}

void Game_Shutdown() {
	Simulation::Shutdown();
	Server::Shutdown();
	Client::Shutdown();
}

static gameExports_s exports;
extern "C" {
	__declspec(dllexport) gameExports_s* GetRefAPI(gameImports_s* imp) {
		trap = imp;

		exports.startclientfromsave = Game_InitClient;
		exports.startserverfromsave = Game_InitServer;
		exports.runserverframe = Server::Frame;
		exports.runclientframe = Client::Frame;
		exports.serializepackettoclient = Server::SerializePacket;
		exports.deserializepacketfromclient = Server::DeserializePacket;
		exports.serializepackettoserver = Client::SerializePacket;
		exports.deserializepacketfromserver = Client::DeserializePacket;
		exports.saveandexit = Game_Shutdown;

		exports.passkeypress = Game_KeyPress;
		exports.passmousedown = Game_MouseDown;
		exports.passmousemove = Game_MouseMove;
		exports.passmouseup = Game_MouseUp;
		return &exports;
	}
}