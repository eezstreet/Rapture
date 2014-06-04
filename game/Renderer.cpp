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
	static SDL_Texture* renderTex = NULL;
	static unordered_map<string, SDL_Texture*> images;
	static int textFieldCount = 0;
#define MAX_TEXTRENDER 8
	static SDL_Texture* textFields[MAX_TEXTRENDER];

	static void InitCvars() {
		r_fullscreen = Cvar::Get<bool>("r_fullscreen", "Dictates whether the application runs in fullscreen mode.", Cvar::CVAR_ARCHIVE, false);
		r_width = Cvar::Get<int>("r_width", "Resolution: width", Cvar::CVAR_ARCHIVE, 1024);
		r_height = Cvar::Get<int>("r_height", "Resolution: height", Cvar::CVAR_ARCHIVE, 768);
		r_windowtitle = Cvar::Get<char*>("r_windowtitle", "Window title", Cvar::CVAR_ARCHIVE | Cvar::CVAR_ROM, "Rapture");
		r_gamma = Cvar::Get<float>("r_gamma", "Gamma", Cvar::CVAR_ARCHIVE, 1.0f);

		// Debug cvars
#ifdef _DEBUG
		r_imgdebug = Cvar::Get<bool>("r_imgdebug", "Draw lines around image bounds", 0, true);
#endif
	}

	void Initialize() {
		R_Printf("Viewlog init\n");
		Sys_InitViewlog();

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

		viewlog->TestViewlogShow();

		renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
		if(renderer == NULL) {
			SDL_Quit();
			return;
		}

		SDL_RendererInfo info;
		SDL_GetRendererInfo(renderer, &info);
		if(info.flags & SDL_RENDERER_SOFTWARE) {
			R_Printf("WARNING: Using software renderer due to hardware fallback. Performance will suffer.\n");
		}
		if(!(info.flags & SDL_RENDERER_TARGETTEXTURE)) {
			R_Error("ERROR: Renderer does not support render-to-texture. Game will not run.");
			return;
		}

		if(r_gamma->Value() != 1.0f) {
			unsigned short ramp[256];
			SDL_CalculateGammaRamp(r_gamma->Value(), ramp);
			SDL_SetWindowGammaRamp(window, ramp, ramp, ramp);
		}

		renderSurf = SDL_CreateRGBSurface(0, r_width->Integer(), r_height->Integer(), 32,  0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
		renderTex = SDL_CreateTexture( renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, r_width->Integer(), r_height->Integer() );
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);

		int flags = IMG_INIT_JPG|IMG_INIT_PNG;
		R_Printf("IMG_Init()\n");
		if(IMG_Init(flags)&flags != flags) {
			R_Printf("FAILED! %s\n", IMG_GetError());
		}

		SDL_SetRenderTarget(renderer, NULL);

		R_Printf("Init font\n");
		for(int i = 0; i < MAX_TEXTRENDER; i++) {
			textFields[i] = SDL_CreateTexture( renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, r_width->Integer(), r_height->Integer() );
		}
	}

	void Restart() {
		// TODO
	}

	void Exit(const bool bSilent) {
		SDL_DestroyTexture(renderTex);
		SDL_FreeSurface(renderSurf);

		for(int i = 0; i < MAX_TEXTRENDER; i++) {
			SDL_DestroyTexture(textFields[i]);
		}

		if(!bSilent) {
			R_Printf("IMG_Quit()\n");
		}
		IMG_Quit();
		for(auto it = images.begin(); it != images.end(); ++it) {
			SDL_DestroyTexture(it->second);
		}
		images.clear();

		if(!bSilent) {
			R_Printf("gamma ramp down--\n");
		}
		unsigned short ramp[256];
		SDL_CalculateGammaRamp(1.0f, ramp);
		SDL_SetWindowGammaRamp(window, ramp, ramp, ramp);

		SDL_DestroyWindow(window);
	}

	void BlankFrame() {
		SDL_RenderClear(renderer);
		textFieldCount = 0;
	}

	static void CreateScreenshot(const string& filename) {
		SDL_Surface* s = SDL_GetWindowSurface(window);
		unsigned int numPixels = s->w * s->h * s->format->BytesPerPixel;
		unsigned char * pixels = new (nothrow) unsigned char[numPixels];
		SDL_RenderReadPixels(renderer, &s->clip_rect, s->format->format, pixels, s->w * s->format->BytesPerPixel);
		SDL_Surface* screenshot = SDL_ConvertSurfaceFormat(
			SDL_CreateRGBSurfaceFrom(pixels, s->w, s->h, s->format->BitsPerPixel, s->w * s->format->BytesPerPixel, s->format->Rmask, s->format->Gmask, s->format->Bmask, s->format->Amask),
			SDL_PIXELFORMAT_RGB444, 0);

		File* f = File::Open(filename, "wb+");
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
		SDL_RenderPresent(renderer);
	}

	void AddSurface(void* surf) {
		SDL_Surface* sdlsurf = (SDL_Surface*)surf;
		SDL_BlitSurface(sdlsurf, NULL, renderSurf, NULL);
		SDL_FreeSurface(sdlsurf);
	}

	void BlendTexture(void* tex) {
		SDL_Texture* text = (SDL_Texture*)tex;
		SDL_SetRenderTarget(renderer, NULL);
		SDL_SetTextureBlendMode(text, SDL_BLENDMODE_BLEND);
		SDL_RenderCopy(renderer, text, NULL, NULL);
	}

	void* GetRenderer() {
		return (void*)renderer;
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
		SDL_Texture* text;

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
		text = SDL_CreateTextureFromSurface(renderer, perm);

		SDL_FreeSurface(temp);
		SDL_FreeSurface(perm);
		images[name] = text;
		return text;
	}

#ifdef _DEBUG
	void DebugImageDraw(SDL_Rect* rect) {
		SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255);
		SDL_RenderDrawLine(renderer, rect->x, rect->y, rect->x + rect->w, rect->y);
		SDL_RenderDrawLine(renderer, rect->x, rect->y, rect->x, rect->y + rect->h);
		SDL_RenderDrawLine(renderer, rect->x, rect->y + rect->h, rect->x + rect->w, rect->y + rect->h);
		SDL_RenderDrawLine(renderer, rect->x + rect->w, rect->y, rect->x + rect->w, rect->y + rect->h);
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
	}
#endif

	void R_DrawImage(SDL_Texture* image, int x, int y, int w, int h) {
		if(!image) {
			return;
		}
		SDL_Rect rect;
		rect.x = x; rect.y = y; rect.w = w; rect.h = h;
		SDL_RenderCopy(renderer, image, NULL, &rect);
#ifdef _DEBUG
		if(r_imgdebug->Bool()) {
			DebugImageDraw(&rect);
		}
#endif
	}

	void R_DrawImageClipped(SDL_Texture* image, SDL_Rect* spos, SDL_Rect* ipos) {
		if(!image) {
			return;
		}
		SDL_RenderCopy(renderer, image, ipos, spos);
#ifdef _DEBUG
		if(r_imgdebug->Bool()) {
			DebugImageDraw(spos);
		}
#endif
	}

	void DrawImage(void* image, float xPct, float yPct, float wPct, float hPct) {
		if(!image) {
			return;
		}
		SDL_Texture* img = reinterpret_cast<SDL_Texture*>(image);
		SDL_Rect wreckt;
		const int width = r_width->Integer();
		const int height = r_height->Integer();

		wreckt.x = (width * xPct)/100; wreckt.y = (height * yPct)/100;
		wreckt.w = (width * wPct)/100; wreckt.h = (height * hPct)/100;

		R_DrawImage(img, wreckt.x, wreckt.y, wreckt.w, wreckt.h);
	}

	void DrawImageAspectCorrection(void* image, float xPct, float yPct, float wPct, float hPct) {
		if(!image) {
			return;
		}
		SDL_Texture* img = reinterpret_cast<SDL_Texture*>(image);
		const float height = (float)r_height->Integer();
		const float width = (float)(height/(float)r_width->Integer());
		const float ycorrect = (float)(r_width->Integer()/height);

		SDL_Rect rect;
		rect.w = (width * wPct * r_width->Integer())/100;
		rect.h = (height * hPct)/100;
		rect.x = (r_width->Integer() * xPct)/100;
		rect.y = (height * hPct * ycorrect)/100;

		R_DrawImage(img, rect.x, rect.y, rect.w, rect.h);
	}

	void DrawImageClipped(void* image, float sxPct, float syPct, float swPct,
		float shPct, float ixPct, float iyPct, float iwPct, float ihPct) {
		if(!image) {
			return;
		}
		SDL_Texture* img = reinterpret_cast<SDL_Texture*>(image);
		const int sx = (int)(r_width->Integer() * sxPct)/100;
		const int sy = (int)(r_height->Integer() * syPct)/100;
		const int sw = (int)(r_width->Integer() * swPct)/100;
		const int sh = (int)(r_height->Integer() * shPct)/100;
		SDL_Rect screen;
		screen.x = sx; screen.y = sy; screen.w = sw; screen.h = sh;
		int iTexWidth, iTexHeight, iAccess, iFormat;
		SDL_QueryTexture(img, (Uint32*)&iFormat, &iAccess, &iTexWidth, &iTexHeight);
		const int ix = (int)(iTexWidth * ixPct)/100;
		const int iy = (int)(iTexHeight * iyPct)/100;
		const int iw = (int)(iTexWidth * iwPct)/100;
		const int ih = (int)(iTexHeight * ihPct)/100;
		SDL_Rect irect;
		irect.x = ix; irect.y = iy; irect.w = iw; irect.h = ih;
		R_DrawImageClipped(img, &screen, &irect);
	}

	void DrawImageAbs(void* image, int x, int y, int w, int h) {
		if(!image) {
			return;
		}
		SDL_Texture* img = reinterpret_cast<SDL_Texture*>(image);
		
		if(w == 0 && h == 0) {
			// Draw at image's width/height
			Uint32 format;
			int wx, hx, a;
			SDL_QueryTexture(img, &format, &a, &wx, &hx);
			R_DrawImage(img, x, y, wx, hx);
		} else {
			R_DrawImage(img, x, y, w, h);
		}
	}

	void DrawImageAbs(void* image, int x, int y) {
		DrawImageAbs(image, x, y, 0, 0);
	}

	void InitMaterials() {
		R_Printf("Initializing materials..\n");
		mats = new MaterialHandler();
	}

	void ShutdownMaterials() {
		R_Printf("Freeing materials..\n");
		delete mats;
	}

	void* RegisterMaterial(const char* name) {
		return (void*)mats->GetMaterial(name);
	}

	void SendMaterialToRenderer(void* ptMaterial, float x, float y) {
		Material *X = reinterpret_cast<Material*>(ptMaterial);
		X->SendToRenderer(x, y);
	}

	void RenderTextSolid(void* font, const char* text, int r, int g, int b) {
		Font* trueFont = (Font*)font;
		SDL_Color color;
		color.r = r; color.g = g; color.b = b; color.a = 255;
		SDL_Surface* surf = TTF_RenderText_Solid(trueFont->GetFont(), text, color);
		if(surf == NULL) {
			return;
		}

		if(textFieldCount++ >= MAX_TEXTRENDER) {
			return;
		}
		SDL_Surface* surf2 = SDL_ConvertSurfaceFormat(surf, SDL_PIXELFORMAT_ARGB8888, 0);
		if(textFields[textFieldCount]) SDL_DestroyTexture(textFields[textFieldCount]);
		textFields[textFieldCount] = SDL_CreateTextureFromSurface(renderer, surf2);
		SDL_RenderCopy(renderer, textFields[textFieldCount], NULL, &surf->clip_rect);
		SDL_FreeSurface(surf);
		SDL_FreeSurface(surf2);
	}

	void RenderTextShaded(void* font, const char* text, int br, int bg, int bb, int fr, int fg, int fb) {
		Font* trueFont = (Font*)font;
		SDL_Color colorForeground, colorBackground;
		colorBackground.r = br; colorBackground.g = bg; colorBackground.b = bb; colorBackground.a = 255;
		colorForeground.r = fr; colorForeground.g = fg; colorForeground.b = fb; colorForeground.a = 255;
		SDL_Surface* surf = TTF_RenderText_Shaded(trueFont->GetFont(), text, colorBackground, colorForeground);
		if(surf == NULL) {
			R_Printf("%s\n", SDL_GetError());
			return;
		}

		if(textFieldCount++ >= MAX_TEXTRENDER) {
			return;
		}
		SDL_Surface* surf2 = SDL_ConvertSurfaceFormat(surf, SDL_PIXELFORMAT_ARGB8888, 0);
		if(textFields[textFieldCount]) SDL_DestroyTexture(textFields[textFieldCount]);
		textFields[textFieldCount] = SDL_CreateTextureFromSurface(renderer, surf2);
		SDL_RenderCopy(renderer, textFields[textFieldCount], NULL, &surf->clip_rect);
		SDL_FreeSurface(surf);
		SDL_FreeSurface(surf2);
	}

	void RenderTextBlended(void* font, const char* text, int r, int g, int b) {
		Font* trueFont = (Font*)font;
		SDL_Color color;
		color.r = r; color.g = g; color.b = b; color.a = 255;
		SDL_Surface* surf = TTF_RenderText_Blended(trueFont->GetFont(), text, color);
		if(surf == NULL) {
			return;
		}

		if(textFieldCount++ >= MAX_TEXTRENDER) {
			return;
		}
		SDL_Surface* surf2 = SDL_ConvertSurfaceFormat(surf, SDL_PIXELFORMAT_ARGB8888, 0);
		if(textFields[textFieldCount]) SDL_DestroyTexture(textFields[textFieldCount]);
		textFields[textFieldCount] = SDL_CreateTextureFromSurface(renderer, surf2);
		SDL_RenderCopy(renderer, textFields[textFieldCount], NULL, &surf->clip_rect);
		SDL_FreeSurface(surf);
		SDL_FreeSurface(surf2);
	}
};