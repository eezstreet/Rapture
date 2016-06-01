#include "tr_local.h"

TextManager* ptText = nullptr;

void TextManager::RenderTextSolid(Font* font, const char* text, int r, int g, int b) {
	SDL_Color color = { r, g, b, 255 };
	SDL_Surface* surf = TTF_RenderText_Solid(trap->GetFontTTF(font), text, color);
	if (surf == nullptr) {
		return;
	}

	SDL_Surface* surf2 = SDL_ConvertSurfaceFormat(surf, SDL_PIXELFORMAT_ARGB8888, 0);
	SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf2);
	SDL_RenderCopy(renderer, tex, nullptr, &surf->clip_rect);
	SDL_FreeSurface(surf);
	SDL_FreeSurface(surf2);

	// Cache the texture so we can kill it after this frame
	vTextFields.push_back(tex);
}

void TextManager::RenderTextShaded(Font* font, const char* text, int br, int bg, int bb, int fr, int fg, int fb) {
	SDL_Color cB = { br, bg, bb, 255 };
	SDL_Color cF = { fr, fg, fb, 255 };
	SDL_Surface* surf = TTF_RenderText_Shaded(trap->GetFontTTF(font), text, cF, cB);
	if (surf == nullptr) {
		return;
	}

	SDL_Surface* surf2 = SDL_ConvertSurfaceFormat(surf, SDL_PIXELFORMAT_ARGB8888, 0);
	SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf2);
	SDL_RenderCopy(renderer, tex, nullptr, &surf->clip_rect);
	SDL_FreeSurface(surf);
	SDL_FreeSurface(surf2);

	// Cache the texture so we can kill it after this frame
	vTextFields.push_back(tex);
}

void TextManager::RenderTextBlended(Font* font, const char* text, int r, int g, int b) {
	SDL_Color color = { r, g, b, 255 };
	SDL_Surface* surf = TTF_RenderText_Blended(trap->GetFontTTF(font), text, color);
	if (surf == nullptr) {
		return;
	}

	SDL_Surface* surf2 = SDL_ConvertSurfaceFormat(surf, SDL_PIXELFORMAT_ARGB8888, 0);
	SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf2);
	SDL_RenderCopy(renderer, tex, nullptr, &surf->clip_rect);
	SDL_FreeSurface(surf);
	SDL_FreeSurface(surf2);

	// Cache the texture so we can kill it after this frame
	vTextFields.push_back(tex);
}

void TextManager::ResetFrame() {
	for (auto it = vTextFields.begin(); it != vTextFields.end(); it++) {
		SDL_DestroyTexture(*it);
	}
	vTextFields.clear();
}

TextManager::TextManager() {}
TextManager::~TextManager() {
	for (auto it = vTextFields.begin(); it != vTextFields.end(); it++) {
		SDL_DestroyTexture(*it);
	}
	vTextFields.clear();
}