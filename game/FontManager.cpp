#include "sys_local.h"

FontManager* FontMan = nullptr;

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

TTF_Font* FontManager::GetFontTTF(Font* ptFont) {
	return ptFont->GetFont();
}

void* EXPORT_RegisterFont(const char* sFontFile, int iPointSize) {
	return FontMan->RegisterFont(sFontFile, iPointSize);
}