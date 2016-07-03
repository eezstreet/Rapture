#pragma once
#include "sys_shared.h"

struct SDL_Window;
typedef struct _TTF_Font TTF_Font;

class Texture;
class Material;

// Stuff that gets exported FROM the renderer
struct renderExports_s {
	// Basic logic
	void		(*Initialize)();
	void		(*Shutdown)();
	void		(*Restart)();

	// These get run every frame
	void		(*ClearFrame)();
	void		(*DrawActiveFrame)();

	// Material manipulation
	Material*	(*RegisterMaterial)(const char* URI);
	void		(*DrawMaterial)(Material* ptMaterial, float xPct, float yPct, float wPct, float hPct);
	void		(*DrawMaterialAspectCorrection)(Material* ptMaterial, float xPct, float yPct, float wPct, float hPct);
	void		(*DrawMaterialClipped)(Material* ptMaterial, float sxPct, float syPct, float swPct, float shPct, float ixPct, float iyPct, float iwPct, float ihPct);
	void		(*DrawMaterialAbs)(Material* ptMaterial, int nX, int nY, int nW, int nH);
	void		(*DrawMaterialAbsClipped)(Material* ptMaterial, int sX, int sY, int sW, int sH, int iX, int iY, int iW, int iH);

	// Streaming textures
	Texture*	(*RegisterStreamingTexture)(unsigned int nWidth, unsigned int nHeight);
	int			(*LockStreamingTexture)(Texture* ptTexture, unsigned int nX, unsigned int nY, unsigned int nW, unsigned int nH, void** pixels, int* pitch);
	void		(*UnlockStreamingTexture)(Texture* ptTexture);
	void		(*DeleteStreamingTexture)(Texture* ptTexture);

	// Screenshots
	void		(*QueueScreenshot)(const char* szFileName, const char* szExtension);
	
	// Special effects
	void		(*FadeFromBlack)(int ms);

	// Text
	void		(*RenderTextSolid)(Font* font, const char* text, int r, int g, int b);
	void		(*RenderTextShaded)(Font* font, const char* text, int br, int bg, int bb, int fr, int fg, int fb);
	void		(*RenderTextBlended)(Font* font, const char* text, int r, int g, int b);

	// Callbacks
	void		(*WindowWidthChanged)(int newWidth);
	void		(*WindowHeightChanged)(int newHeight);
};

// Stuff that gets imported TO the renderer
struct renderImports_s {
	// Dispatch
	void			(*Print)(int priority, const char* fmt, ...);

	// Asynchronous File Operations
	File*			(*OpenFileAsync)(const char* file, const char* mode, fileOpenedCallback callback);
	void			(*ReadFileAsync)(File* pFile, void* data, size_t dataSize, fileReadCallback callback);
	void			(*WriteFileAsync)(File* pFile, void* data, size_t dataSize, fileWrittenCallback callback);
	void			(*CloseFileAsync)(File* pFile, fileClosedCallback callback);
	bool			(*AsyncFileOpened)(File* pFile);
	bool			(*AsyncFileRead)(File* pFile);
	bool			(*AsyncFileWritten)(File* pFile);
	bool			(*AsyncFileClosed)(File* pFile);
	bool			(*AsyncFileBad)(File* pFile);

	// Synchronous File Operations
	File*			(*OpenFileSync)(const char* file, const char* mode);
	bool			(*ReadFileSync)(File* pFile, void* data, size_t dataSize);
	bool			(*WriteFileSync)(File* pFile, void* data, size_t dataSize);
	bool			(*CloseFileSync)(File* pFile);

	// Resources
	Resource*		(*ResourceAsync)(const char* asset, const char* component, assetRequestCallback callback);
	Resource*		(*ResourceAsyncURI)(const char* uri, assetRequestCallback callback);
	Resource*		(*ResourceSync)(const char* asset, const char* component);
	Resource*		(*ResourceSyncURI)(const char* uri);
	void			(*FreeResource)(Resource* pResource);
	AssetComponent*	(*ResourceComponent)(Resource* pResource);

	// Other file operations
	char*			(*ResolveFilePath)(char* buffer, size_t bufferSize, const char* file, const char* mode);
	const char*		(*ResolveAssetPath)(const char* assetName);
	
	// Video
	void			(*GetRenderProperties)(int* renderWidth, int* renderHeight, bool* fullscreen);
	SDL_Window*		(*GetWindow)();

	// Cvar
	Cvar*			(*RegisterStringCvar)(const char* name, const char* description, char* defaultValue, int flags);
	Cvar*			(*RegisterIntCvar)(const char* name, const char* description, int defaultValue, int flags);
	Cvar*			(*RegisterFloatCvar)(const char* name, const char* description, float defaultValue, int flags);
	Cvar*			(*RegisterBoolCvar)(const char* name, const char* description, bool defaultValue, int flags);

	char*			(*CvarStringValue)(Cvar* cvar);
	int				(*CvarIntegerValue)(Cvar* cvar);
	float			(*CvarFloatValue)(Cvar* cvar);
	bool			(*CvarBooleanValue)(Cvar* cvar);
};

//
// Video.cpp
//
namespace Video {
	// Basic
	bool Init();
	void Restart();
	void Shutdown();

	// Run every frame
	void ClearFrame();
	void RenderFrame();

	// Materials
	Material* RegisterMaterial(const char* szMaterial);
	void DrawMaterial(Material* ptMaterial, float xPct, float yPct, float wPct, float hPct);
	void DrawMaterialAspectCorrection(Material* ptMaterial, float xPct, float yPct, float wPct, float hPct);
	void DrawMaterialClipped(Material* ptMaterial, float sxPct, float syPct, float swPct, float shPct, float ixPct, float iyPct, float iwPct, float ihPct);
	void DrawMaterialAbs(Material* ptMaterial, int nX, int nY, int nW, int nH);
	void DrawMaterialAbsClipped(Material* ptMaterial, int sX, int sY, int sW, int sH, int iX, int iY, int iW, int iH);

	// Textures
	Texture* RegisterStreamingTexture(const unsigned int nW, const unsigned int nH);
	int LockStreamingTexture(Texture* ptTexture, unsigned int nX, unsigned int nY, unsigned int nW, unsigned int nH, void** pixels, int* pitch);
	void UnlockStreamingTexture(Texture* ptTexture);
	void DeleteStreamingTexture(Texture* ptTexture);

	// Screenshots
	void QueueScreenshot(const char* szFileName, const char* szExtension);

	// Special Effects
	void FadeFromBlack(int ms);

	// Text
	void RenderTextSolid(Font* font, const char* text, int r, int g, int b);
	void RenderTextShaded(Font* font, const char* text, int br, int bg, int bb, int fr, int fg, int fb);
	void RenderTextBlended(Font* font, const char* text, int r, int g, int b);

	// Imported from the engine
	SDL_Window* GetRaptureWindow();
	void GetWindowInfo(int* renderWidth, int* renderHeight, bool* fullscreen);
	int GetWidth();
	int GetHeight();
};