#include "e_local.h"

static Font* ptSegoe;

#define PROGRAM_NAME "RaptureEdit "
#define PROGRAM_VERSION "0.0.0"

static void DisplayDataTop() {
	stringstream ss;
	ss << PROGRAM_NAME PROGRAM_VERSION;
	ss << "            ";
	if(eCurMode == EMODE_TILES) {
		ss << "TILE MODE";
	} else {
		ss << "ENTS MODE";
	}
	ss << "            ";

	int iDrawFPS;
//	trap->CvarIntVal("cg_drawfps", &iDrawFPS);
	if(iDrawFPS > 0) {
		if(iDrawFPS == 1 || iDrawFPS == 3) {
			ss << "FPS: ";
			ss << GetGameFPS();
			ss << "            ";
		}
		if(iDrawFPS == 2 || iDrawFPS == 3) {
			ss << "ms: ";
			ss << GetGameFrametime();
			ss << "            ";
		}
	}

	ss << GetFileName();
	ss << "            ";
	//trap->RenderTextShaded(ptSegoe, ss.str().c_str(), 0, 0, 0, 255, 255, 255);
}

void DisplayData() {
	DisplayDataTop();
}

void InitDisplay() {
	//ptSegoe = trap->RegisterFont("fonts/segoeui.ttf", 14);
}