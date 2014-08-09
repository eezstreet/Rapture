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
	void AddJavaScriptCallback(Menu* menu, const char* sCallbackName, void(*ptCallback)());
	unsigned int GetJavaScriptNumArgs(Menu* menu);
	void GetJavaScriptStringArgument(Menu* menu, unsigned int iArgNum, char* sBuffer, size_t numChars);
	int GetJavaScriptIntArgument(Menu* menu, unsigned int iArgNum);
	double GetJavaScriptDoubleArgument(Menu* menu, unsigned int iArgNum);
	bool GetJavaScriptBoolArgument(Menu* menu, unsigned int iArgNum);
};

void PushConsoleMessage(string message);