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

// AnimationHandler.cpp
// Materials may or may not have an animation assigned to them.
struct Sequence {
	char name[64];
	bool bLoop;
	int rowNum;
	int frameCount;
	int fps;
	bool bInitial;

	void Parse(void* in);
};

struct SequenceData {
	int rowheight;
	int framesize;
};

class AnimationManager {
private:
	unordered_map<string, Sequence>* ptSequences;
	int lastFrameTime;
	SequenceData sdSeqData;
public:
	string sCurrentSequence;
	int iCurrentFrame;
	int iLongestSequence;
	AnimationManager(const string& sSequence, unordered_map<string, Sequence>* ptSequenceSet, int longestSequence, SequenceData sData);
	void PushFrame();
	void DrawActiveFrame(SDL_Texture* in, SDL_Rect* pos);
};

// MaterialHandler.cpp
// Anything that exists in the world uses a Material.
// Images and the like do not use Materials since they do not exist in the world.
class Material {
private:
	bool bLoadedResources; // Have we loaded resources? (images, etc)
	bool bLoadedIncorrectly; // If we loaded, did we load correctly?
	bool bHasTransparencyMap; // Does it have a transparency map?
	void LoadResources();
	void FreeResources();

	int xOffset, yOffset;

	char name[64];
	char resourceFile[64];
	char transResourceFile[64];
	SDL_Texture *ptResource;
	SDL_Texture *ptTransResource;

	int iNumSequences;
	unordered_map<string, Sequence> mSequences;
	AnimationManager* ptAnims;
public:
	Material();
	~Material();
	void SendToRenderer(int x, int y);
	void SendToRendererTransparency(int x, int y);

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