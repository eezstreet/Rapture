#include "tr_local.h"

namespace RenderCode {
	Cvar* r_fullscreen = NULL;
	Cvar* r_width = NULL;
	Cvar* r_height = NULL;
	Cvar* r_hardware = NULL;
	Cvar* r_windowtitle = NULL;
	Cvar* r_gamma = NULL;

	static SDL_Window *window;
	static SDL_Renderer *renderer;

	static void InitCvars() {
		r_fullscreen = Cvar::Get<bool>("r_fullscreen", "Dictates whether the application runs in fullscreen mode.", Cvar::CVAR_ARCHIVE, false);
		r_width = Cvar::Get<int>("r_width", "Resolution: width", Cvar::CVAR_ARCHIVE, 1024);
		r_height = Cvar::Get<int>("r_height", "Resolution: height", Cvar::CVAR_ARCHIVE, 768);
		r_hardware = Cvar::Get<bool>("r_hardware", "Dictates whether hardware acceleration is used", Cvar::CVAR_ARCHIVE, true);
		r_windowtitle = Cvar::Get<char*>("r_windowtitle", "Window title", Cvar::CVAR_ARCHIVE | Cvar::CVAR_ROM, "Rapture");
		r_gamma = Cvar::Get<float>("r_gamma", "Gamma", Cvar::CVAR_ARCHIVE, 1.2f);
	}

	void Initialize() {
		InitCvars();

		if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK) < 0) {
			printf("could not init SDL (error code: %s)\n", SDL_GetError());
			return;
		}

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

		renderer = SDL_CreateRenderer(window, -1, r_hardware->Bool() ?
										SDL_RENDERER_ACCELERATED : SDL_RENDERER_SOFTWARE);
		if(renderer == NULL) {
			SDL_Quit();
			return;
		}

		if(r_gamma->Value() != 1.0f) {
			unsigned short ramp[256];
			SDL_CalculateGammaRamp(r_gamma->Value(), ramp);
			SDL_SetWindowGammaRamp(window, ramp, ramp, ramp);
		}
	}

	void Restart() {
	}

	void Exit() {
	}

	void BlankFrame() {
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
		SDL_RenderClear(renderer);
	}

	void Display() {
		SDL_RenderPresent(renderer);
	}

	void* TexFromSurface(void* surface) {
		return (void*)SDL_CreateTextureFromSurface(renderer, (SDL_Surface*)surface);
	}

	void GetWindowSize(int* w, int* h) {
		SDL_GetWindowSize(window, w, h);
	}
};