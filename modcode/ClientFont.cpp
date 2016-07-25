#include "ClientFont.h"

#define FONT_TIMEOUTLEN		5000		// If fonts take longer than this amount of time, something is bad
#define FONT_ASSETNAME		"fonts"		// All fonts used by client code are located in this asset file

namespace ClientFont {
	// Component names for fonts
	static const char* fontNames[FONT_MAX] = {
		"consolas",		/*  FONT_CONSOLAS */
		"segoeui",		/*  FONT_SEGOEUI  */
	};

	static Font* fontHandles[FONT_MAX] {0};
	static int numFontsInitialized = 0;

	// Called when a font is retrieved from assets
	void ClientFontCallback(const char* fontName, Font* fontFile) {
		// Search for the font that matches our component name
		for (int i = 0; i < FONT_MAX; i++) {
			if (!stricmp(fontName, fontNames[i])) {
				fontHandles[i] = fontFile;
				numFontsInitialized++;
				break;
			}
		}
	}

	// Queues up all of the necessary fonts
	void Initialize() {
		char fontURIBuffer[COMP_NAMELEN + ASSET_NAMELEN]{0};
		for (int i = 0; i < FONT_MAX; i++) {
			sprintf(fontURIBuffer, "asset://%s/%s", FONT_ASSETNAME, fontNames[i]);
			trap->RegisterFontAsync(fontURIBuffer, ClientFontCallback);
			memset(fontURIBuffer, 0, sizeof(fontURIBuffer));
		}
	}

	// Blocks until all fonts are finished loading
	void FinishInitialization() {
		int startTicks = trap->GetTicks();
		int currentTicks = startTicks;
		while (numFontsInitialized < FONT_MAX && currentTicks - startTicks < FONT_TIMEOUTLEN) {
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			currentTicks = trap->GetTicks();
		}
		if (numFontsInitialized < FONT_MAX) {
			trap->printf(PRIORITY_ERROR, "%i fonts failed to load.\n", FONT_MAX - numFontsInitialized);
		}
	}

	// Get me this font, my man!
	Font* RetrieveFont(const Fonts font) {
		if (font == FONT_MAX) {
			return nullptr;
		}
		return fontHandles[font];
	}
}

