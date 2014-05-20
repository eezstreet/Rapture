#include "g_local.h"

unordered_map<string, PresetFileData> presets;

static void MP_LoadTilePreset(const string& path) {
	if(path.size() <= 0) {
		return;
	}
	void* file = trap->OpenFile(path.c_str(), "rb");
	if(!file) {
		R_Error("Directory failure in levels/preset: %s", path.c_str());
		return; // ? shouldn't happen
	}


}

void MP_LoadTilePresets() {
	vector<string> paths;
	int numFiles = trap->ListFilesInDir("levels/preset", paths, ".pdp");
	if(paths.size() <= 0) {
		R_Error("Could not load presets in levels/preset (none?)");
		return;
	}

	for(auto it = paths.begin(); it != paths.end(); ++it) {
		MP_LoadTilePreset(*it);
	}

	R_Printf("Loaded %i presets\n", paths.size());
}

PresetFileData* MP_GetPreset(const string& sName) {
	auto it = presets.find(sName);
	if(it == presets.end()) {
		return NULL;
	}
	return &it->second;
}