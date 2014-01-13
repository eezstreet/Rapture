#include "ui_local.h"

float UISystemInterface::GetElapsedTime() {
	return SDL_GetTicks() / 1000.0f;
}