#pragma once
#include "sys_shared.h"

// Renderer.cpp
namespace RenderCode {
	void Initialize();
	void Exit(const bool bSilent = false);
	void Restart();
	void BlankFrame();
	void Display();
	
	void AddSurface(void* surf);
	void BlendTexture(void* tex);
	void QueueScreenshot(const string& fileName, const string& extension);

	Image* RegisterImage(const char* name);
	void DrawImage(Image* image, float xPct, float yPct, float wPct, float hPct);
	void DrawImageAbs(Image* image, int x, int y);
	void DrawImageAbs(Image* image, int x, int y, int w, int h);
	void DrawImageAspectCorrection(Image* image, float xPct, float yPct, float wPct, float hPct);
	void DrawImageClipped(Image* image, float sxPct, float syPct, float swPct,
		float shPct, float ixPct, float iyPct, float iwPct, float ihPct);

	void InitMaterials();
	void ShutdownMaterials();
	Material* RegisterMaterial(const char* name);
	void SendMaterialToRenderer(Material* ptMaterial, int x, int y);
	void SendMaterialToRendererTrans(Material* ptMaterial, int x, int y);

	void RenderTextSolid(Font* font, const char* text, int r, int g, int b);
	void RenderTextShaded(Font* font, const char* text, int br, int bg, int bb, int fr, int fg, int fb);
	void RenderTextBlended(Font* font, const char* text, int r, int g, int b);

	void* GetRenderer();
};
