#include "tr_local.h"
#include "../json/cJSON.h"

MaterialHandler* mats = NULL;

void MaterialHandler::LoadMaterial(const char* matfile) {
	File* f = File::Open(matfile, "r");
	if(!f) {
		return;
	}
	string contents = f->ReadPlaintext();
	char error[1024];
	cJSON* json = cJSON_ParsePooled(contents.c_str(), error, sizeof(error));
	if(!json) {
		R_Printf("ERROR loading material %s: %s\n", matfile, error);
		return;
	}

	char name[64];
	cJSON* child = cJSON_GetObjectItem(json, "name");
	if(!child) {
		R_Printf("WARNING: Material without name (%s)\n", matfile);
		strcpy(name, matfile);
	}
	else {
		strcpy(name, cJSON_ToString(child));
	}

	// TODO: stage parsing
	child = cJSON_GetObjectItem(json, "diffuseMap");
	if(!child) {
		R_Printf("WARNING: %s doesn't have a diffuse map!\n", name);
		return;
	}

	// TODO: map fields to Material class
	Material* mat = Zone::New<Material>(Zone::TAG_MATERIALS);
	mat->bLoadedResources = false;
	materials.insert(make_pair(name, mat));
}

MaterialHandler::MaterialHandler() {
	vector<string> matFiles;
	int numFiles = FS::EXPORT_ListFilesInDir("materials/", matFiles, ".json");
	for(auto it = matFiles.begin(); it != matFiles.end(); ++it) {
		LoadMaterial(it->c_str());
	}
	if(numFiles == 0) {
		R_Printf("WARNING: no materials loaded\n");
		return;
	}
}

MaterialHandler::~MaterialHandler() {
	for(auto it = materials.begin(); it != materials.end(); ++it) {
		Zone::FastFree(it->second, "materials");
	}
}