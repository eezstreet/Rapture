#include "g_local.h"

gameImports_s* trap;

static gameExports_s exportFns;
static bool bLoadingScreen = false;
static void* ptLoadScreenImage = NULL;

void Game_Init() {
	trap->printf("--- Game Initialization ---\n");
	ptLoadScreenImage = trap->RegisterImage("ui/images/loading2");
	bLoadingScreen = true;
}

void Game_Shutdown() {
	trap->printf("--- Game Shutdown ---\n");
}

void Game_Frame() {
	if(bLoadingScreen) {
		trap->DrawImage(ptLoadScreenImage, 25.0f, 25.0f, 50.0f, 50.0f);
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