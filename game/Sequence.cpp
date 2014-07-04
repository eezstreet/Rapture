#include "tr_local.h"
#include "../json/cJSON.h"

void Sequence::Parse(void* in) {
	cJSON* root = (cJSON*)in;
	cJSON* node;

	node = cJSON_GetObjectItem(root, "name");
	if(!node) {
		R_Printf("WARNING: Sequence without name\n");
		return;
	} else {
		strncpy(name, cJSON_ToString(node), sizeof(name));
	}

	node = cJSON_GetObjectItem(root, "row");
	rowNum = cJSON_ToIntegerOpt(root, 0);

	node = cJSON_GetObjectItem(root, "fps");
	fps = cJSON_ToIntegerOpt(root, 20);

	node = cJSON_GetObjectItem(root, "initial");
	bInitial = cJSON_ToBooleanOpt(node, false);

	node = cJSON_GetObjectItem(root, "numframes");
	frameCount = cJSON_ToIntegerOpt(root, 1);

	node = cJSON_GetObjectItem(root, "loop");
	bLoop = cJSON_ToBooleanOpt(node, true);
}