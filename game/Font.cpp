#include "sys_local.h"

/* Font class */
Font::Font() {
	ptFont = nullptr;
}

Font::~Font() {
	if (ptFont != nullptr) {
		TTF_CloseFont(ptFont);
		ptFont = nullptr;
	}
}

void Font::LoadFont(const char* sFontFile, int iPointSize) {
	if (ptFont != nullptr) {
		R_Message(PRIORITY_WARNING, "WARNING: attempted to reload font %s\n", sFontFile);
		return;
	}

	// We need to load the actual file using FS and then run it through TTF.
	File* file = File::Open(sFontFile, "r");
	if (file == nullptr) {
		R_Message(PRIORITY_WARNING, "WARNING: could not read font %s\n", sFontFile);
		return;
	}

	size_t fileSize = file->GetSize();
	unsigned char* bytes = new unsigned char[fileSize];
	file->ReadBinary(bytes, 0, false);
	SDL_RWops* rw = SDL_RWFromMem(bytes, fileSize);
	ptFont = TTF_OpenFontRW(rw, 1, iPointSize);
	//delete[] bytes;
	if (!ptFont) {
		R_Message(PRIORITY_WARNING, "WARNING: could not read font %s: %s\n", sFontFile, SDL_GetError());
		ptFont = nullptr;
		return;
	}
	file->Close();
}

Font* Font::Register(const char* sFontFile, int iPointSize) {
	if (!FontMan) {
		R_Message(PRIORITY_ERROR, "Couldn't load %s (font manager not loaded)\n");
		return nullptr;
	}
	return FontMan->RegisterFont(sFontFile, iPointSize);
}
