#include "e_local.h"

static gameExports_s exportFns;
gameImports_s* trap;

EditorMode_e eCurMode;

static string fileName = "Untitled.json";
const string& GetFileName() { return fileName; }

static Menu* hotkeyDisplay;

void Editor_Load() {
	InitFPS();
	InitDisplay();

	trap->RegisterCvarInt("cg_drawfps", "Draw FPS ingame?", 0, 1);

	eCurMode = EMODE_TILES;

	hotkeyDisplay = trap->RegisterStaticMenu("ui/editorhotkeys.html");
}

void Editor_ClientInit(const char* fileName) {
	trap->printf(PRIORITY_NOTE, "--- Editor Begin ---\n");
	Editor_Load();
}

void Editor_ServerInit(const char* fileName) {
	trap->printf(PRIORITY_NOTE, "Local server not started");
}

void Editor_Shutdown() {
	trap->printf(PRIORITY_NOTE, "--- Editor Exit ---\n");
	trap->KillStaticMenu(hotkeyDisplay);
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

void Editor_OnKeyPress(int x) {
	const string& key = keycodeNames[x];

	// Selection
	if(key == "Space") {
		// Deselect
	}

	// Show/hide keys
	else if(key == "F1") {
		// Toggle Floor 1
	} else if(key == "F2") {
		// Toggle Floor 2
	} else if(key == "F3") {
		// Toggle Floor 3
	} else if(key == "F4") {
		// Toggle Wall 1
	} else if(key == "F5") {
		// Toggle Wall 2
	} else if(key == "F6") {
		// Toggle Wall 3
	} else if(key == "F7") {
		// Toggle Shadow
	} else if(key == "F8") {
		// Toggle Special
	}

	// Editor keys
	else if(key == "Tab") {
		// Change editing mode
	} else if(key == "Home") {
		// Map Properties
	} else if(key == "Page Up") {
		// Edit Levels.json
	} else if(key == "Page Down") {
		// Edit Preset
	}
}

bool Editor_InterpretPacket(Packet* packet) {
	// The Editor isn't networked
	return false;
}

bool Editor_AcceptClient(ClientPacket::ClientAttemptPacket* packet) {
	return false;
}

void Editor_ServerFrame() {
	return; // do nothing, it's totally client-side
}

void Editor_ClientFrame() {

}

extern "C" {
	__declspec(dllexport) gameExports_s* GetRefAPI(gameImports_s* import) {
		trap = import;

		exportFns.startclientfromsave = Editor_ClientInit;
		exportFns.startserverfromsave = Editor_ServerInit;
		exportFns.runserverframe = Editor_ServerFrame;
		exportFns.runclientframe = Editor_ClientFrame;
		exportFns.acceptclient = Editor_AcceptClient;
		exportFns.saveandexit = Editor_Shutdown;
		exportFns.interpretPacketFromClient = exportFns.interpretPacketFromServer = Editor_InterpretPacket;
		exportFns.passmouseup = Editor_OnMouseUp;
		exportFns.passmousedown = Editor_OnMouseDown;
		exportFns.passmousemove = Editor_OnMouseMove;
		exportFns.passkeypress = Editor_OnKeyPress;
		return &exportFns;
	}
}