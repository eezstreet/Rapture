#pragma once
#include "sys_shared.h"
#include <SDL.h>

namespace UI {
	void Initialize();
	void Render();
	void Shutdown();
	void Update();
	void KeyboardEvent(SDL_Scancode sc);
	void MouseButtonEvent(unsigned int buttonId, bool down);
	void MouseMoveEvent(int x, int y);
};