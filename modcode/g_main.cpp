#include "g_local.h"

gameImports_s* trap;

static gameExports_s exportFns;

void Game_Init() {
	trap->printf("--- Game Initialization ---\n");
}

void Game_Shutdown() {
	trap->printf("--- Game Shutdown ---\n");
}

extern "C" {
	__declspec(dllexport) gameExports_s* GetRefAPI(gameImports_s* import) {
		trap = import;

		exportFns.init = Game_Init;
		exportFns.shutdown = Game_Shutdown;
		return &exportFns;
	}
};