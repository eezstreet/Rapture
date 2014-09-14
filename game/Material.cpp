#include "tr_local.h"

Material::Material() {
	bLoadedResources = false;
	bLoadedIncorrectly = false;
	bHasTransparencyMap = false;
}

Material::~Material() {
	if(bLoadedResources) {
		FreeResources();
	}
}

void Material::SendToRenderer(int x, int y) {
	if(!bLoadedResources) {
		LoadResources();
	}

	//RenderCode::DrawImageAbs((Image*)ptResource, x + xOffset, y + yOffset);
}

void Material::SendToRendererTransparency(int x, int y) {
	// Similar to SendToRenderer, but use a transparency map if one is available
	if(!bLoadedResources) {
		LoadResources();
	}

	if(bHasTransparencyMap) {
		//RenderCode::DrawImageAbs((Image*)ptTransResource, x + xOffset, y + yOffset);
	} else {
		//RenderCode::DrawImageAbs((Image*)ptResource, x + xOffset, y + yOffset);
	}
}

void Material::LoadResources() {
	if(bLoadedResources || bLoadedIncorrectly) {
		// Don't bug us about this again, please.
		return;
	}
	bLoadedResources = true;
	// Load diffuse map
	SDL_Surface* temp = IMG_Load(File::GetFileSearchPath(resourceFile).c_str());
	if(!temp) {
		R_Message(PRIORITY_WARNING, "WARNING: %s: could not load diffuse map '%s'\n", name, resourceFile);
		bLoadedResources = false;
		bLoadedIncorrectly = true;
		return;
	}
	bLoadedIncorrectly = false;
	ptResource = RenderCode::TextureFromPixels(temp->pixels, temp->w, temp->h, temp);
	RenderCode::BlendTexture(ptResource);
	SDL_FreeSurface(temp);

	if(bHasTransparencyMap) {
		temp = IMG_Load(File::GetFileSearchPath(transResourceFile).c_str());
		if(!temp) {
			R_Message(PRIORITY_WARNING, "WARNING: %s: could not load trans map '%s'\n", name, transResourceFile);
			bHasTransparencyMap = false;
		} else {
			ptTransResource = RenderCode::TextureFromPixels(temp->pixels, temp->w, temp->h, temp);
			RenderCode::BlendTexture(ptTransResource);
			SDL_FreeSurface(temp);
		}
	}
}

void Material::FreeResources() {
	if(!bLoadedResources) {
		return;
	}
	delete ptResource;
	if(ptTransResource) {
		delete ptTransResource;
	}
	bLoadedResources = false;
	bLoadedIncorrectly = false;
}