#pragma once
#include "sys_local.h"
#include "tr_shared.h"
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_opengl.h>
#include <gl/GL.h>
#include <gl/GLU.h>

extern Cvar* r_fullscreen;
extern Cvar* r_width;
extern Cvar* r_height;
extern Cvar* r_windowtitle;
extern Cvar* r_gamma;
extern Cvar* r_filter;
#ifdef _DEBUG
extern Cvar* r_imgdebug;
#endif
extern Cvar* gl_mode;
extern Cvar* gl_major;
extern Cvar* gl_minor;

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
	void DrawActiveFrame(RenderTexture* in, SDL_Rect* pos);
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
	RenderTexture *ptResource;
	RenderTexture *ptTransResource;

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


// Internal texture resource
struct RenderTexture {
	union {
		SDL_Texture* ptTexture;
		GLuint glTexture;
	} data;
	Uint32 width;
	Uint32 height;
	
	// --- only in GL ---
	GLuint* pixels;
	// --- only in GL ---
};

// Specific renderer types
class Renderer {
public:
	virtual void Restart() = 0;
	virtual void StartFrame() = 0;
	virtual void EndFrame() = 0;
	virtual bool Start(SDL_Window* ptWindow) = 0;

	// Texture manipulation
	virtual RenderTexture* CreateTexture(Uint32 dwWidth, Uint32 dwHeight) = 0;
	virtual RenderTexture* TextureFromPixels(void* pixels, int width, int height, uint32_t extra) = 0;
	virtual void LockTexture(RenderTexture* ptvTexture, void** pixels, int* span, bool bWriteOnly) = 0;
	virtual void UnlockTexture(RenderTexture* ptvTexture) = 0;
	virtual void DeleteTexture(RenderTexture* ptvTexture) = 0;

	// Image manipulation
	virtual void BlendTexture(RenderTexture* ptvTexture) = 0;

	// Screenshot
	virtual void CreateScreenshot(const string& sFileName) = 0;

	static Renderer* GetCurrent();
};

extern Renderer* ptRenderer;

class RendSDL : public Renderer {
private:
	SDL_Renderer* ptRenderer;

	SDL_Surface* renderSurf;
	SDL_Texture* renderTex;
	unordered_map<string, SDL_Texture*> images;
	int textFieldCount = 0;
#define MAX_TEXTRENDER 8
	SDL_Texture* textFields[MAX_TEXTRENDER];

	SDL_Window* m_ptWindow;
public:
	virtual void Restart();
	virtual void StartFrame();
	virtual void EndFrame();
	virtual bool Start(SDL_Window* ptWindow);

	// Texture manipulation
	virtual RenderTexture* CreateTexture(Uint32 dwWidth, Uint32 dwHeight);
	virtual RenderTexture* TextureFromPixels(void* pixels, int width, int height, uint32_t extra);
	virtual void LockTexture(RenderTexture* ptvTexture, void** pixels, int* span, bool bWriteOnly);
	virtual void UnlockTexture(RenderTexture* ptvTexture);
	virtual void DeleteTexture(RenderTexture* ptvTexture);

	virtual void BlendTexture(RenderTexture* ptvTexture);

	// Screenshot
	virtual void CreateScreenshot(const string& sFileName);

	RendSDL();
	~RendSDL();
};

class RendGL : public Renderer {
private:
	SDL_GLContext gl;
	GLuint screenVBO;
	GLuint screenVAO;

	unordered_map<string, GLuint> mShaderPrograms;

	void(APIENTRY *glGenVertexArrays)(GLsizei n, GLuint* arrays);
	void(APIENTRY *glBindVertexArray)(GLuint array);
	void(APIENTRY *glGenBuffers)(GLsizei n, GLuint* buffers);
	void(APIENTRY *glBindBuffer)(GLenum target, GLuint buffer);
	void(APIENTRY *glBufferData)(GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage);
	void(APIENTRY *glActiveTexture)(GLenum texture);

	GLuint(APIENTRY *glCreateShader)(GLenum shaderType);
	GLuint(APIENTRY *glCreateProgram)(void);
	void(APIENTRY *glShaderSource)(GLuint shader, GLsizei count, const GLchar** string, const GLint* length);
	void(APIENTRY *glCompileShader)(GLuint shader);
	void(APIENTRY *glAttachShader)(GLuint program, GLuint shader);
	void(APIENTRY *glBindFragDataLocation)(GLuint program, GLuint colorNumber, const char* name);
	void(APIENTRY *glLinkProgram)(GLuint program);
	void(APIENTRY *glUseProgram)(GLuint program);
	GLint(APIENTRY *glGetAttribLocation)(GLuint program, const GLchar* name);
	void(APIENTRY *glEnableVertexAttribArray)(GLuint index);
	void(APIENTRY *glVertexAttribPointer)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* pointer);

	void LoadShaderText(const string& fileName, string& text);
public:
	virtual void Restart();
	virtual void StartFrame();
	virtual void EndFrame();
	virtual bool Start(SDL_Window* ptWindow);

	// Texture manipulation
	virtual RenderTexture* CreateTexture(Uint32 dwWidth, Uint32 dwHeight);
	virtual RenderTexture* TextureFromPixels(void* pixels, int width, int height, uint32_t extra);
	virtual void LockTexture(RenderTexture* ptvTexture, void** pixels, int* span, bool bWriteOnly);
	virtual void UnlockTexture(RenderTexture* ptvTexture);
	virtual void DeleteTexture(RenderTexture* ptvTexture);

	virtual void BlendTexture(RenderTexture* ptvTexture);

	virtual void CreateScreenshot(const string& sFileName);

	RendGL();
	~RendGL();
};