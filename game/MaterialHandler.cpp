#include "tr_local.h"
#include "../json/cJSON.h"

MaterialHandler* mats = NULL;

void MaterialHandler::LoadMaterial(const char* matfile) {
	Material* mat = Zone::New<Material>(Zone::TAG_MATERIALS);
	mat->bLoadedResources = false;
	File* f = File::Open(matfile, "r");
	if(!f) {
		Zone::FastFree(mat, "materials");
		return;
	}
	string contents = f->ReadPlaintext();
	char error[1024];
	cJSON* json = cJSON_ParsePooled(contents.c_str(), error, sizeof(error));
	if(!json) {
		Zone::FastFree(mat, "materials");
		R_Printf("ERROR loading material %s: %s\n", matfile, error);
		return;
	}

	cJSON* child = cJSON_GetObjectItem(json, "name");
	if(!child) {
		Zone::FastFree(mat, "materials");
		R_Printf("WARNING: Material without name (%s)\n", matfile);
		strcpy(mat->name, matfile);
	} else {
		strcpy(mat->name, cJSON_ToString(child));
	}

	child = cJSON_GetObjectItem(json, "diffuseMap");
	if(!child) {
		Zone::FastFree(mat, "materials");
		R_Printf("WARNING: %s doesn't have a diffuse map!\n", mat->name);
	} else {
		strcpy(mat->resourceFile, cJSON_ToString(child));
	}

	materials.insert(make_pair(mat->name, mat));
}

MaterialHandler::MaterialHandler() {
	vector<string> matFiles;
	int numFiles = FS::EXPORT_ListFilesInDir("materials/", matFiles, ".json");
	if(numFiles == 0) {
		R_Printf("WARNING: no materials loaded\n");
		return;
	}
	for(auto it = matFiles.begin(); it != matFiles.end(); ++it) {
		LoadMaterial(it->c_str());
	}
	R_Printf("Loaded %i materials\n", numFiles);
}

MaterialHandler::~MaterialHandler() {
	Zone::FreeAll("materials");
}

Material* MaterialHandler::GetMaterial(const char* material) {
	auto found = materials.find(material);
	if(found == materials.end()) {
		R_Printf("WARNING: material '%s' not found\n", material);
		return NULL;
	}
	return found->second;
}

Material::Material() {
	bLoadedResources = false;
	bLoadedIncorrectly = false;
}

Material::~Material() {
	if(bLoadedResources) {
		FreeResources();
	}
}

void Material::SendToRenderer(float x, float y) {
	if(!bLoadedResources) {
		LoadResources();
	}
	RenderCode::DrawImageAbs((void*)ptResource, x, y);
}

void Material::LoadResources() {
	if(bLoadedResources || bLoadedIncorrectly) {
		// Don't bug us about this again, please.
		return;
	}
	bLoadedResources = true;
	SDL_Surface* temp = IMG_Load(File::GetFileSearchPath(resourceFile).c_str());
	if(!temp) {
		R_Printf("WARNING: %s: could not load diffuse map '%s'\n", name, resourceFile);
		bLoadedResources = false;
		bLoadedIncorrectly = true;
		return;
	}
	bLoadedIncorrectly = false;
	ptResource = SDL_CreateTextureFromSurface((SDL_Renderer*)RenderCode::GetRenderer(), temp);
	SDL_SetTextureBlendMode(ptResource, SDL_BLENDMODE_BLEND);
	SDL_FreeSurface(temp);
}

void Material::FreeResources() {
	if(!bLoadedResources) {
		return;
	}
	SDL_DestroyTexture(ptResource);
	bLoadedResources = false;
	bLoadedIncorrectly = false;
}