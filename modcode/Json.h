#pragma once
#include "Local.h"
#include "../json/cJSON.h"

// JSON parser structs
typedef function<void (cJSON*, void*)> jsonParseFunc;
bool JSON_ParseFieldSet(cJSON* root, const unordered_map<const char*, jsonParseFunc>& parsers, void* output);
bool JSON_ParseFile(char *filename, const unordered_map<const char*, jsonParseFunc>& parsers, void* output);

template <class T>
bool JSON_ParseMultifile(const char* filename, const unordered_map<const char*, jsonParseFunc>& parsers, vector<T>& out) {
	if(!filename || !filename[0]) {
		R_Message(PRIORITY_WARNING, "JSON_ParseMultifile: bad filename sent\n");
		return false;
	}
	File* file = trap->OpenFile(filename, "rb");
	if(!file) {
		R_Message(PRIORITY_WARNING, "JSON_ParseMultifile: could not open file %s\n", filename);
		return false;
	}
	size_t numChars = trap->GetFileSize(file);
	char* s = (char*)trap->Zone_Alloc(numChars * sizeof(char), "files");
	trap->ReadPlaintext(file, numChars, s);
	trap->CloseFile(file);

	char error[1024] = {0};
	cJSON* root = cJSON_ParsePooled(s, error, sizeof(error));
	if(error[0] || !root) {
		R_Message(PRIORITY_ERROR, "ERROR: %s: %s\n", filename, error);
		trap->Zone_FastFree(s, "files");
		return false;
	}
	int i = 0;
	for(auto it = cJSON_GetFirstItem(root); it; it = cJSON_GetNextItem(it), i++) {
		T x;
		JSON_ParseFieldSet(it, parsers, (void*)&x);
		out.push_back(x);
	}
	trap->Zone_FastFree(s, "files");
	return true;
}