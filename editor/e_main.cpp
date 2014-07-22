#include "e_local.h"

static gameExports_s exportFns;
gameImports_s* trap;

EditorMode_e eCurMode;

static string fileName = "Untitled.bdf";
const string& GetFileName() { return fileName; }

static Menu* hotkeyDisplay;

void Editor_Load() {
	InitFPS();
	InitDisplay();

	trap->InitMaterials();

	trap->RegisterCvarInt("cg_drawfps", "Draw FPS ingame?", 0, 1);

	eCurMode = EMODE_TILES;

	hotkeyDisplay = trap->RegisterStaticMenu("ui/editorhotkeys.html");
}

void Editor_Init() {
	trap->printf(PRIORITY_NOTE, "--- Editor Begin ---\n");
	Editor_Load();
}

void Editor_Shutdown() {
	trap->printf(PRIORITY_NOTE, "--- Editor Exit ---\n");
	trap->KillStaticMenu(hotkeyDisplay);
	trap->ShutdownMaterials();
}

void Editor_Frame() {
	FPSFrame();
	DisplayData();
}

void Editor_OnMouseUp(int x, int y) {
}

void Editor_OnMouseDown(int x, int y) {
}

void Editor_OnMouseMove(int x, int y) {
}

extern "C" {
	__declspec(dllexport) gameExports_s* GetRefAPI(gameImports_s* import) {
		trap = import;

		exportFns.init = Editor_Init;
		exportFns.shutdown = Editor_Shutdown;
		exportFns.runactiveframe = Editor_Frame;
		exportFns.passmouseup = Editor_OnMouseUp;
		exportFns.passmousedown = Editor_OnMouseDown;
		exportFns.passmousemove = Editor_OnMouseMove;
		return &exportFns;
	}
}