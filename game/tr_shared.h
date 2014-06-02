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
	void DrawImageAspectCorrection(void* image, float xPct, float yPct, float wPct, float hPct);
	void DrawImageNoScaling(void* image, float xPct, float yPct);
	void DrawImageClipped(void* image, float sxPct, float syPct, float swPct,
		float shPct, float ixPct, float iyPct, float iwPct, float ihPct);
	void DrawImageAbs(void* image, int x, int y, int w, int h);
	void DrawImageAbsAspectCorrection(void* image, int x, int y, int w, int h);
	void DrawImageAbsNoScaling(void* image, int x, int y);
	void DrawImageAbsClipped(void* image, int sx, int sy, int sw, int sh, int ix, int iy, int iw, int ih);

	void InitMaterials();
	void ShutdownMaterials();
	void* RegisterMaterial(const char* name);
	void SendMaterialToRenderer(void* ptMaterial, float x, float y);

	void RenderTextSolid(void* font, const char* text, int r, int g, int b);
	void RenderTextShaded(void* font, const char* text, int br, int bg, int bb, int fr, int fg, int fb);
	void RenderTextBlended(void* font, const char* text, int r, int g, int b);

	void* GetRenderer();
};
