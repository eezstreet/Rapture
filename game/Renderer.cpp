#include "tr_local.h"

Cvar* r_fullscreen = NULL;
Cvar* r_width = NULL;
Cvar* r_height = NULL;
Cvar* r_windowtitle = NULL;
Cvar* r_gamma = NULL;

namespace RenderCode {

	static SDL_Window *window;
	SDL_Renderer *renderer;
	static bool screenshotQueued = false;
	static string screenshotName = "";

	static vector<SDL_Texture*> texs = vector<SDL_Texture*>();

	static void InitCvars() {
		r_fullscreen = Cvar::Get<bool>("r_fullscreen", "Dictates whether the application runs in fullscreen mode.", Cvar::CVAR_ARCHIVE, false);
		r_width = Cvar::Get<int>("r_width", "Resolution: width", Cvar::CVAR_ARCHIVE, 1024);
		r_height = Cvar::Get<int>("r_height", "Resolution: height", Cvar::CVAR_ARCHIVE, 768);
		r_windowtitle = Cvar::Get<char*>("r_windowtitle", "Window title", Cvar::CVAR_ARCHIVE | Cvar::CVAR_ROM, "Rapture");
		r_gamma = Cvar::Get<float>("r_gamma", "Gamma", Cvar::CVAR_ARCHIVE, 1.0f);
	}

	void Initialize() {
		R_Printf("Initializing renderer\n");
		InitCvars();

		R_Printf("SDL_Init()\n");
		if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK) < 0) {
			printf("could not init SDL (error code: %s)\n", SDL_GetError());
			return;
		}

		R_Printf("SDL_CreateWindow()\n");
		window = SDL_CreateWindow(r_windowtitle->String(),
										SDL_WINDOWPOS_CENTERED, 
										SDL_WINDOWPOS_CENTERED,
										r_width->Integer(),
										r_height->Integer(),
										SDL_WINDOW_OPENGL | (r_fullscreen->Bool() ? 
										SDL_WINDOW_FULLSCREEN : SDL_WINDOW_SHOWN));
		if(window == NULL) {
			SDL_Quit();
			return;
		}

		renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
		if(renderer == NULL) {
			SDL_Quit();
			return;
		}

		if(r_gamma->Value() != 1.0f) {
			unsigned short ramp[256];
			SDL_CalculateGammaRamp(r_gamma->Value(), ramp);
			SDL_SetWindowGammaRamp(window, ramp, ramp, ramp);
		}

		R_Printf("ImageClass::InitImages()\n");
		ImageClass::InitImages();

		R_Printf("Initializing FreeImage\n");
		FreeImage_Initialise();
		R_Printf("FreeImage found, version %s\n", FreeImage_GetVersion());
		R_Printf("%s\n", FreeImage_GetCopyrightMessage());
	}

	void Restart() {
	}

	void Exit() {
		R_Printf("Shutting down FreeImage\n");
		FreeImage_DeInitialise();

		R_Printf("ImageClass::ShutdownImages()\n");
		ImageClass::ShutdownImages();

		R_Printf("gamma ramp down--\n");
		unsigned short ramp[256];
		SDL_CalculateGammaRamp(1.0f, ramp);
		SDL_SetWindowGammaRamp(window, ramp, ramp, ramp);
	}

	void BlankFrame() {
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
		SDL_RenderClear(renderer);
	}

	static void CreateScreenshot(const string& filename) {
		SDL_Surface* s = SDL_GetWindowSurface(window);
		unsigned int numPixels = s->w * s->h * s->format->BytesPerPixel;
		unsigned char * pixels = new (nothrow) unsigned char[numPixels];
		SDL_RenderReadPixels(renderer, &s->clip_rect, s->format->format, pixels, s->w * s->format->BytesPerPixel);
		SDL_Surface* screenshot = SDL_ConvertSurfaceFormat(
			SDL_CreateRGBSurfaceFrom(pixels, s->w, s->h, s->format->BitsPerPixel, s->w * s->format->BytesPerPixel, s->format->Rmask, s->format->Gmask, s->format->Bmask, s->format->Amask),
			SDL_PIXELFORMAT_RGB444, 0);

		SDL_RWops *rw = SDL_RWFromMem(pixels, numPixels);
		SDL_SaveBMP_RW(screenshot, rw, 0);
		auto size = rw->size(rw);
		ImageClass* img = new ImageClass(pixels, size);
		bool bSuccess = img->WriteToFile(filename);
		delete img;
		SDL_FreeRW(rw);
		delete pixels;
		if(bSuccess) {
			R_Printf("Screenshot: %s\n", filename.c_str());
		}
		else {
			R_Printf("Could not write %s\n", filename.c_str());
		}
	}

	void Display() {
		SDL_RenderPresent(renderer);
		if(screenshotQueued) {
			// We have a screenshot command queued up, so we need to handle it.
			CreateScreenshot(screenshotName);
			screenshotQueued = false;
		}
		for(auto it = texs.begin(); it != texs.end(); ++it) {
			SDL_DestroyTexture(*it);
		}
		texs.clear(); // clear out the texture pool
	}

	void* AddSurface(void* surf) {
		SDL_Surface* sdlsurf = (SDL_Surface*)surf;
		SDL_Texture* text = SDL_CreateTextureFromSurface(renderer, sdlsurf);
		SDL_RenderCopy(renderer, text, NULL, NULL);
		SDL_FreeSurface(sdlsurf);
		texs.push_back(text);
		return text;
	}

	void QueueScreenshot(const string& fileName, const string& extension) {
		screenshotQueued = true;
		if(fileName.length() <= 0) {
			string formatter = "screenshots/screenshot%04i";
			formatter.append(extension);
			char* s = (char*)Zone::Alloc(formatter.size()+1, Zone::TAG_RENDERER);
			int i = 0;
			for(int i = 0; i <= 9999; i++) {
				sprintf(s, formatter.c_str(), i);
				if(!File::Open(s, "r")) {
					break;
				}
			}
			screenshotName = s;
			Zone::FastFree(s, "renderer");
			return;
		}
		// Tell the renderer that we need to screenshot the next frame
		string thisIsMyFinalForm = fileName;
		auto lastDot = thisIsMyFinalForm.find_last_of('.');
		if(lastDot == thisIsMyFinalForm.npos) {
			// go ahead and assume .bmp
			thisIsMyFinalForm += extension;
		}
		screenshotName = thisIsMyFinalForm;
	}

	void* RegisterImage(const char* name) {
		return ImageClass::RegisterImage(name);
	}

	void DrawImage(void* image, float xPct, float yPct, float wPct, float hPct) {
		ImageClass::RenderImage(image, xPct, yPct, wPct, hPct);
	}
};