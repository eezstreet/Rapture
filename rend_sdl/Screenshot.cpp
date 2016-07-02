#include "tr_local.h"

vector<Screenshot*> qScreenshots;

Screenshot::Screenshot(const char* szName) {
	strcpy(name, szName);

	SDL_Surface* s = SDL_GetWindowSurface(trap->GetWindow());
	unsigned int numPixels = s->w * s->h * s->format->BytesPerPixel;
	unsigned char * pixels = new unsigned char[numPixels];
	SDL_RenderReadPixels(renderer, &s->clip_rect, s->format->format, pixels, s->w * s->format->BytesPerPixel);
	this->screenSurf = SDL_ConvertSurfaceFormat(SDL_CreateRGBSurfaceFrom
		(pixels, s->w, s->h, s->format->BitsPerPixel, s->w * s->format->BytesPerPixel, s->format->Rmask, s->format->Gmask, s->format->Bmask, s->format->Amask),
		SDL_PIXELFORMAT_RGB444, 0);

	SDL_FreeSurface(s);
}

Screenshot::~Screenshot() {
	// When the screenshot object dies, we write it to the disk. Clever, no?
	const char* mode = "wb+";
	File* f = trap->OpenFileSync(this->name, mode);

	SDL_RWops* rw = SDL_AllocRW();
	SDL_SaveBMP_RW(screenSurf, rw, 0);

	// Figure out how big from the rwops
	size_t fileSize;
	SDL_RWseek(rw, 0, RW_SEEK_SET);	// Make sure that we start at the beginning
	SDL_RWseek(rw, 0, RW_SEEK_END); 
	fileSize = SDL_RWtell(rw);

	void* mem = malloc(fileSize);
	SDL_RWseek(rw, 0, RW_SEEK_SET);	// Reset again from the beginning
	SDL_RWread(rw, mem, 1, fileSize);
	SDL_RWclose(rw);
	SDL_FreeRW(rw);

	// Write using Rapture FIO
	trap->WriteFileSync(f, mem, fileSize);
	trap->CloseFileSync(f);

	SDL_FreeSurface(screenSurf);
	free(mem);
}

Screenshot::Screenshot(const Screenshot& other) {
	strcpy(name, other.name);
	screenSurf = other.screenSurf;
}