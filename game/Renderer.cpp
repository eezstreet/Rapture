#include "tr_local.h"
/*
namespace RenderCode {
	static bool screenshotQueued = false;
	static string screenshotName = "";

	void Display() {
		AnimationManager::Animate();
		if(screenshotQueued) {
			CreateScreenshot(screenshotName);
			screenshotQueued = false;
		}
	}

	void BlendTexture(void* tex) {
		if(gl_mode->Bool() == false) {
			
		}
	}

	void* CreateFullscreenTexture() {
		if(gl_mode->Bool() == false) {
			return SDL_CreateTexture(renderTarget.renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, r_width->Integer(), r_height->Integer());
		} else {
			GLuint tex;
			glGenTextures(1, &tex);
			return (void*)tex;
		}
	}

	void QueueScreenshot(const string& fileName, const string& extension) {
		screenshotQueued = true;
		if(fileName.length() <= 0) {
			string formatter = "screenshots/screenshot%04i";
			formatter.append(extension);
			char* s = (char*)Zone::Alloc(formatter.size()+1, Zone::TAG_RENDERER);
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

	Image* RegisterImage(const char* name) {
		if(gl_mode->Bool() == false) {
			SDL_Surface* temp;
			SDL_Surface* perm;
			SDL_Texture* text;

			auto it = images.find(name);
			if(it != images.end()) {
				return (Image*)it->second;
			}

			string path = ResolveImagePath(name);
			if(path.length() <= 0) {
				return nullptr;
			}

			temp = IMG_Load(path.c_str());
			if(temp == nullptr) {
				return (Image*)temp;
			}

			perm = SDL_ConvertSurfaceFormat(temp, SDL_PIXELFORMAT_RGBA4444, 0);
			text = SDL_CreateTextureFromSurface(renderTarget.renderer, perm);

			SDL_FreeSurface(temp);
			SDL_FreeSurface(perm);
			images[name] = text;
			return (Image*)text;
		}
	}

#ifdef _DEBUG
	void DebugImageDraw(SDL_Rect* rect) {
		if(gl_mode->Bool() == false) {
			SDL_SetRenderDrawColor(renderTarget.renderer, 255, 0, 255, 255);
			SDL_RenderDrawLine(renderTarget.renderer, rect->x, rect->y, rect->x + rect->w, rect->y);
			SDL_RenderDrawLine(renderTarget.renderer, rect->x, rect->y, rect->x, rect->y + rect->h);
			SDL_RenderDrawLine(renderTarget.renderer, rect->x, rect->y + rect->h, rect->x + rect->w, rect->y + rect->h);
			SDL_RenderDrawLine(renderTarget.renderer, rect->x + rect->w, rect->y, rect->x + rect->w, rect->y + rect->h);
			SDL_SetRenderDrawColor(renderTarget.renderer, 0, 0, 0, 0);
		}
	}
#endif

	void R_DrawImage(SDL_Texture* image, int x, int y, int w, int h) {
		if(!image) {
			return;
		}
		if(gl_mode->Bool() == false) {
			SDL_Rect rect;
			rect.x = x; rect.y = y; rect.w = w; rect.h = h;

			// Handle fade to black
			if(bShouldWeFade) {
				int color = 255 - ((fadeTime - iCurrentTime) / ((float)initialFadeTime - fadeTime)*-255);
				SDL_SetTextureColorMod(image, color, color, color);
			} else {
				SDL_SetTextureColorMod(image, 255, 255, 255);
			}

			// Copy the texture
			SDL_RenderCopy(renderTarget.renderer, image, nullptr, &rect);
#ifdef _DEBUG
			if(r_imgdebug->Bool()) {
				DebugImageDraw(&rect);
			}
#endif
		}
	}

	void R_DrawImageClipped(SDL_Texture* image, SDL_Rect* spos, SDL_Rect* ipos) {
		if(!image) {
			return;
		}

		if(gl_mode->Bool() == false) {
			// Handle fade to black
			if(bShouldWeFade) {
				int color = 255 - ((fadeTime - iCurrentTime) / ((float)initialFadeTime - fadeTime)*-255);
				SDL_SetTextureColorMod(image, color, color, color);
			} else {
				SDL_SetTextureColorMod(image, 255, 255, 255);
			}

			SDL_RenderCopy(renderTarget.renderer, image, ipos, spos);
#ifdef _DEBUG
			if(r_imgdebug->Bool()) {
				DebugImageDraw(spos);
			}
#endif
		}
	}

	void DrawImage(Image* image, float xPct, float yPct, float wPct, float hPct) {
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

	void DrawImageAspectCorrection(Image* image, float xPct, float yPct, float wPct, float hPct) {
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

	void DrawImageClipped(Image* image, float sxPct, float syPct, float swPct,
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

	void DrawImageAbs(Image* image, int x, int y, int w, int h) {
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

	void DrawImageAbs(Image* image, int x, int y) {
		DrawImageAbs(image, x, y, 0, 0);
	}

	void DrawImageAbsClipped(Image* image, int sX, int sY, int sW, int sH, int iX, int iY, int iW, int iH) {
		if(!image) {
			return;
		}
		if(sW == 0 && sH == 0) {
			// Use image data
			sW = iW;
			sH = iH;
		}
		SDL_Texture* img = reinterpret_cast<SDL_Texture*>(image);
		SDL_Rect imgRect, scnRect;
		imgRect.x = iX; imgRect.y = iY; imgRect.w = iW; imgRect.h = iH;
		scnRect.x = sX; scnRect.y = sY; scnRect.w = sW; scnRect.h = sH;
		
		R_DrawImageClipped(img, &scnRect, &imgRect);
	}

	void DrawImageAbsClipped(Image* image, int sX, int sY, int iX, int iY, int iW, int iH) {
		DrawImageAbsClipped(image, sX, sY, 0, 0, iX, iY, iW, iH);
	}

	void RenderTextSolid(Font* font, const char* text, int r, int g, int b) {
		if(gl_mode->Bool() == false) {
			SDL_Color color;
			color.r = r; color.g = g; color.b = b; color.a = 255;
			SDL_Surface* surf = TTF_RenderText_Solid(font->GetFont(), text, color);
			if(surf == nullptr) {
				return;
			}

			if(textFieldCount++ >= MAX_TEXTRENDER) {
				return;
			}
			SDL_Surface* surf2 = SDL_ConvertSurfaceFormat(surf, SDL_PIXELFORMAT_ARGB8888, 0);
			if(textFields[textFieldCount]) SDL_DestroyTexture(textFields[textFieldCount]);
			textFields[textFieldCount] = SDL_CreateTextureFromSurface(renderTarget.renderer, surf2);
			SDL_RenderCopy(renderTarget.renderer, textFields[textFieldCount], nullptr, &surf->clip_rect);
			SDL_FreeSurface(surf);
			SDL_FreeSurface(surf2);
		}
	}

	void RenderTextShaded(Font* font, const char* text, int br, int bg, int bb, int fr, int fg, int fb) {
		if(gl_mode->Bool() == false) {
			SDL_Color colorForeground, colorBackground;
			colorBackground.r = br; colorBackground.g = bg; colorBackground.b = bb; colorBackground.a = 255;
			colorForeground.r = fr; colorForeground.g = fg; colorForeground.b = fb; colorForeground.a = 255;
			SDL_Surface* surf = TTF_RenderText_Shaded(font->GetFont(), text, colorBackground, colorForeground);
			if(surf == nullptr) {
				R_Message(PRIORITY_ERROR, "%s\n", SDL_GetError());
				return;
			}

			if(textFieldCount++ >= MAX_TEXTRENDER) {
				return;
			}
			SDL_Surface* surf2 = SDL_ConvertSurfaceFormat(surf, SDL_PIXELFORMAT_ARGB8888, 0);
			if(textFields[textFieldCount]) SDL_DestroyTexture(textFields[textFieldCount]);
			textFields[textFieldCount] = SDL_CreateTextureFromSurface(renderTarget.renderer, surf2);
			SDL_RenderCopy(renderTarget.renderer, textFields[textFieldCount], nullptr, &surf->clip_rect);
			SDL_FreeSurface(surf);
			SDL_FreeSurface(surf2);
		}
	}

	void RenderTextBlended(Font* font, const char* text, int r, int g, int b) {
		if(gl_mode->Bool() == false) {
			SDL_Color color;
			color.r = r; color.g = g; color.b = b; color.a = 255;
			SDL_Surface* surf = TTF_RenderText_Blended(font->GetFont(), text, color);
			if(surf == nullptr) {
				return;
			}

			if(textFieldCount++ >= MAX_TEXTRENDER) {
				return;
			}
			SDL_Surface* surf2 = SDL_ConvertSurfaceFormat(surf, SDL_PIXELFORMAT_ARGB8888, 0);
			if(textFields[textFieldCount]) SDL_DestroyTexture(textFields[textFieldCount]);
			textFields[textFieldCount] = SDL_CreateTextureFromSurface(renderTarget.renderer, surf2);
			SDL_RenderCopy(renderTarget.renderer, textFields[textFieldCount], nullptr, &surf->clip_rect);
			SDL_FreeSurface(surf);
			SDL_FreeSurface(surf2);
		}
	}


	
};*/