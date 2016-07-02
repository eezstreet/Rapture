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

class Material {
private:
	static unordered_map<string, Material*> umMaterials;
	
	ComponentMaterial::MaterialHeader matHeader;
	Texture* diffuseTexture;
	Texture* normalTexture;	// not used on SDL renderer?
	Texture* depthTexture;	// not used on SDL renderer?

	bool bValid;
public:
	Material(const char* szURI);
	~Material();
	Material(Material& other);
	void Draw(float xPct, float yPct, float wPct, float hPct);
	void DrawAspectCorrection(float xPct, float yPct, float wPct, float hPct);
	void DrawClipped(float sxPct, float syPct, float swPct, float shPct, float ixPct, float iyPct, float iwPct, float ihPct);
	void DrawAbs(int nX, int nY, int nW, int nH);
	void DrawAbsClipped(int sX, int sY, int sW, int sH, int iX, int iY, int iW, int iH);
	bool Valid() { return bValid; }

	static Material* Register(const char* szURI);
	static void DrawMaterial(Material* pMat, float xPct, float yPct, float wPct, float hPct);
	static void DrawMaterialAspectCorrection(Material* pMat, float xPct, float yPct, float wPct, float hPct);
	static void DrawMaterialClipped(Material* pMat, float sxPct, float syPct, float swPct, float shPct, float ixPct, float iyPct, float iwPct, float ihPct);
	static void DrawMaterialAbs(Material* pMat, int nX, int nY, int nW, int nH);
	static void DrawMaterialAbsClipped(Material* pMat, int sX, int sY, int sW, int sH, int iX, int iY, int iW, int iH);
	static void KillAllMaterials();
};

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

class Texture {
private:
	SDL_Texture* ptTexture;
	bool bValid;
public:
	Texture(const unsigned int nW, const unsigned int nH, uint32_t* pixels = nullptr);
	Texture(const unsigned int nW, const unsigned int nH, uint16_t* pixels);
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

	static Texture* RegisterStreamingTexture(const unsigned int nW, const unsigned int nH);
	static int LockStreamingTexture(Texture* ptTexture, unsigned int nX, unsigned int nY, unsigned int nW, unsigned int nH, void** pixels, int* pitch);
	static void UnlockStreamingTexture(Texture* ptTexture);
	static void DeleteStreamingTexture(Texture* pTexture);
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