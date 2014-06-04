#pragma once
#include "sys_shared.h"

// Renderer.cpp
namespace RenderCode {
	void Initialize();
	void Exit(const bool bSilent = false);
	void BlankFrame();
	void Display();
	
	void AddSurface(void* surf);
	void BlendTexture(void* tex);
	void QueueScreenshot(const string& fileName, const string& extension);

	void* RegisterImage(const char* name);
	void DrawImage(void* image, float xPct, float yPct, float wPct, float hPct);
	void DrawImageAbs(void* image, int x, int y);
	void DrawImageAbs(void* image, int x, int y, int w, int h);
	void DrawImageAspectCorrection(void* image, float xPct, float yPct, float wPct, float hPct);
	void DrawImageClipped(void* image, float sxPct, float syPct, float swPct,
		float shPct, float ixPct, float iyPct, float iwPct, float ihPct);

	void InitMaterials();
	void ShutdownMaterials();
	void* RegisterMaterial(const char* name);
	void SendMaterialToRenderer(void* ptMaterial, float x, float y);

	void RenderTextSolid(void* font, const char* text, int r, int g, int b);
	void RenderTextShaded(void* font, const char* text, int br, int bg, int bb, int fr, int fg, int fb);
	void RenderTextBlended(void* font, const char* text, int r, int g, int b);

	void* GetRenderer();
};
