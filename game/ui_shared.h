#pragma once
#include "sys_shared.h"
#include <SDL.h>

namespace UI {
	void Initialize();
	void Render();
	void Shutdown();
	void Update();
	void Restart();
	void KeyboardEvent(SDL_Keysym keysym, bool bIsKeyDown, char* text);
	void TextEvent(char* text);
	void MouseButtonEvent(unsigned int buttonId, bool down);
	void MouseMoveEvent(int x, int y);

	Menu* RegisterStaticMenu(const char* menuPath);
	void KillStaticMenu(Menu* menu);
	void RunJavaScript(Menu* menu, const char* sJS);
};

void PushConsoleMessage(string message);