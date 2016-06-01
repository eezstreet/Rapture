#include "tr_local.h"

Texture::Texture(const char* szTexture) : bValid(false) {
	char* path = trap->FileSearchPath(szTexture);
	if (strlen(path) <= 0) {
		return;
	}

	SDL_Surface* temp = IMG_Load(path);
	if (temp == nullptr) {
		return;
	}

	SDL_Surface* temp2 = SDL_ConvertSurfaceFormat(temp, SDL_PIXELFORMAT_RGBA4444, 0);
	ptTexture = SDL_CreateTextureFromSurface(renderer, temp2);

	SDL_FreeSurface(temp);
	SDL_FreeSurface(temp2);
	bValid = true;
}

Texture::Texture(const unsigned int nW, const unsigned int nH) : bValid(false) {
	ptTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, nW, nH);
	bValid = true;
}

Texture::Texture(const Texture& other) {
	this->ptTexture = other.ptTexture;
	strcpy(this->name, other.name);
}

Texture::~Texture() {
	if (ptTexture != nullptr) {
		SDL_DestroyTexture(ptTexture);
	}
}

int Texture::Lock(unsigned int nX, unsigned int nY, unsigned int nW, unsigned int nH, void** pixels, int* pitch) {
	SDL_Rect lockRect = { nX, nY, nW, nH };
	if (lockRect.x == 0 && lockRect.x == lockRect.y && lockRect.x == lockRect.w && lockRect.x == lockRect.h) {
		return SDL_LockTexture(ptTexture, nullptr, pixels, pitch);
	}
	return SDL_LockTexture(ptTexture, &lockRect, pixels, pitch);
}

void Texture::Unlock() {
	SDL_UnlockTexture(ptTexture);
}

void Texture::Blend() {
	SDL_SetRenderTarget(renderer, nullptr);
	SDL_SetTextureBlendMode(ptTexture, SDL_BLENDMODE_BLEND);
	SDL_RenderCopy(renderer, ptTexture, nullptr, nullptr);
}

void Texture::FadeTexture() {
	if (fade.bShouldWeFade) {
		int color = 255 - ((fade.fadeTime - fade.currentTime) / ((float)fade.initialFadeTime - fade.fadeTime)*-255);
		SDL_SetTextureColorMod(ptTexture, color, color, color);
	}
	else {
		SDL_SetTextureColorMod(ptTexture, 255, 255, 255);
	}
}

void Texture::DrawImage(int nX, int nY, int nW, int nH) {
	SDL_Rect rect;
	rect.x = nX; rect.y = nY; rect.w = nW; rect.h = nH;

	FadeTexture();

	SDL_RenderCopy(renderer, ptTexture, nullptr, &rect);
}

void Texture::DrawImage(float fXPct, float fYPct, float fWPct, float fHPct) {
	SDL_Texture* tex = this->ptTexture;
	int w, h;
	bool fullscreen;

	trap->GetRenderProperties(&w, &h, &fullscreen);
	SDL_Rect rect = { (w * fXPct) / 100, (h * fYPct) / 100, (w * fWPct) / 100, (h * fHPct) / 100 };
	
	FadeTexture();
	SDL_RenderCopy(renderer, tex, nullptr, &rect);
}

void Texture::DrawImageAspectCorrection(float fXPct, float fYPct, float fWPct, float fHPct) {
	SDL_Texture* tex = this->ptTexture;
	int w, h;
	bool fullscreen;
	float fAspect;

	trap->GetRenderProperties(&w, &h, &fullscreen);
	fAspect = w / (float)h;
	SDL_Rect rect = { (w * fXPct) / 100, (h * fHPct * fAspect) / 100, (w * fWPct * w) / 100, (h * fHPct) / 100 };

	FadeTexture();
	SDL_RenderCopy(renderer, tex, nullptr, &rect);
}

void Texture::DrawImageClipped(float sxPct, float syPct, float swPct, float shPct, float ixPct, float iyPct, float iwPct, float ihPct) {
	SDL_Texture* tex = this->ptTexture;
	int w, h;
	bool fullscreen;

	trap->GetRenderProperties(&w, &h, &fullscreen);

	SDL_Rect screenRect = { w * sxPct / 100, h * syPct / 100, w * swPct / 100, h * shPct / 100 };
	
	Uint32 iFormat;
	int iTexWidth, iTexHeight, iAccess;
	SDL_QueryTexture(tex, &iFormat, &iAccess, &iTexWidth, &iTexHeight);

	SDL_Rect imageRect = { iTexWidth * ixPct / 100, iTexHeight * iyPct / 100, iTexWidth * iwPct / 100, iTexHeight * ihPct / 100 };
	
	FadeTexture();
	SDL_RenderCopy(renderer, tex, &imageRect, &screenRect);
}

void Texture::DrawImageClipped(SDL_Rect* spos, SDL_Rect* ipos) {
	FadeTexture();
	SDL_RenderCopy(renderer, ptTexture, ipos, spos);
}

void Texture::DrawAbs(int nX, int nY, int nW, int nH) {
	if (nW == 0 && nH == 0) {
		// Draw at image's width/height
		Uint32 format;
		int wx, hx, a;
		SDL_QueryTexture(ptTexture, &format, &a, &wx, &hx);
		DrawImage(nX, nY, wx, hx);
	}
	else {
		DrawImage(nX, nY, nW, nH);
	}
}

void Texture::DrawAbsClipped(int sX, int sY, int sW, int sH, int iX, int iY, int iW, int iH) {
	if (sW == 0 && sH == 0) {
		// Use image data
		sW = iW;
		sH = iH;
	}
	SDL_Rect imgRect, scnRect;
	imgRect.x = iX; imgRect.y = iY; imgRect.w = iW; imgRect.h = iH;
	scnRect.x = sX; scnRect.y = sY; scnRect.w = sW; scnRect.h = sH;

	DrawImageClipped(&scnRect, &imgRect);
}

// Render Exports are really just wrappers for a lot of the texture functions
Texture* RenderExport::RegisterTexture(const char* szTexture) {
	return ptTexMan->RegisterTexture(szTexture);
}

Texture* RenderExport::RegisterBlankTexture(const unsigned int nW, const unsigned int nH) {
	return ptTexMan->RegisterTexture(new Texture(nW, nH));
}

int RenderExport::LockStreamingTexture(Texture* ptTexture, unsigned int nX, unsigned int nY, unsigned int nW, unsigned int nH, void** pixels, int* pitch) {
	if (ptTexture != nullptr) {
		return ptTexture->Lock(nX, nY, nW, nH, pixels, pitch);
	}
	return -1;
}

void RenderExport::UnlockStreamingTexture(Texture* ptTexture) {
	if (ptTexture != nullptr) {
		ptTexture->Unlock();
	}
}

void RenderExport::BlendTexture(Texture* ptTexture) {
	if (ptTexture != nullptr) {
		ptTexture->Blend();
	}
}

void RenderExport::DrawImageAbs(Texture* ptTexture, int nX, int nY, int nW, int nH) {
	if (ptTexture != nullptr) {
		ptTexture->DrawAbs(nX, nY, nW, nH);
	}
}

void RenderExport::DrawImageAbsClipped(Texture* ptTexture, int sX, int sY, int sW, int sH, int iX, int iY, int iW, int iH) {
	if (ptTexture != nullptr) {
		ptTexture->DrawAbsClipped(sX, sY, sW, sH, iX, iY, iW, iH);
	}
}

void RenderExport::DrawImage(Texture* ptTexture, float xPct, float yPct, float wPct, float hPct) {
	if (ptTexture != nullptr) {
		ptTexture->DrawImage(xPct, yPct, wPct, hPct);
	}
}

void RenderExport::DrawImageAspectCorrection(Texture* ptTexture, float xPct, float yPct, float wPct, float hPct) {
	if (ptTexture != nullptr) {
		ptTexture->DrawImageAspectCorrection(xPct, yPct, wPct, hPct);
	}
}

void RenderExport::DrawImageClipped(Texture* ptTexture, float sxPct, float syPct, float swPct, float shPct, float ixPct, float iyPct, float iwPct, float ihPct) {
	if (ptTexture != nullptr) {
		ptTexture->DrawImageClipped(sxPct, syPct, swPct, shPct, ixPct, iyPct, iwPct, ihPct);
	}
}