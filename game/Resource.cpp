#include "sys_local.h"

string assetURI = "asset://";

extern unordered_map<string, AssetComponent*> m_assetComponents;

Resource::Resource() {
	szAssetFile = szComponent = "";
	bRetrieved = false;
	component = nullptr;
}

Resource* Resource::ResourceAsync(const char* asset, const char* component, assetRequestCallback callback) {
	Resource* pRes = new Resource();
	if (pRes == nullptr) {
		return pRes;
	}

	pRes->szAssetFile = asset;
	pRes->szComponent = component;
	transform(pRes->szAssetFile.begin(), pRes->szAssetFile.end(), pRes->szAssetFile.begin(), ::tolower);
	transform(pRes->szComponent.begin(), pRes->szComponent.end(), pRes->szComponent.begin(), ::tolower);
	Filesystem::QueueResource(pRes, callback);

	return pRes;
}

Resource* Resource::ResourceAsyncURI(const char* uri, assetRequestCallback callback) {
	// Parse out asset/component
	string szAsset, szComponent;
	smatch rmMatch;
	regex rxStr("^(?:asset://)?([^/]*)/([^/]*)$");
	string szURI(uri);

	regex_match(szURI, rmMatch, rxStr);
	if (rmMatch.empty()) {
		// regex match failed
		R_Message(PRIORITY_WARNING, "Resource::ResourceAsyncURI: malformed URI: '%s'\n", uri);
		return nullptr;
	}
	
	szAsset = rmMatch[1];
	szComponent = rmMatch[2];
	return ResourceAsync(szAsset.c_str(), szComponent.c_str(), callback);
}

Resource* Resource::ResourceSync(const char* asset, const char* component) {
	Resource* pRes = new Resource();
	pRes->szAssetFile = asset;
	pRes->szComponent = component;
	pRes->DequeRetrieve(nullptr);
	return pRes;
}

Resource* Resource::ResourceSyncURI(const char* uri) {
	// Parse out asset/component
	string szAsset, szComponent;
	string szURI = uri;
	smatch rmMatch;
	regex rxStr("^(?:asset://)?([^/]*)/([^/]*)$");

	if (regex_match(szURI, rmMatch, rxStr)) {
		ssub_match first = rmMatch[1];
		ssub_match second = rmMatch[2];
		szAsset = first.str();
		szComponent = second.str();
		return ResourceSync(szAsset.c_str(), szComponent.c_str());
	}
	else {
		return nullptr;
	}
}

void Resource::FreeResource(Resource* pResource) {
	delete pResource;
}

void Resource::DequeRetrieve(assetRequestCallback callback) {
	bool found = true;
	string fullStr = szAssetFile + '/' + szComponent;
	if (m_assetComponents.find(fullStr) == m_assetComponents.end()) {
		// The asset file hasn't been opened
		RaptureAsset* rap = (RaptureAsset*)Zone::Alloc(sizeof(RaptureAsset), "files");
		Filesystem::LoadRaptureAsset(&rap, szAssetFile);

		// Toss all of the components from the asset file into the asset components map
		for (int i = 0; i < rap->head.numberComponents; i++) {
			AssetComponent* pComp = &rap->components[i];
			string compName = szAssetFile + '/' + pComp->meta.componentName;
			transform(compName.begin(), compName.end(), compName.begin(), ::tolower);
			m_assetComponents[compName] = pComp;
		}

		// Try and find it again
		auto it = m_assetComponents.find(fullStr);
		if (it == m_assetComponents.end()) {
			R_Message(PRIORITY_WARNING, "Component %s not found in asset %s\n", szComponent.c_str(), rap->head.assetName);
			this->bBad = true;
			found = false;
		}
		else {
			component = it->second;
		}
	}
	else {
		component = m_assetComponents[fullStr];
		found = true;
	}

	if (!found) {
		return;
	}
	if (callback) {
		callback(component);
	}
	bRetrieved = true;
}

bool Resource::Bad() {
	if (!bBad) {
		return false;
	}
	bBad = false;
	return true;
}

bool Resource::Retrieved() {
	if (!bRetrieved) {
		return false;
	}
	bRetrieved = false;
	return true;
}