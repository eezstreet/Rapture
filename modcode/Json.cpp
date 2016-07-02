#include "Json.h"

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
	/*for(auto it = cJSON_GetFirstItem(root); it != nullptr; it = cJSON_GetNextItem(it)) {
		const char* name = cJSON_GetItemKey(it);
		auto obj = parsers.find(name);
		if(obj != parsers.end()) {
			obj->second(it, output);
		}
		else {
			R_Message("WARNING: unknown JSON field (%s) found\n", name);
		}
	}*/
	return true;
}

bool JSON_ParseFile(char *filename, const unordered_map<const char*, jsonParseFunc>& parsers, void* output) {
	if(!filename || !filename[0]) {
		R_Message(PRIORITY_WARNING, "JSON_ParseFile: bad filename sent\n");
		return false;
	}
	File* file = trap->OpenFileSync(filename, "rb");
	if(!file) {
		R_Message(PRIORITY_WARNING, "JSON_ParseFile: could not open file %s\n", filename);
		return false;
	}
	size_t numChars = trap->GetFileSize(file);
	char* s = (char*)trap->Zone_Alloc(numChars * sizeof(char), "files");
	trap->ReadPlaintext(file, numChars, s);
	trap->CloseFileAsync(file, nullptr);

	char error[1024] = {0};
	cJSON* root = cJSON_ParsePooled(s, error, sizeof(error));
	if(error[0] || !root) {
		R_Message(PRIORITY_ERROR, "ERROR: %s: %s\n", filename, error);
		trap->Zone_FastFree(s, "files");
		return false;
	}
	for(auto it = cJSON_GetFirstItem(root); it; it = cJSON_GetNextItem(it)) {
		JSON_ParseFieldSet(root, parsers, output);
	}
	trap->Zone_FastFree(s, "files");
	return true;
}