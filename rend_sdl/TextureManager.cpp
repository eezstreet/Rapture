#include "tr_local.h"

TextureManager* ptTexMan = nullptr;

TextureManager::TextureManager() {
	nLastUnnamed = 0;
}

TextureManager:: ~TextureManager() {
	for (auto it = umTextureMap.begin(); it != umTextureMap.end(); ++it) {
		delete it->second;
	}
	umTextureMap.clear();
}

TextureManager::TextureManager(const TextureManager& other) {
	umTextureMap = other.umTextureMap;
	nLastUnnamed = other.nLastUnnamed;
}

Texture* TextureManager::RegisterTexture(const char* name) {
	Texture* found = nullptr;

	// Make sure our string is all lowercase first
	string sName = name;
	transform(sName.begin(), sName.end(), sName.begin(), ::tolower);

	auto texture = umTextureMap.find(sName);
	if (texture == umTextureMap.end()) {
		found = new Texture(sName.c_str());
		umTextureMap[sName] = found;
		return found;
	}
	return texture->second;
}

Texture* TextureManager::RegisterTexture(Texture* newTexture) {
	string sName = "__unnamed" + nLastUnnamed++;
	umTextureMap[sName] = newTexture;

	return newTexture;
}