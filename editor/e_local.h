// Header for ingame editor module
#include <sys_shared.h>

extern gameImports_s* trap;
#define R_Printf trap->printf
#define R_Error trap->error

// e_fps.cpp
void InitFPS();
void FPSFrame();
float GetGameFPS();
unsigned int GetGameFrametime();

// e_display.cpp
void InitDisplay();
void DisplayData();

enum EditorMode_e {
	EMODE_TILES,
	EMODE_ENTITIES
};

// e_interface.cpp
enum CursorPositions_e {
	CPOS_UPPERLEFT,
	CPOS_UPPER,
	CPOS_UPPERRIGHT,
	CPOS_RIGHT,
	CPOS_LOWERRIGHT,
	CPOS_LOWER,
	CPOS_LOWERLEFT,
	CPOS_LEFT,
	CPOS_NONE,
};

CursorPositions_e CursorPosForMouseXY(int iMouseX, int iMouseY);
void EditorMoveFrame();

// e_main.cpp
const string& GetFileName();


extern EditorMode_e eCurMode;