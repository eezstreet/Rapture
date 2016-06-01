#pragma once
#include "../game/tr_shared.h"
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

extern renderImports_s* trap;
extern SDL_Renderer* renderer;

namespace RenderExport {
	void Initialize();
	void Shutdown();
	void Restart();

	void ClearFrame();
	void DrawActiveFrame();

	// Texture Manipulation
	Texture* RegisterTexture(const char* szTexture);
	Texture* RegisterBlankTexture(const unsigned int nW, const unsigned int nH);
	int LockStreamingTexture(Texture* ptTexture, unsigned int nX, unsigned int nY, unsigned int nW, unsigned int nH, void** pixels, int* pitch);
	void UnlockStreamingTexture(Texture* ptTexture);
	void BlendTexture(Texture* ptTexture);

	// Texture Drawing
	void DrawImageAbs(Texture* ptTexture, int nX, int nY, int nW, int nH);
	void DrawImageAbsClipped(Texture* ptTexture, int sX, int sY, int sW, int sH, int iX, int iY, int iW, int iH);
	void DrawImage(Texture* ptTexture, float xPct, float yPct, float wPct, float hPct);
	void DrawImageAspectCorrection(Texture* ptTexture, float xPct, float yPct, float wPct, float hPct);
	void DrawImageClipped(Texture* ptTexture, float sxPct, float syPct, float swPct, float shPct, float ixPct, float iyPct, float iwPct, float ihPct);

	// Screenshots
	void QueueScreenshot(const char* szFileName, const char* szExtension);

	// Special Effects
	void FadeFromBlack(int ms);

	// Text
	void RenderTextSolid(Font* font, const char* text, int r, int g, int b);
	void RenderTextShaded(Font* font, const char* text, int br, int bg, int bb, int fr, int fg, int fb);
	void RenderTextBlended(Font* font, const char* text, int r, int g, int b);

	// Callbacks
	void WindowWidthChanged(int newWidth);
	void WindowHeightChanged(int newHeight);
}

class TextManager {
private:
	vector<SDL_Texture*> vTextFields;
public:
	TextManager();
	~TextManager();
	TextManager(const TextManager& other);

	void RenderTextSolid(Font* font, const char* text, int r, int g, int b);
	void RenderTextShaded(Font* font, const char* text, int br, int bg, int bb, int fr, int fg, int fb);
	void RenderTextBlended(Font* font, const char* text, int r, int g, int b);
	void ResetFrame();
};
extern TextManager* ptText;

class TextureManager {
private:
	unordered_map<string, Texture*> umTextureMap;
	int nLastUnnamed = 0;
public:
	TextureManager();
	~TextureManager();
	TextureManager(const TextureManager& other);

	Texture* RegisterTexture(const char* name);
	Texture* RegisterTexture(Texture* newTexture);
};
extern TextureManager* ptTexMan;

class Texture {
private:
	SDL_Texture* ptTexture;
	char name[64];
	bool bValid;
public:
	Texture(const char* name);
	Texture(const unsigned int nW, const unsigned nH);
	Texture(const Texture& other);
	~Texture();

	int Lock(unsigned int nX, unsigned int nY, unsigned int nW, unsigned int nH, void** pixels, int* pitch);
	void Unlock();
	void Blend();

	void FadeTexture();

	void DrawImage(int nX, int nY, int nW, int nH);
	void DrawImage(float fXPct, float fYPct, float fWPct, float fHPct);
	void DrawImageAspectCorrection(float fXPct, float fYPct, float fWPct, float fHPct);
	void DrawImageClipped(float sxPct, float syPct, float swPct, float shPct, float ixPct, float iyPct, float iwPct, float ihPct);
	void DrawImageClipped(SDL_Rect* spos, SDL_Rect* ipos);
	void DrawAbs(int nX, int nY, int nW, int nH);
	void DrawAbsClipped(int sX, int sY, int sW, int sH, int iX, int iY, int iW, int iH);
friend class TextureManager;
};

struct Screenshot {
	SDL_Surface* screenSurf;
	char name[64];

	Screenshot(const char* name);
	Screenshot(const Screenshot& other);
	~Screenshot();
};
extern vector<Screenshot*> qScreenshots;

struct FadeEffectData {
	int fadeTime;
	int initialFadeTime;
	bool bShouldWeFade;
	Uint32 currentTime;
};
extern FadeEffectData fade;