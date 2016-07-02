#include "tr_local.h"

unordered_map<string, Material*> Material::umMaterials;

/* Class methods */
Material::Material(const char* szURI) : bValid(false) {
	Resource* pRes = trap->ResourceSyncURI(szURI);
	if (pRes == nullptr) {
		trap->Print(PRIORITY_WARNING, "Couldn't find material resource: %s\n", szURI);
		return;
	}
	AssetComponent* comp = trap->ResourceComponent(pRes);
	if (comp == nullptr || comp->meta.componentType != Asset_Material || comp->data.materialComponent == nullptr) {
		trap->Print(PRIORITY_WARNING, "Resource %s was attempted to load as material, but isn't.\n", szURI);
		return;
	}

	ComponentMaterial* mat = comp->data.materialComponent;
	this->matHeader = mat->head;
	if (matHeader.mapsPresent & (1 << Maptype_Diffuse)) {
		diffuseTexture = new Texture(matHeader.width, matHeader.height, mat->diffusePixels);
	}
	if (matHeader.mapsPresent & (1 << Maptype_Depth)) {
		depthTexture = new Texture(matHeader.depthWidth, matHeader.depthHeight, mat->depthPixels);
	}
	if (matHeader.mapsPresent & (1 << Maptype_Normal)) {
		normalTexture = new Texture(matHeader.normalWidth, matHeader.normalHeight, mat->normalPixels);
	}
	bValid = true;
}

Material::~Material() {
	if (diffuseTexture) {
		delete diffuseTexture;
	}
	if (normalTexture) {
		delete normalTexture;
	}
	if (depthTexture) {
		delete depthTexture;
	}
}

Material::Material(Material& other) {
	matHeader = other.matHeader;
	diffuseTexture = other.diffuseTexture;
	normalTexture = other.normalTexture;
	depthTexture = other.depthTexture;
	bValid = other.bValid;
}

void Material::Draw(float xPct, float yPct, float wPct, float hPct) {
	if (diffuseTexture) {
		diffuseTexture->DrawImage(xPct, yPct, wPct, hPct);
	}
}

void Material::DrawAspectCorrection(float xPct, float yPct, float wPct, float hPct) {
	if (diffuseTexture) {
		diffuseTexture->DrawImageAspectCorrection(xPct, yPct, wPct, hPct);
	}
}

void Material::DrawClipped(float sxPct, float syPct, float swPct, float shPct, float ixPct, float iyPct, float iwPct, float ihPct) {
	if (diffuseTexture) {

	}
}

void Material::DrawAbs(int nX, int nY, int nW, int nH) {
	if (diffuseTexture) {
		diffuseTexture->DrawAbs(nX, nY, nW, nH);
	}
}

void Material::DrawAbsClipped(int sX, int sY, int sW, int sH, int iX, int iY, int iW, int iH) {
	if (diffuseTexture) {
		diffuseTexture->DrawAbsClipped(sX, sY, sW, sH, iX, iY, iW, iH);
	}
}

/* Static methods */
Material* Material::Register(const char* uri) {
	string szURI = uri;

	transform(szURI.begin(), szURI.end(), szURI.end(), ::tolower);
	
	auto it = umMaterials.find(szURI);
	if (it != umMaterials.end()) {
		return it->second;
	}

	Material* newMat = new Material(uri);
	if (!newMat->Valid()) {
		delete newMat;
		return nullptr;
	}
	umMaterials[szURI] = newMat;
	return newMat;
}

void Material::DrawMaterial(Material* pMat, float xPct, float yPct, float wPct, float hPct) {
	if (pMat == nullptr) {
		return;
	}
	pMat->Draw(xPct, yPct, wPct, hPct);
}

void Material::DrawMaterialAspectCorrection(Material* pMat, float xPct, float yPct, float wPct, float hPct) {
	if (pMat == nullptr) {
		return;
	}
	pMat->DrawAspectCorrection(xPct, yPct, wPct, hPct);
}

void Material::DrawMaterialClipped(Material* pMat, float sxPct, float syPct, float swPct, float shPct, float ixPct, float iyPct, float iwPct, float ihPct) {
	if (pMat == nullptr) {
		return;
	}
	pMat->DrawClipped(sxPct, syPct, swPct, shPct, ixPct, iyPct, iwPct, ihPct);
}

void Material::DrawMaterialAbs(Material* pMat, int nX, int nY, int nW, int nH) {
	if (pMat == nullptr) {
		return;
	}
	pMat->DrawAbs(nX, nY, nW, nH);
}

void Material::DrawMaterialAbsClipped(Material* pMat, int sX, int sY, int sW, int sH, int iX, int iY, int iW, int iH) {
	if (pMat == nullptr) {
		return;
	}
	pMat->DrawAbsClipped(sX, sY, sW, sH, iX, iY, iW, iH);
}

void Material::KillAllMaterials() {
	for (auto it = umMaterials.begin(); it != umMaterials.end(); ++it) {
		delete it->second;
	}
	umMaterials.clear();
}