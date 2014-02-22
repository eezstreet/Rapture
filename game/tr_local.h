#pragma once
#include "sys_local.h"
#include "tr_shared.h"
#include <SDL.h>
#include <SDL_image.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include <FreeImage.h>

extern Cvar* r_fullscreen;
extern Cvar* r_width;
extern Cvar* r_height;
extern Cvar* r_windowtitle;
extern Cvar* r_gamma;

void CreateConsole();
void DestroyConsole();
void ShowConsole();
void HideConsole();

class ImageClass {
private:
	FREE_IMAGE_FORMAT DetermineFileFormat(const string& filename, bool bJustExtension = false);
	unsigned char *pixBuffer;
	unsigned int numPixInBuffer;

	FIBITMAP* image;
	FREE_IMAGE_FORMAT format;

	bool ParseImage(const string& filename);

	static unordered_map<string, ImageClass*> registeredImages;
	static unordered_map<string, FREE_IMAGE_FORMAT> formats;
public:
	ImageClass(unsigned char *pixels, unsigned int numPixels);
	ImageClass();
	bool WriteToFile(const string& filename);

	static void* RegisterImage(const string& filename);
	static void RenderImage(void* image, float xPct, float yPct, float wPct, float hPct);
	static void InitImages();
	static void ShutdownImages();
};