#include "g_local.h"

int currentMouseX = 0;
int currentMouseY = 0;
float currentWorldpsaceX = 0;
float currentWorldspaceY = 0;

Font* ptConsolasFont;

void RegisterMedia() {
	ptConsolasFont = trap->RegisterFont("fonts/consola.ttf", 18);
}

void DrawViewportInfo() {
	int drawFPS = 0;
	bool drawXY = false;
	bool drawWorldXY = false;
	stringstream ss;

	trap->CvarIntVal("cg_drawfps", &drawFPS);
	trap->CvarBoolVal("cg_drawxy", &drawXY);
	trap->CvarBoolVal("cg_drawworldxy", &drawWorldXY);

	if(drawFPS >= 1) {
		FPSFrame();

		if(drawFPS == 1 || drawFPS == 3) {
			ss << "FPS: ";
			ss << GetGameFPS();
			ss << "      ";
		}
		if(drawFPS == 2 || drawFPS == 3) {
			ss << "ms: ";
			ss << GetGameFrametime();
			ss << "      ";
		}
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
		ss << (Worldspace::ScreenSpaceToWorldPlaceX(currentMouseX, currentMouseY));;
		ss << "X / ";
		ss << (Worldspace::ScreenSpaceToWorldPlaceY(currentMouseX, currentMouseY));
		ss << "Y      ";
	}
	if(ss.str().length() > 0) {
		trap->RenderTextShaded(ptConsolasFont, ss.str().c_str(), 255, 255, 255, 127, 127, 127);
	}
}