#pragma once
#ifdef _WIN32
#include <Windows.h>
#include <Rpc.h>
#endif
#include <stdlib.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <list>
#include <queue>
#include <sstream>
#include <algorithm>
#include <map>
#include <regex>
#include <mutex>
#include <chrono>
#include <thread>
#include <dirent.h>
#include <functional>
#include <iomanip>
#include <time.h>
#include <assert.h>
#include <RaptureAsset.h>
using namespace std;

#ifdef _WIN32
#pragma warning(disable: 4996) // X is possibly unsafe
#pragma warning(error: 4190) // 'X' has C-linkage specified but returns UDT Y
#endif

#define MAX_HANDLE_STRING	64

extern const string keycodeNames[];
vector<string>& split(const string& str, const char delim, vector<string>& elems);
vector<wstring>& split(const wstring& str, const wchar_t delim);
bool atob(const string& str);
bool atob(const char* str);
string trim(const string& str, const string& trim = " \t");
string hexstring(const int address);
typedef void (__cdecl *conCmd_t)(vector<string>& args);
void tostring(const wstring& in, string& out);
void towstring(const string& in, wstring& out);
bool checkExtension (const string &fullString, const string &ending);
void stringreplace(string& fullString, const string& sequence, const string& replace);
const char* btoa(bool b);
string stripextension(const string& str);

// Export types
class File;
class Resource;
class Image;
class Font;
class Menu;
class Material;
class Cvar;
class AnimationManager;
class Texture;

enum {
	PRIORITY_NONE,
	PRIORITY_NOTE,
	PRIORITY_DEBUG,
	PRIORITY_MESSAGE,
	PRIORITY_WARNING,
	PRIORITY_ERROR,
	PRIORITY_ERRFATAL,
	PRIORITY_MAX
};

enum cvarFlags_e {
	CVAR_ARCHIVE,
	CVAR_ROM,
	CVAR_ANNOUNCE,
};

// Callbacks
typedef void(*fileOpenedCallback)(File* pFile);
typedef void(*fileReadCallback)(File* pFile, void* buffer, size_t bufferSize);
typedef fileReadCallback fileWrittenCallback;
typedef fileOpenedCallback fileClosedCallback;
typedef void(*assetRequestCallback)(AssetComponent* component);

extern "C" {
	struct gameImports_s {
		// Logging
		void(*printf)(int iPriority, const char* fmt, ...);
		void(*error)(const char* fmt, ...);

		// Time
		int(*GetTicks)();

		// Files
		File* (*OpenFileSync)(const char* filename, const char* mode);
		bool(*ReadFileSync)(File* pFile, void* data, size_t dataSize);
		bool(*WriteFileSync)(File* pFile, void* data, size_t dataSize);
		bool(*CloseFileSync)(File* pFile);

		File*(*OpenFileAsync)(const char* fileName, const char* mode, fileOpenedCallback callback);
		void(*ReadFileAsync)(File* pFile, void* data, size_t dataSize, fileReadCallback callback);
		void(*WriteFileAsync)(File* pFile, void* data, size_t dataSize, fileWrittenCallback callback);
		void(*CloseFileAsync)(File* pFile, fileClosedCallback callback);
		bool(*FileOpened)(File* pFile);
		bool(*FileRead)(File* pFile);
		bool(*FileWritten)(File* pFile);
		bool(*FileClosed)(File* pFile);
		bool(*FileBad)(File* pFile);

		// Resources
		Resource* (*ResourceAsync)(const char* asset, const char* component, assetRequestCallback callback);
		Resource* (*ResourceAsyncURI)(const char* uri, assetRequestCallback callback);
		Resource* (*ResourceSync)(const char* asset, const char* component);
		Resource* (*ResourceSyncURI)(const char* uri);
		void	  (*FreeResource)(Resource* pResource);
		AssetComponent* (*GetAssetComponent)(Resource* pResource);
		bool(*ResourceRetrieved)(Resource* pResource);
		bool(*ResourceBad)(Resource* pResource);

		// Materials
		Material*	(*RegisterMaterial)(const char* szMaterial);
		void		(*DrawMaterial)(Material* ptMaterial, float xPct, float yPct, float wPct, float hPct);
		void		(*DrawMaterialAspectCorrection)(Material* ptMaterial, float xPct, float yPct, float wPct, float hPct);
		void		(*DrawMaterialClipped)(Material* ptMaterial, float sxPct, float syPct, float swPct, float shPct, float ixPct, float iyPct, float iwPct, float ihPct);
		void		(*DrawMaterialAbs)(Material* ptMaterial, int nX, int nY, int nW, int nH);
		void		(*DrawMaterialAbsClipped)(Material* ptMaterial, int sX, int sY, int sW, int sH, int iX, int iY, int iW, int iH);

		// Textures
		Texture*	(*RegisterStreamingTexture)(const unsigned int nW, const unsigned int nH);
		int			(*LockStreamingTexture)(Texture* ptTexture, unsigned int nX, unsigned int nY, unsigned int nW, unsigned int nH, void** pixels, int* pitch);
		void		(*UnlockStreamingTexture)(Texture* ptTexture);
		void		(*DeleteStreamingTexture)(Texture* ptTexture);
		void		(*BlendTexture)(Texture* ptTexture);

		// UI
		Menu* (*RegisterStaticMenu)(const char* sMenuFile);
		void  (*KillStaticMenu)(Menu* menu);
		void  (*RunJavaScript)(Menu* menu, const char* sJS);
		bool  (*IsConsoleOpen)();
		void  (*AddJSCallback)(Menu* menu, const char* sCallbackName, void(*ptCallback)());
		unsigned int   (*GetJSNumArgs)(Menu* ptMenu);
		void (*GetJSStringArg)(Menu* ptMenu, unsigned int argNum, char* sBuffer, size_t numChars);
		int   (*GetJSIntArg)(Menu* ptMenu, unsigned int argNum);
		double(*GetJSDoubleArg)(Menu* ptMenu, unsigned int argNum);
		bool  (*GetJSBoolArg)(Menu* ptMenu, unsigned int argNum);

		// Cvars
		void(*CvarIntVal)(const char* cvarName, int* value);
		void(*CvarStrVal)(const char* cvarName, char* value);
		void(*CvarBoolVal)(const char* cvarName, bool* value);
		void(*CvarValue)(const char* cvarName, float* value);
		Cvar* (*RegisterCvarInt)(const char* cvarName, const char* description, int flags, int startingValue);
		Cvar* (*RegisterCvarFloat)(const char* cvarName, const char* description, int flags, float startingValue);
		Cvar* (*RegisterCvarBool)(const char* cvarName, const char* description, int flags, bool bStartingValue);
		Cvar* (*RegisterCvarStr)(const char* cvarName, const char* description, int flags, char* sStartingValue);

		// Zone memory
		void* (*Zone_Alloc)(int iSize, const char* tag);
		void(*Zone_NewTag)(const char* tag);
		void(*Zone_Free)(void *memory);
		void(*Zone_FastFree)(void* memory, const char* tag);
		void(*Zone_FreeAll)(const char* tag);
		void* (*Zone_Realloc)(void* memory, size_t iNewSize);

		// Global effects
		void(*FadeFromBlack)(int time);
	};

	struct gameExports_s {
		void(*init)();
		void(*shutdown)();
		void(*runactiveframe)();

		void(*passmouseup)(int x, int y);
		void(*passmousedown)(int x, int y);
		void(*passmousemove)(int x, int y);

		void(*passkeypress)(int x);
	};
}

// sys_main.cpp
void R_Message(int iPriority, const char *fmt, ...);

// Erases an item from a vector provided that the vector contains all unique elements.
template <typename T>
void VectorErase(vector<T>& rtVector, T uniqueMember) {
	for(auto it = rtVector.begin(); it != rtVector.end(); ++it) {
		if(*it == uniqueMember) {
			rtVector.erase(it);
			break;
		}
	}
}