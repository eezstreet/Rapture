#include "sys_local.h"

FontManager* FontMan = nullptr;

/* Font class */
Font::Font() {
	ptFont = nullptr;
}

Font::~Font() {
	if(ptFont != nullptr) {
		TTF_CloseFont(ptFont);
		ptFont = nullptr;
	}
}

void Font::LoadFont(const char* sFontFile, int iPointSize) {
	if(ptFont != nullptr) {
		R_Message(PRIORITY_WARNING, "WARNING: attempted to reload font %s\n", sFontFile);
		return;
	}

	// We need to load the actual file using FS and then run it through TTF.
	File* file = File::Open(sFontFile, "r");
	if(file == nullptr) {
		R_Message(PRIORITY_WARNING, "WARNING: could not read font %s\n", sFontFile);
		return;
	}

	size_t fileSize = file->GetSize();
	unsigned char* bytes = new unsigned char[fileSize];
	file->ReadBinary(bytes, 0, false);
	SDL_RWops* rw = SDL_RWFromMem(bytes, fileSize);
	ptFont = TTF_OpenFontRW(rw, 1, iPointSize);
	//delete[] bytes;
	if(!ptFont) {
		R_Message(PRIORITY_WARNING, "WARNING: could not read font %s: %s\n", sFontFile, SDL_GetError());
		ptFont = nullptr;
		return;
	}
	file->Close();
}

Font* Font::Register(const char* sFontFile, int iPointSize) {
	if(!FontMan) {
		R_Message(PRIORITY_ERROR, "Couldn't load %s (font manager not loaded)\n");
		return nullptr;
	}
	return FontMan->RegisterFont(sFontFile, iPointSize);
}

/* FontManager class */
FontManager::FontManager() {
	auto result = TTF_Init();
	if(result == -1) {
		R_Error("Font Manager: %s\n", TTF_GetError());
		return;
	}
}

FontManager::~FontManager() {
	Zone::FreeAll("font");
	TTF_Quit();
}

Font* FontManager::RegisterFont(const char* sFontFile, int iPointSize) {
	Font* ptFont = Zone::New<Font>(Zone::TAG_FONT);
	ptFont->LoadFont(sFontFile, iPointSize);
	return ptFont;
}

void* EXPORT_RegisterFont(const char* sFontFile, int iPointSize) {
	return FontMan->RegisterFont(sFontFile, iPointSize);
}