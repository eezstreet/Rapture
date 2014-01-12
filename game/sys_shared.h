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
using namespace std;

vector<string>& split(const string& str, const char delim);
vector<wstring>& split(const wstring& str, const wchar_t delim);
bool atob(const string& str);
bool atob(const char* str);

typedef void (__cdecl *conCmd_t)(vector<string>& args);