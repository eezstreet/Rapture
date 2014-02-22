#pragma once
#include "sys_shared.h"

// Renderer.cpp
namespace RenderCode {
	void Initialize();
	void Exit();
	void BlankFrame();
	void Display();
	
	void* AddSurface(void* surf);
	void QueueScreenshot(const string& fileName, const string& extension);

	void* RegisterImage(const char* name);
	void DrawImage(void* image, float xPct, float yPct, float wPct, float hPct);
};
