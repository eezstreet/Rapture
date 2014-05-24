#include "g_local.h"

bool JSON_ParseFieldSet(cJSON* root, const unordered_map<const char*, jsonParseFunc>& parsers, void* output) {
	cJSON* x = output;
	if(!output) {
		return false;
	}
	for(auto it = parsers.begin(); it != parsers.end(); ++it) {
		cJSON* obj = cJSON_GetObjectItem(root, it->first);
		if(!obj) {
			continue;
		}
		it->second(obj, output);
	}

	// FIXME: below method is faster and reports incorrect fields, but doesn't work for some reason...
	/*for(auto it = cJSON_GetFirstItem(root); it != NULL; it = cJSON_GetNextItem(it)) {
		const char* name = cJSON_GetItemKey(it);
		auto obj = parsers.find(name);
		if(obj != parsers.end()) {
			obj->second(it, output);
		}
		else {
			R_Printf("WARNING: unknown JSON field (%s) found\n", name);
		}
	}*/
	return true;
}

bool JSON_ParseFile(char *filename, const unordered_map<const char*, jsonParseFunc>& parsers, void* output) {
	if(!filename || !filename[0]) {
		R_Printf("JSON_ParseFile: bad filename sent\n");
		return false;
	}
	void* file = trap->OpenFile(filename, "rb");
	if(!file) {
		R_Printf("JSON_ParseFile: could not open file %s\n", filename);
		return false;
	}
	string s = trap->ReadPlaintext(file, 0);
	trap->CloseFile(file);

	char error[1024] = {0};
	cJSON* root = cJSON_ParsePooled(s.c_str(), error, sizeof(error));
	if(error[0] || !root) {
		R_Printf("ERROR: %s: %s\n", filename, error);
		return false;
	}
	for(auto it = cJSON_GetFirstItem(root); it; it = cJSON_GetNextItem(it)) {
		JSON_ParseFieldSet(root, parsers, output);
	}
	return true;
}