#include "Local.h"
#include "Server.h"
#include "SaveGame.h"

gameImports_s* trap;

static gameExports_s exportFns;
static int iLoadingScreen = 0;
static Texture* ptLoadScreenImage = nullptr;

void Game_Init() {
	trap->printf(PRIORITY_NOTE, "--- New Game ---\n");
	ptLoadScreenImage = trap->RegisterImage("ui/images/loading");
	iLoadingScreen = 1;
}

void Game_Shutdown() {
	SaveGame* ptSave = SaveGame::NewSingleplayerSave(ptServer, ptServer->GetClient());

	trap->printf(PRIORITY_NOTE, "--- Quit Game ---\n");
	trap->ShutdownMaterials();
	delete ptServer;
	delete ptSave;
}

void Game_Load() {
	trap->InitMaterials();

	trap->RegisterCvarInt("cg_drawfps", "Draw FPS ingame? (1 = FPS, 2 = MS, 3 = both)", (1 << CVAR_ARCHIVE), 0);
	trap->RegisterCvarBool("cg_drawxy", "Draw mouse X/Y coordinates?", (1 << CVAR_ARCHIVE), false);
	trap->RegisterCvarBool("cg_drawworldxy", "Draw world X/Y coordinates?", (1 << CVAR_ARCHIVE), false);

	ptServer = new Server();
	ptServer->ptDungeonManager->StartBuild("Survivor's Camp");
	ptServer->ptDungeonManager->SpawnPlayer("Survivor's Camp");

	iLoadingScreen = 0;
}

void Game_Frame() {
	if(iLoadingScreen == 1) {
		// Force it to draw the loading screen
		trap->DrawImageAspectCorrection(ptLoadScreenImage, 37.5, 37.5, 25, 25);
		iLoadingScreen++;
	}
	else if(iLoadingScreen == 2) {
		// Now do the actual loading
		Game_Load();
		iLoadingScreen = 0;
	}
	else {
		// In a multiplayer game, we would be looping through all 4 acts and running GetWorld()->Run,
		// but since we're only concerned about singleplayer, we're going to focus on the one that
		// our client is on

		// Preframe. We need to keep client/server synchronized
		ptClient->Preframe();
		// Server frame
		ptServer->ptDungeonManager->GetWorld(ptServer->GetClient()->ptPlayer->iAct)->Run();
		// Client frame
		ptClient->Frame();
	}
}

void Game_OnMouseUp(int x, int y) {
	if(iLoadingScreen == 0) {
		// This should be client?
		ptClient->PassMouseUp(x, y);
	}
}

void Game_OnMouseDown(int x, int y) {
	if(x >= (ptClient->screenWidth/2) - 200 && x <= (ptClient->screenWidth/2) + 200 && y >= ptClient->screenHeight - 170) {
		return;
	}
	if(iLoadingScreen == 0) {
		if(!trap->IsConsoleOpen()) {
			ptClient->PassMouseDown(x, y);
		}
	}
}

void Game_OnMouseMove(int x, int y) {
	if(iLoadingScreen == 0) {
		if(!trap->IsConsoleOpen()) {
			ptClient->PassMouseMove(x, y);
		}
	}
}

void Game_OnKeyPress(int x) {
}

extern "C" {
	// FIXME: not UNIX compatible!!
	__declspec(dllexport) gameExports_s* GetRefAPI(gameImports_s* import) {
		trap = import;

		exportFns.init = Game_Init;
		exportFns.shutdown = Game_Shutdown;
		exportFns.runactiveframe = Game_Frame;
		exportFns.passmouseup = Game_OnMouseUp;
		exportFns.passmousedown = Game_OnMouseDown;
		exportFns.passmousemove = Game_OnMouseMove;
		exportFns.passkeypress = Game_OnKeyPress;
		return &exportFns;
	}
};