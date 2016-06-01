#include "sys_local.h"

Material::Material() {
	bLoadedResources = false;
	bLoadedIncorrectly = false;
	bHasTransparencyMap = false;
	bHasDepthMap = false;
}

void Material::SendToRenderer(int x, int y) {
	if(!bLoadedResources) {
		LoadResources();
	}

	Video::DrawImageAbs(ptResource, x + xOffset, y + yOffset, 0, 0);
}

void Material::SendToRendererTransparency(int x, int y) {
	// Similar to SendToRenderer, but use a transparency map if one is available
	if(!bLoadedResources) {
		LoadResources();
	}

	if(bHasTransparencyMap) {
		Video::DrawImageAbs(ptTransResource, x + xOffset, y + yOffset, 0, 0);
	} else {
		Video::DrawImageAbs(ptTransResource, x + xOffset, y + yOffset, 0, 0);
	}
}

void Material::LoadResources() {
	if(bLoadedResources || bLoadedIncorrectly) {
		// Don't bug us about this again, please.
		return;
	}
	bLoadedResources = true;

	// Load diffuse map
	Texture* temp = Video::RegisterTexture(resourceFile);
	if(!temp) {
		R_Message(PRIORITY_WARNING, "WARNING: %s: could not load diffuse map '%s'\n", name, resourceFile);
		bLoadedResources = false;
		bLoadedIncorrectly = true;
		return;
	}
	bLoadedIncorrectly = false;
	ptResource = temp;

	if(bHasTransparencyMap) {
		temp = Video::RegisterTexture(transResourceFile);
		if(!temp) {
			R_Message(PRIORITY_WARNING, "WARNING: %s: could not load trans map '%s'\n", name, transResourceFile);
			bHasTransparencyMap = false;
		} else {
			ptTransResource = temp;
		}
	}

	if (bHasDepthMap) {
		temp = Video::RegisterTexture(depthResourceFile);
		if (!temp) {
			R_Message(PRIORITY_WARNING, "WARNING: %s: could not load depth map '%s'\n", name, depthResourceFile);
			bHasDepthMap = false;
		}
		else {
			ptDepthResource = temp;
		}
	}

	if (bHasNormalMap) {
		temp = Video::RegisterTexture(normlResourceFile);
		if (!temp) {
			R_Message(PRIORITY_WARNING, "WARNING: %s: could not load normal map '%s'\n", name, normlResourceFile);
			bHasNormalMap = false;
		}
		else {
			ptNormalResource = temp;
		}
	}
}