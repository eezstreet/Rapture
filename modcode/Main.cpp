#include "Server.h"

gameImports_s* trap = nullptr;

void Game_InitServer(const char* szSavePath) {

}

void Game_InitClient(const char* szSavePath) {

}

void Game_KeyPress(int x) {

}

void Game_MouseDown(int x, int y) {

}

void Game_MouseMove(int x, int y) {

}

void Game_MouseUp(int x, int y) {

}

void Game_ServerFrame() {

}

void Game_ClientFrame() {

}

void Game_Shutdown() {

}

bool Game_ServerPacket(Packet* pPacket) {
	return true;
}

bool Game_ClientPacket(Packet* pPacket) {
	return true;
}

static gameExports_s exports;
extern "C" {
	__declspec(dllexport) gameExports_s* GetRefAPI(gameImports_s* imp) {
		trap = imp;

		exports.startclientfromsave = Game_InitClient;
		exports.startserverfromsave = Game_InitServer;
		exports.runserverframe = Game_ServerFrame;
		exports.runclientframe = Game_ClientFrame;
		exports.passkeypress = Game_KeyPress;
		exports.passmousedown = Game_MouseDown;
		exports.passmousemove = Game_MouseMove;
		exports.passmouseup = Game_MouseUp;
		exports.saveandexit = Game_Shutdown;
		exports.interpretPacketFromClient = Game_ClientPacket;
		exports.interpretPacketFromServer = Game_ServerPacket;
		return &exports;
	}
}