#include "tr_local.h"

RendSDL::RendSDL() {
	renderSurf = nullptr;
	renderTex = nullptr;
}

RendSDL::~RendSDL() {
	SDL_DestroyTexture(renderTex);
	SDL_FreeSurface(renderSurf);

	for(int i = 0; i < MAX_TEXTRENDER; i++) {
		SDL_DestroyTexture(textFields[i]);
	}

	if(1) {
		R_Message(PRIORITY_NOTE, "IMG_Quit()\n");
	}
	IMG_Quit();
	for(auto it = images.begin(); it != images.end(); ++it) {
		SDL_DestroyTexture(it->second);
	}
	images.clear();
}

bool RendSDL::Start(SDL_Window* ptWindow) {
	ptRenderer = SDL_CreateRenderer(ptWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
	if(ptRenderer == nullptr) {
		R_Message(PRIORITY_ERROR, "Unable to start renderer\n");
		return false;
	}

	SDL_RendererInfo info;
	SDL_GetRendererInfo(ptRenderer, &info);
	if(info.flags & SDL_RENDERER_SOFTWARE) {
		R_Message(PRIORITY_WARNING, "WARNING: No GPU detected, falling back to software renderer...\n");
	}
	if(!(info.flags & SDL_RENDERER_TARGETTEXTURE)) {
		R_Error("ERROR: Renderer does not support render-to-texture. Your GPU does not meet the minimum requirements.\n");
		return false;
	}

	// Anisotropic filtering
	string s = "" + r_filter->Integer();
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, s.c_str());
	SDL_RenderSetLogicalSize(ptRenderer, r_width->Integer(), r_height->Integer());

	renderSurf = SDL_CreateRGBSurface(0, r_width->Integer(), r_height->Integer(), 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
	renderTex = SDL_CreateTexture(ptRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, r_width->Integer(), r_height->Integer());
	SDL_SetRenderDrawColor(ptRenderer, 0, 0, 0, 0);

	int flags = IMG_INIT_JPG | IMG_INIT_PNG;
	R_Message(PRIORITY_NOTE, "IMG_Init()\n");
	if((IMG_Init(flags) & flags) != flags) { // the 'S' in SDL does not stand for "sense"
		R_Message(PRIORITY_ERROR, "IMG_Init failed: %s\n", IMG_GetError());
	}

	SDL_SetRenderTarget(ptRenderer, nullptr);

	R_Message(PRIORITY_NOTE, "Init font\n");
	for(int i = 0; i < MAX_TEXTRENDER; i++) {
		textFields[i] = SDL_CreateTexture(ptRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, r_width->Integer(), r_height->Integer());
	}

	m_ptWindow = ptWindow;
}

void RendSDL::Restart() {
	string sFilterHint = "" + r_filter->Integer();

	r_gamma->RunCallback();

	SDL_SetWindowFullscreen(m_ptWindow, (r_fullscreen->Bool() ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0));
	SDL_SetWindowSize(m_ptWindow, r_width->Integer(), r_height->Integer());

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, sFilterHint.c_str());
	SDL_RenderSetLogicalSize(ptRenderer, r_width->Integer(), r_height->Integer());
}

void RendSDL::StartFrame() {
	SDL_RenderClear(ptRenderer);
	textFieldCount = 0;
}

void RendSDL::EndFrame() {
	SDL_RenderPresent(ptRenderer);
}

// Texture manipulation
RenderTexture* RendSDL::CreateTexture(Uint32 dwWidth, Uint32 dwHeight) {
	RenderTexture* returnTexture = Zone::New<RenderTexture>(Zone::TAG_IMAGES);
	returnTexture->width = dwWidth;
	returnTexture->height = dwHeight;
	returnTexture->data.ptTexture = SDL_CreateTexture(this->ptRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, dwWidth, dwHeight);
	return returnTexture;
}

RenderTexture* RendSDL::TextureFromPixels(void* pixels, int width, int height, uint32_t extra) {
	RenderTexture* returnTexture = Zone::New<RenderTexture>(Zone::TAG_IMAGES);
	returnTexture->width = width;
	returnTexture->height = height;
	returnTexture->data.ptTexture = SDL_CreateTextureFromSurface(ptRenderer, (SDL_Surface*)extra);
	return returnTexture;
}

void RendSDL::LockTexture(RenderTexture* ptvTexture, void** pixels, int* span, bool bWriteOnly) {
	// In SDL, the bWriteOnly arg is actually irrelevant since you can't do a texture lock
	// without reading from the texture (stupid)
	SDL_Texture* tex = (SDL_Texture*)ptvTexture;
	if(SDL_LockTexture(tex, nullptr, pixels, span) < 0) {
		R_Error("Failed to lock texture: %s\n", SDL_GetError());
	}
}

void RendSDL::UnlockTexture(RenderTexture* ptvTexture) {
	SDL_UnlockTexture((SDL_Texture*)ptvTexture);
}

void RendSDL::DeleteTexture(RenderTexture* ptvTexture) {
	SDL_DestroyTexture((SDL_Texture*)ptvTexture);
}

void RendSDL::BlendTexture(RenderTexture* ptvTexture) {
	SDL_Texture* ptTexture = ptvTexture->data.ptTexture;
	SDL_SetRenderTarget(ptRenderer, nullptr);
	SDL_SetTextureBlendMode(ptTexture, SDL_BLENDMODE_BLEND);
	SDL_RenderCopy(ptRenderer, ptTexture, nullptr, nullptr);
}

// Screenshot
void RendSDL::CreateScreenshot(const string& sFileName) {
	SDL_Surface* s = SDL_GetWindowSurface(m_ptWindow);
	unsigned int numPixels = s->w * s->h * s->format->BytesPerPixel;
	unsigned char* pixels = new unsigned char[numPixels];
	SDL_RenderReadPixels(ptRenderer, &s->clip_rect, s->format->format, pixels, s->w * s->format->BytesPerPixel);
	SDL_Surface* screenshot = SDL_ConvertSurfaceFormat(
		SDL_CreateRGBSurfaceFrom(pixels, s->w, s->h, s->format->BitsPerPixel, s->w * s->format->BytesPerPixel,
		s->format->Rmask, s->format->Gmask, s->format->Bmask, s->format->Amask),
		SDL_PIXELFORMAT_RGB444, 0);

	// hacky
	File* f = File::Open(sFileName, "wb+");
	f->WritePlaintext("blah");
	f->Close();
	const string path = File::GetFileSearchPath(sFileName);
	SDL_SaveBMP(screenshot, path.c_str());
	R_Message(PRIORITY_MESSAGE, "Screenshot: %s\n", sFileName.c_str());

	delete[] pixels;
	SDL_FreeSurface(s);
	SDL_FreeSurface(screenshot);
}