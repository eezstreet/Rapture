#include "tr_local.h"

Cvar* r_fullscreen = nullptr;
Cvar* r_width = nullptr;
Cvar* r_height = nullptr;
Cvar* r_windowtitle = nullptr;
Cvar* r_gamma = nullptr;
Cvar* r_filter = nullptr;

#ifdef _DEBUG
Cvar* r_imgdebug = nullptr;
#endif

Cvar* gl_mode = nullptr;
Cvar* gl_major = nullptr;
Cvar* gl_minor = nullptr;

static int fadeTime = 0;
static int initialFadeTime = 0;
static bool bShouldWeFade = false;
static Uint32 iCurrentTime = 0;

static Renderer* ptRenderer;

namespace RenderCode {
	static SDL_Window *window;

	static void GammaCallback(float newValue) {
		unsigned short ramp[256];
		SDL_CalculateGammaRamp(newValue, ramp);
		SDL_SetWindowGammaRamp(window, ramp, ramp, ramp);
	}

	static void WindowTitleCallback(char* newValue) {
		SDL_SetWindowTitle(window, newValue);
	}

	static void InitCvars() {
		r_fullscreen = Cvar::Get<bool>("r_fullscreen", "Dictates whether the application runs in fullscreen mode.", (1 << CVAR_ARCHIVE), false);
		r_width = Cvar::Get<int>("r_width", "Resolution: width", (1 << CVAR_ARCHIVE), 1024);
		r_height = Cvar::Get<int>("r_height", "Resolution: height", (1 << CVAR_ARCHIVE), 768);
		r_windowtitle = Cvar::Get<char*>("r_windowtitle", "Window title", (1 << CVAR_ARCHIVE) | (1 << CVAR_ROM), "Rapture");
		r_gamma = Cvar::Get<float>("r_gamma", "Gamma", (1 << CVAR_ARCHIVE), 1.0f);
		r_filter = Cvar::Get<int>("r_filter", "Filter quality. 0 = nearest pixel sampling, 1 = linear filtering, 2 = anisotropic filtering", (1 << CVAR_ARCHIVE), 1);

		// WARNING: this assumes that your video card supports OpenGL 3.2. Need to determine properly which version is supported
		gl_mode = Cvar::Get<bool>("gl_mode", "Enable OpenGL rendering?", (1 << CVAR_ARCHIVE), true);
		gl_major = Cvar::Get<int>("gl_major", "OpenGL major version", (1 << CVAR_ARCHIVE), 3);
		gl_minor = Cvar::Get<int>("gl_minor", "OpenGL minor version", (1 << CVAR_ARCHIVE), 2);

		// Debug cvars
#ifdef _DEBUG
		r_imgdebug = Cvar::Get<bool>("r_imgdebug", "Draw lines around image bounds", 0, false);
#endif

		r_gamma->AddCallback((void*)GammaCallback);
		r_windowtitle->AddCallback((void*)WindowTitleCallback);
	}

	void Initialize() {
		R_Message(PRIORITY_NOTE, "Initializing window\n");
		InitCvars();

		R_Message(PRIORITY_NOTE, "SDL_Init()\n");
		if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK) < 0) {
			printf("could not init SDL (error code: %s)\n", SDL_GetError());
			return;
		}

		R_Message(PRIORITY_NOTE, "SDL_CreateWindow()\n");
		if(gl_mode->Bool() == true) {
			ptRenderer = new RendGL();
		} else {
			ptRenderer = new RendSDL();
		}

		window = SDL_CreateWindow(r_windowtitle->String(),
								  SDL_WINDOWPOS_CENTERED,
								  SDL_WINDOWPOS_CENTERED,
								  r_width->Integer(),
								  r_height->Integer(),
								  SDL_WINDOW_OPENGL | (r_fullscreen->Bool() ? SDL_WINDOW_FULLSCREEN : SDL_WINDOW_SHOWN));
		if(window == nullptr) {
			delete ptRenderer;
			SDL_Quit();
			return;
		}

		if(ptRenderer->Start(window) == false) {
			delete ptRenderer;
			SDL_Quit();
			return;
		}
		r_gamma->RunCallback();
	}

	void Restart() {
		r_gamma->RunCallback();

		ptRenderer->Restart();

		UI::Restart();
	}

	void Exit(const bool bSilent) {
		AnimationManager::ShutdownAnims();
		delete ptRenderer;

		if(!bSilent) {
			R_Message(PRIORITY_NOTE, "gamma ramp down--\n");
		}
		unsigned short ramp[256];
		SDL_CalculateGammaRamp(1.0f, ramp);
		SDL_SetWindowGammaRamp(window, ramp, ramp, ramp);

		SDL_DestroyWindow(window);
	}

	void BeginFrame() {
		ptRenderer->StartFrame();

		iCurrentTime = SDL_GetTicks();
		if(fadeTime > iCurrentTime) {
			bShouldWeFade = true;
		} else if(bShouldWeFade) {
			bShouldWeFade = false;
		}
	}

	void EndFrame() {
		AnimationManager::Animate();
		// TODO: screenshots
		ptRenderer->EndFrame();

		// The swap stage has to be done separately in GL mode because it needs the window
		if(gl_mode->Bool() == true) {
			SDL_GL_SwapWindow(window);
		}
	}


	//
	// Texture Management
	//
	RenderTexture* CreateFullscreenTexture() {
		RenderTexture* ptRenderTexture = ptRenderer->CreateTexture(r_width->Integer(), r_height->Integer());
		return ptRenderTexture;
	}

	RenderTexture* TextureFromPixels(void* pixels, int width, int height, void* extra) {
		RenderTexture* ptRenderTexture = ptRenderer->TextureFromPixels(pixels, width, height, (uint32_t)extra);
		return ptRenderTexture;
	}

	void DestroyTexture(RenderTexture* ptTexture) {
		ptRenderer->DeleteTexture(ptTexture);
	}

	void LockTexture(RenderTexture* ptvTexture, void** pixels, int* span, bool bWriteOnly) {
		ptRenderer->LockTexture(ptvTexture, pixels, span, bWriteOnly);
	}

	void UnlockTexture(RenderTexture* ptvTexture) {
		ptRenderer->UnlockTexture(ptvTexture);
	}

	void BlendTexture(RenderTexture* ptvTexture) {
		ptRenderer->BlendTexture(ptvTexture);
	}

	//
	// Screenshot
	//

	void QueueScreenshot(const string& fileName, const string& extension) {
		ptRenderer->CreateScreenshot(fileName + extension);
	}

	// kill me
	void BlankFrame() {

	}

	//
	// Animation Management
	//
	void AnimateMaterial(AnimationManager* ptAnims, Material* ptMaterial, int x, int y, bool bTransparency) {
		if(!ptAnims || !ptMaterial) {
			return;
		}
		ptAnims->DrawAnimated(ptMaterial, x, y, bTransparency);
	}

	AnimationManager* GetAnimation(const char* sUUID, const char* sMaterial) {
		return AnimationManager::GetAnimInstance(sUUID, sMaterial);
	}

	bool AnimationFinished(AnimationManager* ptAnims) {
		return ptAnims->Finished();
	}

	void SetAnimSequence(AnimationManager* ptAnims, const char* sSequence) {
		ptAnims->SetSequence(sSequence);
	}

	const char* GetAnimSequence(AnimationManager* ptAnims) {
		return ptAnims->GetCurrentSequence();
	}


	//
	// Material System
	//
	void InitMaterials() {
		R_Message(PRIORITY_NOTE, "Initializing materials..\n");
		mats = new MaterialHandler();
	}

	void ShutdownMaterials() {
		R_Message(PRIORITY_NOTE, "Freeing materials..\n");
		delete mats;
	}

	Material* RegisterMaterial(const char* name) {
		return mats->GetMaterial(name);
	}

	void SendMaterialToRenderer(Material* ptMaterial, int x, int y) {
		if(!ptMaterial) {
			return;
		}
		ptMaterial->SendToRenderer(x, y);
	}

	void SendMaterialToRendererTrans(Material* ptMaterial, int x, int y) {
		if(!ptMaterial) {
			return;
		}
		ptMaterial->SendToRendererTransparency(x, y);
	}


	//
	// Special Effects
	//
	void FadeFromBlack(int ms) {
		initialFadeTime = SDL_GetTicks();
		fadeTime = initialFadeTime + ms;
		bShouldWeFade = true;
	}
}