#pragma once
#include "sys_shared.h"

struct SDL_Window;
typedef struct _TTF_Font TTF_Font;

class Texture;

// Stuff that gets exported FROM the renderer
struct renderExports_s {
	// Basic logic
	void		(*Initialize)();
	void		(*Shutdown)();
	void		(*Restart)();

	// These get run every frame
	void		(*ClearFrame)();
	void		(*DrawActiveFrame)();

	// Texture manipulation
	Texture*	(*RegisterTexture)(const char* szTexturePath);
	Texture*	(*RegisterBlankTexture)(unsigned int nWidth, unsigned int nHeight);
	int			(*LockStreamingTexture)(Texture* ptTexture, unsigned int nX, unsigned int nY, unsigned int nW, unsigned int nH, void** pixels, int* pitch);
	void		(*UnlockStreamingTexture)(Texture* ptTexture);
	void		(*BlendTexture)(Texture* ptTexture);

	// Texture drawing
	void		(*DrawImage)(Texture* ptTexture, float xPct, float yPct, float wPct, float hPct);
	void		(*DrawImageAspectCorrection)(Texture* ptTexture, float xPct, float yPct, float wPct, float hPct);
	void		(*DrawImageClipped)(Texture* ptTexture, float sxPct, float syPct, float swPct, float shPct, float ixPct, float iyPct, float iwPct, float ihPct);
	void		(*DrawImageAbs)(Texture* ptTexture, int nX, int nY, int nW, int nH);
	void		(*DrawImageAbsClipped)(Texture* ptTexture, int sX, int sY, int sW, int sH, int iX, int iY, int iW, int iH);

	// Screenshots
	void		(*QueueScreenshot)(const char* szFileName, const char* szExtension);
	
	// Special effects
	void		(*FadeFromBlack)(int ms);

	// Text
	void		(*RenderTextSolid)(Font* font, const char* text, int r, int g, int b);
	void		(*RenderTextShaded)(Font* font, const char* text, int br, int bg, int bb, int fr, int fg, int fb);
	void		(*RenderTextBlended)(Font* font, const char* text, int r, int g, int b);

	// Callbacks
	void		(*WindowWidthChanged)(int newWidth);
	void		(*WindowHeightChanged)(int newHeight);
};

// Stuff that gets imported TO the renderer
struct renderImports_s {
	// Dispatch
	void			(*Print)(int priority, const char* fmt, ...);
	
	// Video
	void			(*GetRenderProperties)(int* renderWidth, int* renderHeight, bool* fullscreen);
	SDL_Window*		(*GetWindow)();

	// Cvar
	Cvar*			(*RegisterStringCvar)(const char* name, const char* description, char* defaultValue, int flags);
	Cvar*			(*RegisterIntCvar)(const char* name, const char* description, int defaultValue, int flags);
	Cvar*			(*RegisterFloatCvar)(const char* name, const char* description, float defaultValue, int flags);
	Cvar*			(*RegisterBoolCvar)(const char* name, const char* description, bool defaultValue, int flags);

	char*			(*CvarStringValue)(Cvar* cvar);
	int				(*CvarIntegerValue)(Cvar* cvar);
	float			(*CvarFloatValue)(Cvar* cvar);
	bool			(*CvarBooleanValue)(Cvar* cvar);

	// Files
	File*			(*OpenFile)(const char* szFileName, const char* szMode);
	void			(*CloseFile)(File* pFile);
	void			(*WritePlaintext)(File* pFile, const char* text);
	char*			(*FileSearchPath)(const char* szFileName);

	// Font
	TTF_Font*		(*GetFontTTF)(Font* pFont);
};

//
// Video.cpp
//
namespace Video {
	// Basic
	bool Init();
	void Restart();
	void Shutdown();

	// Run every frame
	void ClearFrame();
	void RenderFrame();

	// Texture manipulation
	Texture* RegisterTexture(const char* szTexture);
	Texture* RegisterBlankTexture(const unsigned int nWidth, const unsigned int nHeight);
	int LockStreamingTexture(Texture* ptTexture, unsigned int nX, unsigned int nY, unsigned int nW, unsigned int nH, void** pixels, int* pitch);
	void UnlockStreamingTexture(Texture* ptTexture);
	void BlendTexture(Texture* ptTexture);

	// Texture Drawing
	void DrawImage(Texture* ptTexture, float xPct, float yPct, float wPct, float hPct);
	void DrawImageAspectCorrection(Texture* ptTexture, float xPct, float yPct, float wPct, float hPct);
	void DrawImageClipped(Texture* ptTexture, float sxPct, float syPct, float swPct, float shPct, float ixPct, float iyPct, float iwPct, float ihPct);
	void DrawImageAbs(Texture* ptTexture, int nX, int nY, int nW, int nH);
	void DrawImageAbsClipped(Texture* ptTexture, int sX, int sY, int sW, int sH, int iX, int iY, int iW, int iH);

	// Screenshots
	void QueueScreenshot(const char* szFileName, const char* szExtension);

	// Special Effects
	void FadeFromBlack(int ms);

	// Text
	void RenderTextSolid(Font* font, const char* text, int r, int g, int b);
	void RenderTextShaded(Font* font, const char* text, int br, int bg, int bb, int fr, int fg, int fb);
	void RenderTextBlended(Font* font, const char* text, int r, int g, int b);

	// Imported from the engine
	SDL_Window* GetRaptureWindow();
	void GetWindowInfo(int* renderWidth, int* renderHeight, bool* fullscreen);
	int GetWidth();
	int GetHeight();
};