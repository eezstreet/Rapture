#include "tr_local.h"

renderImports_s *trap = nullptr;

SDL_Window* pWindow = nullptr;
SDL_Renderer* renderer = nullptr;
SDL_RendererInfo renderInfo;

SDL_Surface* renderSurf = nullptr;
SDL_Texture* renderTex = nullptr;

FadeEffectData fade;

// Cvars
Cvar* r_textureFiltering;

void InitCvars() {
	r_textureFiltering = trap->RegisterIntCvar("r_textureFiltering", 
		"Texture filter quality. 0 = nearest pixel, 1 = linear filtering, 2 = anisotropic filtering", 1, (1 << CVAR_ARCHIVE));
}

// Render Exports
void RenderExport::Initialize() {
	int nWidth = 0, nHeight = 0;
	bool bFullscreen = false;

	trap->Print(PRIORITY_NOTE, "Render Init : SDL\n");
	trap->GetRenderProperties(&nWidth, &nHeight, &bFullscreen);

	pWindow = trap->GetWindow();
	if (pWindow == nullptr) {
		trap->Print(PRIORITY_ERRFATAL, "Bad window sent to renderer\n");
		return;
	}

	renderer = SDL_CreateRenderer(pWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
	if (renderer == nullptr) {
		trap->Print(PRIORITY_ERRFATAL, "Failed to create SDL renderer\n");
		return;
	}

	SDL_GetRendererInfo(renderer, &renderInfo);
	if (renderInfo.flags & SDL_RENDERER_SOFTWARE) {
		trap->Print(PRIORITY_WARNING, "Using software renderer. Performance will suffer.\n");
	}

	if (!(renderInfo.flags & SDL_RENDERER_TARGETTEXTURE)) {
		trap->Print(PRIORITY_ERRFATAL, "Render-to-texture not supported");
		SDL_DestroyRenderer(renderer);
		return;
	}

	SDL_RenderSetLogicalSize(renderer, nWidth, nHeight);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
	SDL_SetRenderTarget(renderer, nullptr);

	InitCvars();

	string filter = "" + trap->CvarIntegerValue(r_textureFiltering);
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, filter.c_str());

	// Create render texture (this will need to be destroyed and recreated entirely if we change video dimensions)
	renderSurf = SDL_CreateRGBSurface(0, nWidth, nHeight, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
	renderTex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, nWidth, nHeight);

	ptTexMan = new TextureManager();

	fade.bShouldWeFade = false;

	ptText = new TextManager();

	int flags = IMG_INIT_JPG | IMG_INIT_PNG;
	trap->Print(PRIORITY_NOTE, "IMG_Init()\n");
	if ((IMG_Init(flags) & flags) != flags) {
		trap->Print(PRIORITY_ERROR, "FAILED! %s\n", IMG_GetError());
	}
}

void RenderExport::Shutdown() {
	delete ptTexMan;
	delete ptText;

	SDL_DestroyTexture(renderTex);
	SDL_FreeSurface(renderSurf);

	SDL_DestroyRenderer(renderer);
}

void RenderExport::Restart() {
	// TODO
}

void RenderExport::WindowWidthChanged(int newWidth) {
	int nWidth = 0, nHeight = 0;
	bool bFullscreen = false;
	
	trap->GetRenderProperties(&nWidth, &nHeight, &bFullscreen);
	SDL_RenderSetLogicalSize(renderer, newWidth, nHeight);
}

void RenderExport::WindowHeightChanged(int newHeight) {
	int nWidth = 0, nHeight = 0;
	bool bFullscreen = false;

	trap->GetRenderProperties(&nWidth, &nHeight, &bFullscreen);
	SDL_RenderSetLogicalSize(renderer, nWidth, newHeight);
}

void RenderExport::ClearFrame() {
	ptText->ResetFrame();
	SDL_RenderClear(renderer);

	fade.currentTime = SDL_GetTicks();
	if (fade.fadeTime > fade.currentTime) {
		fade.bShouldWeFade = true;
	}
	else if (fade.bShouldWeFade) {
		fade.bShouldWeFade = false;
	}
}

void RenderExport::DrawActiveFrame() {
	for (auto it = qScreenshots.begin(); it != qScreenshots.end(); it++) {
		it = qScreenshots.erase(it);
	}
	SDL_RenderPresent(renderer);
}

void RenderExport::QueueScreenshot(const char *szFileName, const char *szExtension) {
	string szFullName = szFileName;
	szFullName += szExtension;

	Screenshot* newScreen = new Screenshot(szFullName.c_str());
}

void RenderExport::FadeFromBlack(int ms) {
	fade.currentTime = SDL_GetTicks();
	fade.initialFadeTime = fade.currentTime;
	fade.fadeTime = fade.initialFadeTime + ms;
}

void RenderExport::RenderTextSolid(Font* font, const char* text, int r, int g, int b) {
	ptText->RenderTextSolid(font, text, r, g, b);
}

void RenderExport::RenderTextShaded(Font* font, const char* text, int br, int bg, int bb, int fr, int fg, int fb) {
	ptText->RenderTextShaded(font, text, br, bg, bb, fr, fg, fb);
}

void RenderExport::RenderTextBlended(Font* font, const char* text, int r, int g, int b) {
	ptText->RenderTextBlended(font, text, r, g, b);
}

// Main entrypoint of the program
static renderExports_s renderExport;
extern "C" {
	__declspec(dllexport) renderExports_s* GetRefAPI(renderImports_s* import) {
		trap = import;

		renderExport.Initialize = RenderExport::Initialize;
		renderExport.Shutdown = RenderExport::Shutdown;
		renderExport.Restart = RenderExport::Restart;

		renderExport.WindowWidthChanged = RenderExport::WindowWidthChanged;
		renderExport.WindowHeightChanged = RenderExport::WindowHeightChanged;

		renderExport.ClearFrame = RenderExport::ClearFrame;
		renderExport.DrawActiveFrame = RenderExport::DrawActiveFrame;

		renderExport.RegisterTexture = RenderExport::RegisterTexture;
		renderExport.RegisterBlankTexture = RenderExport::RegisterBlankTexture;
		renderExport.LockStreamingTexture = RenderExport::LockStreamingTexture;
		renderExport.UnlockStreamingTexture = RenderExport::UnlockStreamingTexture;
		renderExport.BlendTexture = RenderExport::BlendTexture;

		renderExport.DrawImage = RenderExport::DrawImage;
		renderExport.DrawImageAspectCorrection = RenderExport::DrawImageAspectCorrection;
		renderExport.DrawImageClipped = RenderExport::DrawImageClipped;
		renderExport.DrawImageAbs = RenderExport::DrawImageAbs;
		renderExport.DrawImageAbsClipped = RenderExport::DrawImageAbsClipped;

		renderExport.QueueScreenshot = RenderExport::QueueScreenshot;

		renderExport.FadeFromBlack = RenderExport::FadeFromBlack;

		renderExport.RenderTextBlended = RenderExport::RenderTextBlended;
		renderExport.RenderTextShaded = RenderExport::RenderTextShaded;
		renderExport.RenderTextSolid = RenderExport::RenderTextSolid;

		return &renderExport;
	}
}