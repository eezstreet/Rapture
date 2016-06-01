#pragma once
#include "sys_local.h"
#include "tr_shared.h"
#include <SDL.h>
#include <SDL_image.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include <SDL_opengl.h>

extern Cvar* vid_renderer;
extern Cvar* vid_width;
extern Cvar* vid_height;
extern Cvar* vid_windowtitle;
extern Cvar* vid_fullscreen;


class Renderer {
private:
	char szRenderName[64];
	ptModule renderModule;
	bool bExportsValid;

public:
	renderExports_s* pExports;
	Renderer(const char* name);

	// Basic information
	bool AreExportsValid() { return bExportsValid; }

	// Extension of the exports
	void Initialize() { pExports->Initialize(); }
	void Shutdown() { pExports->Shutdown(); }
	void Restart() { pExports->Restart(); }

	void ClearFrame() { pExports->ClearFrame(); }
	void RenderFrame() { pExports->DrawActiveFrame(); }

	Texture* RegisterTexture(const char* szTextureName) { return pExports->RegisterTexture(szTextureName); }
	Texture* RegisterBlankTexture(const unsigned int nW, const unsigned int nH) { return pExports->RegisterBlankTexture(nW, nH); }
};

void CreateConsole();
void DestroyConsole();
void ShowConsole();
void HideConsole();