#include "g_local.h"

int currentMouseX = 0;
int currentMouseY = 0;
float currentWorldpsaceX = 0;
float currentWorldspaceY = 0;

void* ptConsolasFont;

void RegisterMedia() {
	ptConsolasFont = trap->RegisterFont("fonts/consola.ttf", 18);
}

void DrawViewportInfo() {
	bool drawFPS = false;
	bool drawXY = false;
	bool drawWorldXY = false;
	stringstream ss;

	trap->CvarBoolVal("cg_drawfps", &drawFPS);
	trap->CvarBoolVal("cg_drawxy", &drawXY);
	trap->CvarBoolVal("cg_drawworldxy", &drawWorldXY);

	if(drawFPS) {
		FPSFrame();

		ss << "FPS: ";
		ss << GetGameFPS();
		ss << "      ";
	}

	if(drawXY) {
		ss << "Screen Space: ";
		ss << currentMouseX;
		ss << "X / ";
		ss << currentMouseY;
		ss << "Y      ";
	}

	if(drawWorldXY) {
		ss << "World Space: ";
		ss << Worldspace::ScreenSpaceToWorldPlaceX(currentMouseX, currentMouseY);
		ss << "X / ";
		ss << Worldspace::ScreenSpaceToWorldPlaceY(currentMouseX, currentMouseY);
		ss << "Y      ";
	}
	if(ss.str().length() > 0) {
		trap->RenderTextShaded(ptConsolasFont, ss.str().c_str(), 255, 255, 255, 127, 127, 127);
	}
}