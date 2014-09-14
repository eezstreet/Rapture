#pragma once
#include "sys_shared.h"

typedef struct RenderTexture;

// Renderer.cpp
namespace RenderCode {
	void Initialize();
	void Exit(const bool bSilent = false);
	void Restart();
	void BlankFrame();
	void BeginFrame();
	void EndFrame();
	
	void BlendTexture(RenderTexture* tex);
	void QueueScreenshot(const string& fileName, const string& extension);

	Image* RegisterImage(const char* name);
	void DrawImage(Image* image, float xPct, float yPct, float wPct, float hPct);
	void DrawImageAbs(Image* image, int x, int y);
	void DrawImageAbs(Image* image, int x, int y, int w, int h);
	void DrawImageAspectCorrection(Image* image, float xPct, float yPct, float wPct, float hPct);
	void DrawImageClipped(Image* image, float sxPct, float syPct, float swPct,
		float shPct, float ixPct, float iyPct, float iwPct, float ihPct);
	void DrawImageAbsClipped(Image* image, int sX, int sY, int sW, int sH, int iX, int iY, int iW, int iH);
	void DrawImageAbsClipped(Image* image, int sX, int sY, int iX, int iY, int iW, int iH);

	void InitMaterials();
	void ShutdownMaterials();
	Material* RegisterMaterial(const char* name);
	void SendMaterialToRenderer(Material* ptMaterial, int x, int y);
	void SendMaterialToRendererTrans(Material* ptMaterial, int x, int y);

	void RenderTextSolid(Font* font, const char* text, int r, int g, int b);
	void RenderTextShaded(Font* font, const char* text, int br, int bg, int bb, int fr, int fg, int fb);
	void RenderTextBlended(Font* font, const char* text, int r, int g, int b);

	void FadeFromBlack(int ms);

	void AnimateMaterial(AnimationManager* ptAnims, Material* ptMaterial, int x, int y, bool bTransparency);
	AnimationManager* GetAnimation(const char *sUUID, const char* sMaterial);
	bool AnimationFinished(AnimationManager* ptAnim);
	void SetAnimSequence(AnimationManager* ptAnims, const char* sSequence);
	const char* GetAnimSequence(AnimationManager* ptAnims);

	RenderTexture* CreateFullscreenTexture();
	RenderTexture* TextureFromPixels(void* ptPixels, int width, int height, void* extra);
	void DestroyTexture(RenderTexture* texture);
	void LockTexture(RenderTexture* ptvTexture, void** pixels, int* span, bool bWriteOnly);
	void UnlockTexture(RenderTexture* ptvTexture);
};