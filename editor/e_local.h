// Header for ingame editor module
#include <sys_shared.h>

extern gameImports_s* trap;
#define R_Printf trap->printf
#define R_Error trap->error

// e_fps.cpp
void InitFPS();
void FPSFrame();
float GetGameFPS();

// e_display.cpp
void InitDisplay();
void DisplayData();

enum EditorMode_e {
	EMODE_TILES,
	EMODE_ENTITIES
};

extern EditorMode_e eCurMode;