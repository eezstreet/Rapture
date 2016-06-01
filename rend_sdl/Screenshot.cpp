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
	File* f = trap->OpenFile(this->name, "wb+");
	trap->WritePlaintext(f, "blah");
	trap->CloseFile(f);

	// Now we want the full path
	char* fullPath = trap->FileSearchPath(this->name);
	SDL_SaveBMP(screenSurf, fullPath);

	SDL_FreeSurface(screenSurf);
}

Screenshot::Screenshot(const Screenshot& other) {
	strcpy(name, other.name);
	screenSurf = other.screenSurf;
}