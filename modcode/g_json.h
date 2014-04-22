#include "g_local.h"

template <class T>
bool JSON_ParseMultifile(const char* filename, const unordered_map<const char*, jsonParseFunc>& parsers, vector<T>& out) {
	if(!filename || !filename[0]) {
		R_Printf("JSON_ParseMultifile: bad filename sent\n");
		return false;
	}
	void* file = trap->OpenFile(filename, "rb");
	if(!file) {
		R_Printf("JSON_ParseMultifile: could not open file %s\n", filename);
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
	int i = 0;
	for(auto it = cJSON_GetFirstItem(root); it; it = cJSON_GetNextItem(it), i++) {
		T x;
		JSON_ParseFieldSet(it, parsers, root, (void*)&x);
		out.push_back(x);
	}
	return true;
}