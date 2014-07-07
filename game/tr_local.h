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
	char startingSequence[64];
	int rowheight;
	int framesize;
	int longestSequence;
};

class AnimationManager {
private:
	static unordered_map<string, AnimationManager*> mAnimInstances;
	unordered_map<string, Sequence>* ptSequences;
	int lastFrameTime;
	SequenceData* ptSeqData;
public:
	static AnimationManager* GetAnimInstance(const char* sRef, const char* sMaterial);
	static void KillAnimInstance(const char* sRef);
	static void ShutdownAnims();
	static void Animate();

	string sCurrentSequence;
	int iCurrentFrame;
	AnimationManager(unordered_map<string, Sequence>* ptSequenceSet, SequenceData* ptSeqData);
	void PushFrame();
	void DrawActiveFrame(SDL_Texture* in, SDL_Rect* pos);
	void DrawAnimated(Material* ptMaterial, int x, int y, bool bTransparentMap);
	bool Finished();
	void SetSequence(const char* sSequence);
	const char* GetCurrentSequence();
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
	SequenceData sd;
public:
	Material();
	~Material();
	void SendToRenderer(int x, int y);
	void SendToRendererTransparency(int x, int y);
friend class AnimationManager;
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