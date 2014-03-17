#include "g_local.h"

bool JSON_ParseFieldSet(cJSON* root, const unordered_map<const char*, jsonParseFunc>& parsers, void* output) {
	if(!output) {
		return false;
	}
	for(auto it = cJSON_GetFirstItem(root); it != NULL; it = cJSON_GetNextItem(it)) {
		const char* name = cJSON_GetItemKey(it);
		auto obj = parsers.find(name);
		if(obj != parsers.end()) {
			obj->second(it, output);
		}
		else {
			R_Printf("WARNING: unknown JSON field (%s) found\n", name);
		}
	}
	return true;
}

bool JSON_ParseFile(char *filename, const unordered_map<const char*, jsonParseFunc>& parsers, void* output) {
	if(!filename || !filename[0]) {
		R_Printf("JSON_ParseFile: bad filename sent\n");
		return false;
	}
	void* file = trap->OpenFile(filename, "r");
	if(!file) {
		R_Printf("JSON_ParseFile: could not open file %s\n", filename);
		return false;
	}
	string s = trap->ReadPlaintext(file, 0);
	trap->CloseFile(file);

	char error[1024];
	cJSON* root = cJSON_ParsePooled(s.c_str(), error, sizeof(error));
	if(error[0] || !root) {
		R_Printf("ERROR: %s: %s\n", filename, error);
		return false;
	}

	return JSON_ParseFieldSet(root, parsers, output);
}