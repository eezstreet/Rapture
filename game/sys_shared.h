#pragma once
#ifdef _WIN32
#include <Windows.h>
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
	void (*printf)(const char* fmt, ...);

	void* (*OpenFile)(const char* filename, const char* mode);
	void (*CloseFile)(void* filehandle);
	int (*ListFilesInDir)(const char* dir, vector<string>& in, const char* extension);
	string (*ReadPlaintext)(void* filehandle, size_t numChars);
	size_t (*GetFileSize)(void* filehandle);

	
	void* (*RegisterImage)(const char* filename);
	void (*DrawImage)(void* image, float xPct, float yPct, float wPct, float hPct);
	void (*DrawImageAspectCorrection)(void* image, float xPct, float yPct, float wPct, float hPct);
	void (*DrawImageNoScaling)(void* image, float xPct, float yPct);
	void (*DrawImageClipped)(void* image, float sxPct, float syPct, float swPct, float shPct,
		float ixPct, float iyPct, float iwPct, float ihPct);
	void (*DrawImageAbs)(void* image, int x, int y, int w, int h);
	void (*DrawImageAbsAspectCorrection)(void* image, int x, int y, int w, int h);
	void (*DrawImageAbsNoScaling)(void* image, int x, int y);
	void (*DrawImageAbsClipped)(void* image, int sx, int sy, int sw, int sh, int ix, int iy, int iw, int ih);
};

struct gameExports_s {
	void (*init)();
	void (*shutdown)();
	void (*runactiveframe)();
};

// sys_main.cpp
void R_Printf(const char *fmt, ...);