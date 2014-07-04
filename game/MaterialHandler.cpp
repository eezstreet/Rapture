#include "tr_local.h"
#include "../json/cJSON.h"

MaterialHandler* mats = NULL;

void MaterialHandler::LoadMaterial(const char* matfile) {
	Material* mat = Zone::New<Material>(Zone::TAG_MATERIALS);
	mat->bLoadedResources = false;
	mat->ptAnims = nullptr;
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

	child = cJSON_GetObjectItem(json, "transMap");
	if(!child) {
		mat->bHasTransparencyMap = false;
	} else {
		strcpy(mat->transResourceFile, cJSON_ToString(child));
		mat->bHasTransparencyMap = true;
	}

	child = cJSON_GetObjectItem(json, "xOffset");
	if(!child) {
		mat->xOffset = 0;
	} else {
		mat->xOffset = cJSON_ToInteger(child);
	}

	child = cJSON_GetObjectItem(json, "yOffset");
	if(!child) {
		mat->yOffset = 0;
	} else {
		mat->yOffset = cJSON_ToInteger(child);
	}

	child = cJSON_GetObjectItem(json, "sequences");
	if(!child) {
		mat->iNumSequences = 0;
	} else {
		int longestSequence = 0;
		mat->iNumSequences = cJSON_GetArraySize(child);
		string sStartingSequence = "";
		for(int i = 0; i < mat->iNumSequences; i++) {
			Sequence seq;
			seq.Parse(cJSON_GetArrayItem(child, i));
			if(seq.bInitial) {
				sStartingSequence = seq.name;
			}
			if(seq.frameCount > longestSequence) {
				longestSequence = seq.frameCount;
			}
			mat->mSequences[seq.name] = seq;
		}
		if(sStartingSequence.length() <= 0) {
			R_Error("%s: no starting sequence (one sequence needs \"initial\" to be true)", matfile);
			return;
		}
		// Parse the sequencedata
		SequenceData sData;
		cJSON* seqData = cJSON_GetObjectItem(json, "sequenceData");
		if(!seqData) {
			R_Error("%s has sequences but no sequenceData!\n", matfile);
			return;
		}

		sData.rowheight = cJSON_ToIntegerOpt(cJSON_GetObjectItem(seqData, "rowheight"), 0);
		sData.framesize = cJSON_ToIntegerOpt(cJSON_GetObjectItem(seqData, "framesize"), 0);

		mat->ptAnims = new AnimationManager(sStartingSequence, &mat->mSequences, longestSequence, sData);
	}

	materials.insert(make_pair(mat->name, mat));
	f->Close();
}

MaterialHandler::MaterialHandler() {
	int numFiles = 0;
	char** matFiles = FileSystem::EXPORT_ListFilesInDir("materials/", ".json", &numFiles);
	if(numFiles == 0) {
		R_Printf("WARNING: no materials loaded\n");
		return;
	}
	for(int i = 0; i < numFiles; i++) {
		LoadMaterial(matFiles[i]);
	}
	FileSystem::FreeFileList(matFiles, numFiles);
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