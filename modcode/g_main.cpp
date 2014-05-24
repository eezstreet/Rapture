#include "g_local.h"

gameImports_s* trap;

static gameExports_s exportFns;
static int iLoadingScreen = 0;
static void* ptLoadScreenImage = NULL;

void Game_Init() {
	trap->printf("--- New Game ---\n");
	ptLoadScreenImage = trap->RegisterImage("ui/images/loading");
	iLoadingScreen = 1;
}

void Game_Shutdown() {
	trap->printf("--- Quit Game ---\n");
	ShutdownLevels();
	trap->ShutdownMaterials();
}

void Game_Load() {
	trap->InitMaterials();
	InitLevels();
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
}

extern "C" {
	__declspec(dllexport) gameExports_s* GetRefAPI(gameImports_s* import) {
		trap = import;

		exportFns.init = Game_Init;
		exportFns.shutdown = Game_Shutdown;
		exportFns.runactiveframe = Game_Frame;
		return &exportFns;
	}
};