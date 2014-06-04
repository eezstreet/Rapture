#include "e_local.h"

static void* ptSegoe;

#define PROGRAM_NAME "RaptureEdit "
#define PROGRAM_VERSION "0.0.0"

static void DisplayDataTop() {
	stringstream ss;
	ss << PROGRAM_NAME PROGRAM_VERSION;
	ss << "      ";
	if(eCurMode == EMODE_TILES) {
		ss << "TILE MODE";
	} else {
		ss << "ENTS MODE";
	}
	ss << "      ";

	bool bDrawFPS;
	trap->CvarBoolVal("cg_drawfps", &bDrawFPS);
	if(bDrawFPS) {
		ss << "FPS: ";
		ss << GetGameFPS();
		ss << "      ";
	}
	trap->RenderTextShaded(ptSegoe, ss.str().c_str(), 0, 0, 0, 255, 255, 255);
}

void DisplayData() {
	DisplayDataTop();
}

void InitDisplay() {
	ptSegoe = trap->RegisterFont("fonts/segoeui.ttf", 14);
}