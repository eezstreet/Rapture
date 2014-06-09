#include "g_local.h"

gameImports_s* trap;

static gameExports_s exportFns;
static int iLoadingScreen = 0;
static Image* ptLoadScreenImage = NULL;

void Game_Init() {
	trap->printf("--- New Game ---\n");
	InitFPS();
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

	trap->RegisterCvarInt("cg_drawfps", "Draw FPS ingame? (1 = FPS, 2 = MS, 3 = both)", 1, 0);
	trap->RegisterCvarBool("cg_drawxy", "Draw mouse X/Y coordinates?", 1, false);
	trap->RegisterCvarBool("cg_drawworldxy", "Draw world X/Y coordinates?", 1, false);

	RegisterMedia();

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
		FPSFrame();
		world.Run();
		world.Render();
		DrawViewportInfo();
		world.UpdateEntities();
	}
}

void Game_OnMouseUp(int x, int y) {
	Player* ply = world.GetFirstPlayer();
	if(ply != NULL) {
		ply->MouseUpEvent(x, y);
	}
}

void Game_OnMouseDown(int x, int y) {
	Player* ply = world.GetFirstPlayer();
	if(ply != NULL) {
		ply->MouseDownEvent(x, y);
	}
}

void Game_OnMouseMove(int x, int y) {
	currentMouseX = x;
	currentMouseY = y;

	Player* ply = world.GetFirstPlayer();
	if(ply != NULL) {
		ply->MouseMoveEvent(x, y);
	}
}

extern "C" {
	__declspec(dllexport) gameExports_s* GetRefAPI(gameImports_s* import) {
		trap = import;

		exportFns.init = Game_Init;
		exportFns.shutdown = Game_Shutdown;
		exportFns.runactiveframe = Game_Frame;
		exportFns.passmouseup = Game_OnMouseUp;
		exportFns.passmousedown = Game_OnMouseDown;
		exportFns.passmousemove = Game_OnMouseMove;
		return &exportFns;
	}
};