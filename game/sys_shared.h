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
#include <dirent.h>
#include <functional>
using namespace std;

vector<string>& split(const string& str, const char delim);
vector<wstring>& split(const wstring& str, const wchar_t delim);
bool atob(const string& str);
bool atob(const char* str);
string trim(const string& str, const string& trim = " \t");
string hexstring(const int address);
typedef void (__cdecl *conCmd_t)(vector<string>& args);
void tostring(const wstring& in, string& out);
void towstring(const string& in, wstring& out);
bool checkExtension (string const &fullString, string const &ending);
void stringreplace(string& fullString, const string& sequence, const string& replace);
const char* btoa(bool b);
string stripextension(const string& str);

struct gameImports_s {
	// Logging
	void (*printf)(const char* fmt, ...);
	void (*error)(const char* fmt, ...);

	// Time
	int (*GetTicks)();

	// File I/O
	void* (*OpenFile)(const char* filename, const char* mode);
	void (*CloseFile)(void* filehandle);
	int (*ListFilesInDir)(const char* dir, vector<string>& in, const char* extension);
	string (*ReadPlaintext)(void* filehandle, size_t numChars);
	size_t (*ReadBinary)(void* filehandle, unsigned char* bytes, size_t numBytes, const bool bDontResetCursor);
	size_t (*GetFileSize)(void* filehandle);
	
	// Images
	void* (*RegisterImage)(const char* filename);
	void (*DrawImage)(void* image, float xPct, float yPct, float wPct, float hPct);
	void (*DrawImageAbs)(void* image, int x, int y, int w, int h);
	void (*DrawImageAspectCorrection)(void* image, float xPct, float yPct, float wPct, float hPct);
	void (*DrawImageClipped)(void* image, float sxPct, float syPct, float swPct, float shPct,
		float ixPct, float iyPct, float iwPct, float ihPct);

	// Font/text
	void* (*RegisterFont)(const char* sFontFile, int iPointSize);
	void (*RenderTextSolid)(void* font, const char* text, int r, int g, int b);
	void (*RenderTextShaded)(void* font, const char* text, int br, int bg, int bb, int fr, int fg, int fb);
	void (*RenderTextBlended)(void* font, const char* text, int r, int g, int b);

	// UI
	void* (*RegisterStaticMenu)(const char* sMenuFile);
	void (*KillStaticMenu)(void* menu);

	// Materials
	void (*InitMaterials)();
	void (*ShutdownMaterials)();
	void* (*RegisterMaterial)(const char* name);
	void (*RenderMaterial)(void* ptMaterial, float x, float y);

	// Cvars
	void (*CvarIntVal)(const char* cvarName, int* value);
	void (*CvarStrVal)(const char* cvarName, char* value);
	void (*CvarBoolVal)(const char* cvarName, bool* value);
	void (*CvarValue)(const char* cvarName, float* value);
	void* (*RegisterCvarInt)(const string& cvarName, const string& description, int flags, int startingValue);
	void* (*RegisterCvarFloat)(const string& cvarName, const string& description, int flags, float startingValue);
	void* (*RegisterCvarBool)(const string& cvarName, const string& description, int flags, bool bStartingValue);
	void* (*RegisterCvarStr)(const string& cvarName, const string& description, int flags, char* sStartingValue);
};

struct gameExports_s {
	void (*init)();
	void (*shutdown)();
	void (*runactiveframe)();

	void (*passmouseup)(int x, int y);
	void (*passmousedown)(int x, int y);
	void (*passmousemove)(int x, int y);

};

// sys_main.cpp
void R_Printf(const char *fmt, ...);
void R_Error(const char *fmt, ...);