#pragma once
#include "sys_shared.h"

// Renderer.cpp
namespace RenderCode {
	enum ImageFormats {
		IMG_BMP,
	};

	void Initialize();
	void Exit();
	void BlankFrame();
	void Display();
	
	void* AddSurface(void* surf);
	void QueueScreenshot(const string& fileName, const string& extension);
};
