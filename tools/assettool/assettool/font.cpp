#include "assettool.h"
#include <SDL_ttf.h>

void ImportFontFileTTF(uint8_t** fontData, size_t* decompressedSize, int ptSize, int index) {
	if (ptSize <= 0) {
		DisplayMessageBox("Font import error", "Point size must be above 0.", MESSAGEBOX_ERROR);
		return;
	}
	if (fontData == nullptr) {
		DisplayMessageBox("Error", "Reference error in ImportFontFileTTF", MESSAGEBOX_ERROR);
		return;
	}

	const char* path = OpenFileDialog("All Files\0*.*\0TrueType Fonts (.ttf)\0*.ttf", 0);
	if (path[0] == '\0') {
		// User canceled dialog box
		return;
	}

	// This gets slightly tricky. We need to load the TTF font -twice-, once so we can check the index, and once so we can copy the data.
	// This sounds silly, but I blame SDL_
	SDL_RWops* pFile = SDL_RWFromFile(path, "rb+");
	if (pFile == nullptr) {
		DisplayMessageBox("Font import error", "Couldn't open the font file. Try running as administrator?", MESSAGEBOX_ERROR);
		return;
	}

	TTF_Font* pFont = TTF_OpenFontIndexRW(pFile, false, ptSize, index);
	if (pFont == nullptr) {
		DisplayMessageBox("Font import error", "Failed to read the font file. Maybe check the font face? (or it's an invalid TTF)", MESSAGEBOX_ERROR);
		SDL_RWclose(pFile);
		return;
	}

	SDL_RWseek(pFile, 0, RW_SEEK_SET);	// safety measure in case ttf ops screws with the r/w head
	size_t len = SDL_RWseek(pFile, 0, RW_SEEK_END);
	SDL_RWseek(pFile, 0, RW_SEEK_SET);

	*decompressedSize = len + sizeof(ComponentFont::FontHeader);

	if (*fontData != nullptr) {
		free(*fontData);
	}

	*fontData = (uint8_t*)malloc(*decompressedSize);
	SDL_RWread(pFile, *fontData, sizeof(uint8_t), *decompressedSize);

	TTF_CloseFont(pFont);
	SDL_RWclose(pFile);

	// NOTE: be sure to add the size of the header to the decompressed size after this function is complete!!
}

const char* previewText 
	= "THE QUICK BROWN FOX JUMPED OVER THE LAZY DOG\nthe quick brown fox jumped over the lazy dog\n0123456789\n!@#$%^&()-=_+\\|/\n,.<>[]{}`~";
SDL_Color previewColor = { 128, 128, 255, 128 };
void PreviewFont(uint8_t* fontData, size_t decompSize, int ptSize, int style, int face) {
	SDL_RWops* rw = SDL_RWFromMem(fontData, decompSize);
	if (rw == nullptr) {
		DisplayMessageBox("Font Preview Error", "Couldn't read the memory from asset file", MESSAGEBOX_ERROR);
		return;
	}
	TTF_Font* ttf = TTF_OpenFontIndexRW(rw, 0, ptSize, face);
	if (ttf == nullptr) {
		DisplayMessageBox("Font Preview Error", "Bad font face", MESSAGEBOX_ERROR);
		SDL_RWclose(rw);
		return;
	}

	TTF_SetFontStyle(ttf, style);
	SDL_Surface* pFontSurf = TTF_RenderText_Blended_Wrapped(ttf, previewText, previewColor, 1024);
	AlterPreviewImage(pFontSurf);
	previewVisible = true;

	SDL_FreeSurface(pFontSurf);
	TTF_CloseFont(ttf);
	SDL_RWclose(rw);
}