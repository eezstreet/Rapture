#include "assettool.h"
#include <SDL_image.h>
#include <string>
#include <vector>
#include <algorithm>

#pragma warning(disable: 4996)

char fileNameBuffer[300];
void ImportImage(uint32_t** pixels, uint32_t* width, uint32_t* height) {
	if (pixels == nullptr) {
		DisplayMessageBox("Error", "Reference error in ImportImage", MESSAGEBOX_ERROR);
		return;
	}

	const char* path 
		= OpenFileDialog("All Files\0*.*\0Portable Network Graphics (.png)\0*.png\0Windows Bitmap Images (.bmp)\0*.bmp\0JPEG Images (.jpg)\0*.jpg", 0);
	if (path[0] == '\0') {
		return; // User canceled dialog box
	}

	// The image is not always in RGBA format, big example is GIF (web color) or PNG (backwards)
	SDL_Surface* imgSurf = IMG_Load(path);
	SDL_Surface* newSurf = SDL_ConvertSurfaceFormat(imgSurf, SDL_PIXELFORMAT_RGBA8888, 0);

	if (newSurf == nullptr) {
		DisplayMessageBox("Error", "Could not open image file", MESSAGEBOX_ERROR);
		return;
	}

	// Copy pixels over
	SDL_LockSurface(newSurf);
	
	*width = newSurf->w;
	*height = newSurf->h;
	if (*pixels != nullptr) {
		free(*pixels);
	}

	size_t imgTotalSize = *width * *height * sizeof(uint32_t);
	*pixels = (uint32_t*)malloc(imgTotalSize);
	memcpy(*pixels, newSurf->pixels, imgTotalSize);

	SDL_UnlockSurface(newSurf);

	SDL_FreeSurface(newSurf);
	SDL_FreeSurface(imgSurf);
}

void ImportRGBASprite(uint32_t** pixels, int numDirections, uint32_t* frameWidth, uint32_t* frameHeight, uint32_t* totalWidth, uint32_t* totalHeight) {
	if (pixels == nullptr) {
		DisplayMessageBox("Error", "Reference error in ImportRGBASprite", MESSAGEBOX_ERROR);
		return;
	}

	if (numDirections <= 0) { // We don't want to divide by the number of directions if it's zero..
		DisplayMessageBox("Import Error", "You need at least one direction in order to import files", MESSAGEBOX_ERROR);
		return;
	}

	const char* path = OpenFileDialog("All Files\0*.*\0Portable Network Graphics (.png)\0*.png\0Windows Bitmap Images (.bmp)\0*.bmp", OFFLAG_MULTISELECT);
	if (path[0] == '\0') {
		return;	// User canceled dialog box
	}

	SDL_Surface* primSurf = IMG_Load(path);
	SDL_Surface* pixelSurf = nullptr;
	if (*pixels != nullptr) {
		pixelSurf = SDL_CreateRGBSurfaceFrom(*pixels, *totalWidth, *totalHeight, 32, sizeof(uint32_t) * (*totalWidth), 
			0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
	}

	if (primSurf == nullptr) {
		// It's likely a list of files
		char* ppPath = (char*)(path + strlen(path) + 1);
		std::vector<std::string> vPaths;

		// First pass: determine total number of files
		while (ppPath && ppPath[0] != '\0') {
			sprintf(fileNameBuffer, "%s/%s", path, ppPath);
			std::string s = fileNameBuffer;
			vPaths.push_back(s);
			ppPath = (char*)(path + strlen(path) + 1);
		}

		if (vPaths.size() % numDirections) {
			if (pixelSurf != nullptr) {
				SDL_FreeSurface(pixelSurf);
			}
			DisplayMessageBox("Import Error", "Each direction needs an equal number of frames.", MESSAGEBOX_ERROR);
			return;
		}
		std::sort(vPaths.begin(), vPaths.end());

		// Calculate information from the first image that we loaded
		std::string firstFile = vPaths[0];
		primSurf = IMG_Load(firstFile.c_str());
		int frameCount = vPaths.size() / numDirections;

		if (primSurf == nullptr) {
			if (pixelSurf != nullptr) {
				SDL_FreeSurface(pixelSurf);
			}
			DisplayMessageBox("Import Error", "Couldn't load first file. (try running as Administrator)", MESSAGEBOX_ERROR);
			return;
		}
		if (*frameWidth <= 0) {
			*frameWidth = primSurf->w;
		}
		if (*frameHeight <= 0) {
			*frameHeight = primSurf->h;
		}
		*totalWidth = *totalWidth + (*frameWidth * frameCount);
		*totalHeight = *totalHeight + (*frameHeight * numDirections);

		SDL_Surface* outSurf = SDL_CreateRGBSurface(0, *totalWidth, *totalHeight, 32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
		SDL_Rect outRect = { 0, 0, *frameWidth, *frameHeight };
		if (pixelSurf != nullptr) {	// Blit previous spritesheet to this one
			SDL_Rect srcRect = { 0, 0, pixelSurf->w, pixelSurf->h };
			SDL_BlitSurface(pixelSurf, &srcRect, outSurf, &srcRect);
			SDL_FreeSurface(pixelSurf);
			outRect.x += pixelSurf->w;
		}
		uint32_t resetX = outRect.x;

		// Blit all spritesheets 
		int frameNum = 0;
		for (auto it = vPaths.begin()+1; it != vPaths.end(); ++it) {
			SDL_Surface* fileSurf = IMG_Load(it->c_str());
			if (fileSurf != nullptr) {
				SDL_BlitSurface(fileSurf, nullptr, outSurf, &outRect);
				SDL_FreeSurface(fileSurf);
			}
			if (frameNum >= frameCount) {
				outRect.y += *frameHeight;
				outRect.x = resetX;
				frameNum = 0;
			}
			else {
				outRect.x += *frameWidth;
				frameNum++;
			}
		}

		// Copy to pixels
		SDL_LockSurface(outSurf);
		if (*pixels != nullptr) {
			free(*pixels);
		}
		*pixels = (uint32_t*)malloc(sizeof(uint32_t) * outSurf->w * outSurf->h);
		memcpy(*pixels, outSurf->pixels, outSurf->h * outSurf->w * sizeof(uint32_t));

		SDL_UnlockSurface(outSurf);
		SDL_FreeSurface(outSurf);
	}
	else if (numDirections != 1) {
		if (pixelSurf != nullptr) {
			SDL_FreeSurface(pixelSurf);
		}
		DisplayMessageBox("Import Error", "Importing single files only allowed for unidirectional materials", MESSAGEBOX_ERROR);
		return;
	}
	else {
		// It's a one-direction, single file import
		// primSurf contains the image
		if (pixelSurf == nullptr) {
			*totalWidth = 0;
			*totalHeight = 0;
			*frameWidth = primSurf->w;
			*frameHeight = primSurf->h;
		}
		*totalWidth += primSurf->w;
		*totalHeight += primSurf->h;
		
		SDL_Surface* outSurf = SDL_CreateRGBSurface(0, *totalWidth, *totalHeight, 32,
			0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
		if (outSurf == nullptr) {
			return;
		}

		if (pixelSurf != nullptr) {
			SDL_BlitSurface(pixelSurf, nullptr, outSurf, nullptr);
		}
		SDL_Rect dstRect = { *totalWidth - primSurf->w, *totalHeight - primSurf->h, primSurf->w, primSurf->h };
		SDL_BlitSurface(primSurf, nullptr, outSurf, &dstRect);

		SDL_LockSurface(outSurf);

		if (*pixels != nullptr) {
			free(*pixels);
		}
		*pixels = (uint32_t*)malloc(sizeof(uint32_t) * *totalWidth * *totalHeight);
		memcpy(*pixels, outSurf->pixels, sizeof(uint32_t) * *totalWidth * *totalHeight);

		SDL_UnlockSurface(outSurf);
		SDL_FreeSurface(outSurf);
	}
}

void ImportMonochromeSprite(uint16_t** pixels, int numDirections, uint32_t* frameWidth, uint32_t* frameHeight, uint32_t* totalWidth, uint32_t* totalHeight) {
	DisplayMessageBox("Note", "Feature not yet implemented", MESSAGEBOX_INFO);
}

void ExportRGBASprite(uint32_t* pixels, uint32_t width, uint32_t height) {
	if (pixels == nullptr) {
		DisplayMessageBox("Info", "This material doesn't have any data for that spritesheet", MESSAGEBOX_INFO);
		return;
	}
	const char* path = SaveAsDialog("All Files\0*.*\0Portable Network Graphics (.png)\0*.png", "png");
	SDL_Surface* out = SDL_CreateRGBSurfaceFrom(pixels, width, height, 32, width * sizeof(uint32_t), 
		0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
	IMG_SavePNG(out, path);
	SDL_FreeSurface(out);
}

void ExportMonochromeSprite(uint16_t* pixels, uint32_t width, uint32_t height) {
	if (pixels == nullptr) {
		DisplayMessageBox("Info", "This material doesn't have any data for that spritesheet", MESSAGEBOX_INFO);
		return;
	}
	const char* path = SaveAsDialog("All Files\0*.*\0Portable Network Graphics (.png)\0*.png", "png");
	SDL_Surface* out = SDL_CreateRGBSurfaceFrom(pixels, width, height, 16, width * sizeof(uint16_t),
		0xFFFF, 0xFFFF, 0xFFFF, 0);
	IMG_SavePNG(out, path);
	SDL_FreeSurface(out);
}