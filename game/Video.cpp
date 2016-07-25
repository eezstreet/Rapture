#include "sys_local.h"
#include "tr_local.h"

#define VID_DEFAULT_RENDERER	"sdl"
#define VID_DEFAULT_WIDTH		1024
#define VID_DEFAULT_HEIGHT		768
#define VID_DEFAULT_TITLE		"Rapture"

#define VID_MIN_WIDTH			640
#define VID_MIN_HEIGHT			480

Cvar* vid_renderer		= nullptr;
Cvar* vid_width			= nullptr;
Cvar* vid_height		= nullptr;
Cvar* vid_windowtitle	= nullptr;
Cvar* vid_fullscreen	= nullptr;
Cvar* vid_gamma			= nullptr;

namespace Video {
	static SDL_Window* pWindow = nullptr;
	static Renderer* pRenderer = nullptr;

	static void WidthCallback(int newValue) {
		if (newValue <= VID_MIN_WIDTH) {
			R_Message(PRIORITY_WARNING, "WARNING: Invalid vid_width, using fallback resolution\n");
			SDL_SetWindowSize(pWindow, VID_MIN_WIDTH, VID_MIN_HEIGHT);
			return;
		}
		SDL_SetWindowSize(pWindow, newValue, vid_height->Integer());

		if (pRenderer && pRenderer->AreExportsValid()) {
			pRenderer->pExports->WindowWidthChanged(newValue);
		}
	}

	static void HeightCallback(int newValue) {
		if (newValue <= VID_MIN_WIDTH) {
			R_Message(PRIORITY_WARNING, "WARNING: Invalid vid_width, using fallback resolution\n");
			SDL_SetWindowSize(pWindow, VID_MIN_WIDTH, VID_MIN_HEIGHT);
			return;
		}
		SDL_SetWindowSize(pWindow, vid_width->Integer(), newValue);

		if (pRenderer && pRenderer->AreExportsValid()) {
			pRenderer->pExports->WindowHeightChanged(newValue);
		}
	}

	static void WindowTitleCallback(char* newValue) {
		SDL_SetWindowTitle(pWindow, newValue);
	}

	static void FullscreenCallback(bool newValue) {
		SDL_SetWindowFullscreen(pWindow, (newValue ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0));
	}

	static void GammaCallback(float newValue) {
		unsigned short ramp[256];
		SDL_CalculateGammaRamp(newValue, ramp);
		SDL_SetWindowGammaRamp(pWindow, ramp, ramp, ramp);
	}

	bool Init() {
		vid_fullscreen = Cvar::Get<bool>("vid_fullscreen", "Fullscreen mode?", (1 << CVAR_ARCHIVE), false);
		vid_gamma = Cvar::Get<float>("vid_gamma", "Window gamma ramp", (1 << CVAR_ARCHIVE), 1.0f);
		vid_height = Cvar::Get<int>("vid_height", "Size of the window (in pixels)", (1 << CVAR_ARCHIVE), VID_DEFAULT_HEIGHT);
		vid_renderer = Cvar::Get<char*>("vid_renderer", "Which renderer to use", (1 << CVAR_ARCHIVE), VID_DEFAULT_RENDERER);
		vid_width = Cvar::Get<int>("vid_width", "Size of the window (in pixels)", (1 << CVAR_ARCHIVE), VID_DEFAULT_WIDTH);
		vid_windowtitle = Cvar::Get<char*>("vid_windowtitle", "Title of window", (1 << CVAR_ARCHIVE), VID_DEFAULT_TITLE);

		vid_width->AddCallback((void*)WidthCallback);
		vid_height->AddCallback((void*)HeightCallback);
		vid_fullscreen->AddCallback((void*)FullscreenCallback);
		vid_windowtitle->AddCallback((void*)WindowTitleCallback);
		vid_gamma->AddCallback((void*)GammaCallback);

		if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK) < 0) {
			printf("could not init SDL (error code: %s)\n", SDL_GetError());
			return false;
		}

		pWindow = SDL_CreateWindow(vid_windowtitle->String(),
			SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED,
			vid_width->Integer(),
			vid_height->Integer(),
			SDL_WINDOW_OPENGL | (vid_fullscreen->Bool() ?
			SDL_WINDOW_FULLSCREEN : SDL_WINDOW_SHOWN));
		if (pWindow == nullptr) {
			SDL_Quit();
			return false;
		}

		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");

		pRenderer = new Renderer(vid_renderer->String());
		if (!pRenderer->AreExportsValid()) {
			delete pRenderer;
			return false;
		}
		pRenderer->Initialize();

		// Ramp the gamma up to what it should be
		vid_gamma->RunCallback();

		return true;
	}

	void Restart() {
		// TODO
	}

	void Shutdown() {
		unsigned short ramp[256];
		SDL_CalculateGammaRamp(1.0f, ramp);
		SDL_SetWindowGammaRamp(pWindow, ramp, ramp, ramp);
		R_Message(PRIORITY_MESSAGE, "gamma ramp\n");

		if (pRenderer && pRenderer->AreExportsValid()) {
			pRenderer->Shutdown();
		}
		if (pWindow) {
			SDL_DestroyWindow(pWindow);
		}
	}

	// These get exported to the renderer
	SDL_Window* GetRaptureWindow() {
		return pWindow;
	}

	void GetWindowInfo(int* renderWidth, int* renderHeight, bool* fullscreen) {
		if (renderWidth == nullptr || renderHeight == nullptr || fullscreen == nullptr) {
			return;
		}

		*renderWidth = vid_width->Integer();
		*renderHeight = vid_height->Integer();
		*fullscreen = vid_fullscreen->Bool();
	}

	int GetWidth() { return vid_width->Integer(); }
	int GetHeight() { return vid_height->Integer(); }

	// Wrappers for the renderer exports
	void ClearFrame() {
		if (pRenderer && pRenderer->AreExportsValid()) {
			pRenderer->ClearFrame();
		}
	}

	void RenderFrame() {
		if (pRenderer && pRenderer->AreExportsValid()) {
			pRenderer->RenderFrame();
		}
	}

	Texture* RegisterStreamingTexture(const unsigned int nW, const unsigned int nH) {
		if (pRenderer && pRenderer->AreExportsValid()) {
			return pRenderer->pExports->RegisterStreamingTexture(nW, nH);
		}
		return nullptr;
	}

	int LockStreamingTexture(Texture* ptTexture, unsigned int nX, unsigned int nY, unsigned int nW, unsigned int nH, void** pixels, int* pitch) {
		if (pRenderer && pRenderer->AreExportsValid()) {
			return pRenderer->pExports->LockStreamingTexture(ptTexture, nX, nY, nW, nH, pixels, pitch);
		}
		return -1;
	}

	void UnlockStreamingTexture(Texture* ptTexture) {
		if (pRenderer && pRenderer->AreExportsValid()) {
			pRenderer->pExports->UnlockStreamingTexture(ptTexture);
		}
	}

	void DeleteStreamingTexture(Texture* ptTexture) {
		if (pRenderer && pRenderer->AreExportsValid()) {
			pRenderer->pExports->DeleteStreamingTexture(ptTexture);
		}
	}

	void BlendTexture(Texture* ptTexture) {
		if (pRenderer && pRenderer->AreExportsValid()) {
			pRenderer->pExports->BlendTexture(ptTexture);
		}
	}

	// Font
	void RegisterFontAsync(const char* resourceURI, fontRegisteredCallback callback) {
		if (pRenderer && pRenderer->AreExportsValid()) {
			pRenderer->pExports->RegisterFontAsync(resourceURI, callback);
		}
	}

	void RenderSolidText(Font* font, const char* text, int x, int y, int r, int g, int b) {
		if (pRenderer && pRenderer->AreExportsValid()) {
			pRenderer->pExports->RenderSolidText(font, text, x, y, r, g, b);
		}
	}

	void RenderShadedText(Font* font, const char* text, int x, int y, int br, int bg, int bb, int fr, int fg, int fb) {
		if (pRenderer && pRenderer->AreExportsValid()) {
			pRenderer->pExports->RenderShadedText(font, text, x, y, br, bg, bb, fr, fg, fb);
		}
	}

	void RenderBlendedText(Font* font, const char* text, int x, int y, int r, int g, int b) {
		if (pRenderer && pRenderer->AreExportsValid()) {
			pRenderer->pExports->RenderBlendedText(font, text, x, y, r, g, b);
		}
	}

	// Texture Drawing
	Material* RegisterMaterial(const char* szMaterial) {
		if (pRenderer && pRenderer->AreExportsValid()) {
			return pRenderer->pExports->RegisterMaterial(szMaterial);
		}
		return nullptr;
	}

	void DrawMaterial(Material* ptMaterial, float xPct, float yPct, float wPct, float hPct) {
		if (pRenderer && pRenderer->AreExportsValid()) {
			pRenderer->pExports->DrawMaterial(ptMaterial, xPct, yPct, wPct, hPct);
		}
	}

	void DrawMaterialAspectCorrection(Material* ptMaterial, float xPct, float yPct, float wPct, float hPct) {
		if (pRenderer && pRenderer->AreExportsValid()) {
			pRenderer->pExports->DrawMaterialAspectCorrection(ptMaterial, xPct, yPct, wPct, hPct);
		}
	}

	void DrawMaterialClipped(Material* ptMaterial, float sxPct, float syPct, float swPct, float shPct, float ixPct, float iyPct, float iwPct, float ihPct) {
		if (pRenderer && pRenderer->AreExportsValid()) {
			pRenderer->pExports->DrawMaterialClipped(ptMaterial, sxPct, syPct, swPct, shPct, ixPct, iyPct, iwPct, ihPct);
		}
	}

	void DrawMaterialAbs(Material* ptMaterial, int nX, int nY, int nW, int nH) {
		if (pRenderer && pRenderer->AreExportsValid()) {
			pRenderer->pExports->DrawMaterialAbs(ptMaterial, nX, nY, nW, nH);
		}
	}

	void DrawMaterialAbsClipped(Material* ptMaterial, int sX, int sY, int sW, int sH, int iX, int iY, int iW, int iH) {
		if (pRenderer && pRenderer->AreExportsValid()) {
			pRenderer->pExports->DrawMaterialAbsClipped(ptMaterial, sX, sY, sW, sH, iX, iY, iW, iH);
		}
	}

	// Screenshots
	void QueueScreenshot(const char* szFileName, const char* szExtension) {
		if (pRenderer && pRenderer->AreExportsValid()) {
			pRenderer->pExports->QueueScreenshot(szFileName, szExtension);
		}
	}

	// Special Effects
	void FadeFromBlack(int ms) {
		if (pRenderer && pRenderer->AreExportsValid()) {
			pRenderer->pExports->FadeFromBlack(ms);
		}
	}
}