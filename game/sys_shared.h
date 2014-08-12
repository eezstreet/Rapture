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
#include <dirent.h>
#include <functional>
#include <time.h>
#include <assert.h>
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
class Image;
class Font;
class Menu;
class Material;
class Cvar;
class AnimationManager;

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

extern "C" {
	struct gameImports_s {
		// Logging
		void(*printf)(int iPriority, const char* fmt, ...);
		void(*error)(const char* fmt, ...);

		// Time
		int(*GetTicks)();

		// File I/O
		File* (*OpenFile)(const char* filename, const char* mode);
		size_t (*WriteFile)(File* filehandle, const char* text);
		void(*CloseFile)(File* filehandle);
		char** (*ListFilesInDir)(const char* dir, const char* extension, int* iNumFiles);
		void(*FreeFileList)(char** ptFileList, int iNumFiles);
		size_t(*ReadPlaintext)(File* filehandle, size_t numChars, char* chars);
		size_t(*ReadBinary)(File* filehandle, unsigned char* bytes, size_t numBytes, const bool bDontResetCursor);
		size_t(*GetFileSize)(File* filehandle);

		// Images
		Image* (*RegisterImage)(const char* filename);
		void(*DrawImage)(Image* image, float xPct, float yPct, float wPct, float hPct);
		void(*DrawImageAbs)(Image* image, int x, int y, int w, int h);
		void(*DrawImageAspectCorrection)(Image* image, float xPct, float yPct, float wPct, float hPct);
		void(*DrawImageClipped)(Image* image, float sxPct, float syPct, float swPct, float shPct,
								float ixPct, float iyPct, float iwPct, float ihPct);

		// Font/text
		Font* (*RegisterFont)(const char* sFontFile, int iPointSize);
		void(*RenderTextSolid)(Font* font, const char* text, int r, int g, int b);
		void(*RenderTextShaded)(Font* font, const char* text, int br, int bg, int bb, int fr, int fg, int fb);
		void(*RenderTextBlended)(Font* font, const char* text, int r, int g, int b);

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

		// Materials
		void(*InitMaterials)();
		void(*ShutdownMaterials)();
		Material* (*RegisterMaterial)(const char* name);
		void(*RenderMaterial)(Material* ptMaterial, int x, int y);
		void(*RenderMaterialTrans)(Material* ptMaterial, int x, int y);

		// Animation
		AnimationManager* (*GetAnimation)(const char* sUUID, const char* sMaterial);
		void(*AnimateMaterial)(AnimationManager* ptAnims, Material* ptMaterial, int x, int y, bool bTransparency);
		bool(*AnimationFinished)(AnimationManager* ptAnims);
		void(*SetAnimSequence)(AnimationManager* ptAnims, const char* sSequence);
		const char* (*GetAnimSequence)(AnimationManager* ptAnims);

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