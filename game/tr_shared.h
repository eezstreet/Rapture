#include "sys_shared.h"

// Renderer.cpp
namespace RenderCode {
	void Initialize();
	void Exit();
	void BlankFrame();
	void Display();
	
	void* TexFromSurface(void* surface); // FIXME: wrong header
	void GetWindowSize(int* w, int* h);
};
