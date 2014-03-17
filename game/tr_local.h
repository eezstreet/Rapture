#pragma once
#include "sys_local.h"
#include "tr_shared.h"
#include <SDL.h>
#include <SDL_image.h>
#include <gl/GL.h>
#include <gl/GLU.h>

extern Cvar* r_fullscreen;
extern Cvar* r_width;
extern Cvar* r_height;
extern Cvar* r_windowtitle;
extern Cvar* r_gamma;

void CreateConsole();
void DestroyConsole();
void ShowConsole();
void HideConsole();

// MaterialHandler.cpp
// Anything that exists in the world uses a Material.
// Images and the like do not use Materials since they do not exist in the world.
class Material {
private:
	bool bLoadedResources; // Have we loaded resources? (images, etc)
public:
	Material();

	friend class MaterialHandler;
};

class MaterialHandler {
private:
	unordered_map<string, Material*> materials;
	void LoadMaterial(const char* materialFile);
public:
	Material* GetMaterial(const char* material);
	~MaterialHandler();
	MaterialHandler();
};

extern MaterialHandler* mats;