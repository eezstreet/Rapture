#include "g_local.h"

static Menu* cg_HUD = NULL;

extern int visTouching;

void InitHUD() {
	R_Printf("InitHUD()\n");
	cg_HUD = trap->RegisterStaticMenu("ui/hud.html");
	HUD_EnterArea("Survivor's Camp");
}

void ShutdownHUD() {
	if(!cg_HUD) {
		return;
	}
	R_Printf("ShutdownHUD()\n");
	trap->KillStaticMenu(cg_HUD);
	cg_HUD = NULL;
}

void HUD_EnterArea(const char* areaName) {
	stringstream fullName;
	fullName << "changeDivHTML('ID_zone', \"" << areaName << "\");";
	trap->RunJavaScript(cg_HUD, fullName.str().c_str());
}

void HUD_DrawLabel(const char* labelText) {
	stringstream fullName;
	fullName << "startLabelDraw(\"" << labelText << "\", " << currentMouseX << ", " << currentMouseY << ");";
	trap->RunJavaScript(cg_HUD, fullName.str().c_str());
}

void HUD_HideLabels() {
	trap->RunJavaScript(cg_HUD, "stopLabelDraw();");
}

static int previousVis = -1;
void HUD_Frame() {
	if(visTouching != previousVis) {
		if(visTouching == -1) {
			// Stop drawing labels
			HUD_HideLabels();
		} else {
			// Should probably draw label text??
			HUD_DrawLabel(ptDungeonManager->FindNextMapStr(0, 0, visTouching).c_str());
		}
		previousVis = visTouching;
	}
}