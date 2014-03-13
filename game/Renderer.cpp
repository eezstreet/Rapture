#include "tr_local.h"

Cvar* r_fullscreen = NULL;
Cvar* r_width = NULL;
Cvar* r_height = NULL;
Cvar* r_windowtitle = NULL;
Cvar* r_gamma = NULL;

#ifdef _DEBUG
Cvar* r_imgdebug = NULL;
#endif

namespace RenderCode {

	static SDL_Window *window;
	SDL_Renderer *renderer;
	static bool screenshotQueued = false;
	static string screenshotName = "";

	static SDL_Surface* renderSurf = NULL;
	static unordered_map<string, SDL_Surface*> images;

	static void InitCvars() {
		r_fullscreen = Cvar::Get<bool>("r_fullscreen", "Dictates whether the application runs in fullscreen mode.", Cvar::CVAR_ARCHIVE, false);
		r_width = Cvar::Get<int>("r_width", "Resolution: width", Cvar::CVAR_ARCHIVE, 1024);
		r_height = Cvar::Get<int>("r_height", "Resolution: height", Cvar::CVAR_ARCHIVE, 768);
		r_windowtitle = Cvar::Get<char*>("r_windowtitle", "Window title", Cvar::CVAR_ARCHIVE | Cvar::CVAR_ROM, "Rapture");
		r_gamma = Cvar::Get<float>("r_gamma", "Gamma", Cvar::CVAR_ARCHIVE, 1.0f);

		// Debug cvars
#ifdef _DEBUG
		r_imgdebug = Cvar::Get<bool>("r_imgdebug", "Draw lines around image bounds", 0, false);
#endif
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

		renderSurf = SDL_CreateRGBSurface(0, r_width->Integer(), r_height->Integer(), 32,  0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

		int flags = IMG_INIT_JPG|IMG_INIT_PNG;
		R_Printf("IMG_Init()\n");
		if(IMG_Init(flags)&flags != flags) {
			R_Printf("FAILED! %s\n", IMG_GetError());
		}

	}

	void Restart() {
	}

	void Exit() {
		R_Printf("IMG_Quit()\n");
		IMG_Quit();

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

		File* f = File::Open(filename, "wb+"); // this is pretty hack
		f->WritePlaintext("blah");
		f->Close();
		const string path = File::GetFileSearchPath(filename);
		SDL_SaveBMP(screenshot, path.c_str()); 
		if(true) {
			R_Printf("Screenshot: %s\n", filename.c_str());
		}
		else {
			R_Printf("Could not write %s\n", filename.c_str());
		}

		delete pixels;
		SDL_FreeSurface(s);
		SDL_FreeSurface(screenshot);
	}

	void Display() {
		SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, renderSurf);
		SDL_RenderCopy(renderer, tex, NULL, NULL);
		SDL_RenderPresent(renderer);
		SDL_FillRect(renderSurf, NULL, 0x00000000);
		if(screenshotQueued) {
			// We have a screenshot command queued up, so we need to handle it.
			CreateScreenshot(screenshotName);
			screenshotQueued = false;
		}
		SDL_DestroyTexture(tex);
	}

	void AddSurface(void* surf) {
		SDL_Surface* sdlsurf = (SDL_Surface*)surf;
		SDL_BlitSurface(sdlsurf, NULL, renderSurf, NULL);
		SDL_FreeSurface(sdlsurf);
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

	string ResolveImagePath(const char *path) {
		string pathret = File::GetFileSearchPath(path);
		if(pathret.length() > 0) {
			return pathret;
		}
		// Try next extension -- png
		pathret = stripextension(path);
		pathret += ".png";
		pathret = File::GetFileSearchPath(pathret);
		if(pathret.length() > 0) {
			return pathret;
		}
		// Next -- .jpeg
		pathret = stripextension(path);
		pathret += ".jpg";
		pathret = File::GetFileSearchPath(pathret);
		if(pathret.length() > 0) {
			return pathret;
		}
		// Maybe JPG failed? Try .jpeg
		pathret = stripextension(path);
		pathret += ".jpeg";
		pathret = File::GetFileSearchPath(pathret);
		if(pathret.length() > 0) {
			return pathret;
		}
		// Last resort...BMP.
		pathret = stripextension(path);
		pathret += ".bmp";
		pathret = File::GetFileSearchPath(pathret);
		
		// Returns either the correct file (with BMP extension) or nothing at all. Clever, eh?
		return pathret;
	}

	void* RegisterImage(const char* name) {
		SDL_Surface* temp;
		SDL_Surface* perm;

		auto it = images.find(name);
		if(it != images.end()) {
			return (void*)it->second;
		}

		string path = ResolveImagePath(name);
		if(path.length() <= 0) {
			return NULL;
		}
		
		temp = IMG_Load(path.c_str());
		if(temp == NULL) {
			return temp;
		}

		perm = SDL_ConvertSurfaceFormat(temp, SDL_PIXELFORMAT_RGBA4444, 0);
		return perm;
	}

#ifdef _DEBUG
	void DebugImageDraw(SDL_Rect* rect) {
		SDL_RenderDrawLine(renderer, rect->x, rect->y, rect->x + rect->w, rect->y);
		SDL_RenderDrawLine(renderer, rect->x, rect->y, rect->x, rect->y + rect->h);
		SDL_RenderDrawLine(renderer, rect->x, rect->y + rect->h, rect->x + rect->w, rect->y + rect->h);
		SDL_RenderDrawLine(renderer, rect->x + rect->w, rect->y, rect->x + rect->w, rect->y + rect->h);
	}
#endif

	void DrawImage(void* image, float xPct, float yPct, float wPct, float hPct) {
		if(!image) {
			return;
		}
		SDL_Surface* img = reinterpret_cast<SDL_Surface*>(image);
		// These IGNORE the blitting procedure which is pretty ugly, need to rewrite the renderer to use commands instead of surfs :<
		SDL_Rect wreckt;
		const int width = r_width->Integer();
		const int height = r_height->Integer();

		wreckt.x = (width * xPct)/100; wreckt.y = (height * yPct)/100;
		wreckt.w = (width * wPct)/100; wreckt.h = (height * hPct)/100;

		//wreckt.x += wreckt.w / 2;
		//wreckt.y += wreckt.h / 2; // >_>! SDL centers these. bad.
		//SDL_BlitSurface(img, &wreckt, renderSurf, &wreckt);
		SDL_BlitScaled(img, &img->clip_rect, renderSurf, &wreckt);
#ifdef _DEBUG
		if(r_imgdebug->Bool()) {
			DebugImageDraw(&wreckt);
		}
#endif
	}
};