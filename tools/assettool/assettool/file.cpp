#include "assettool.h"
#include "../../common/SerializedRaptureAsset.h"
#include <cereal/archives/binary.hpp>
#include <iostream>
#include <fstream>

#pragma warning (disable:4996)

AssetFile::AssetFile(const char* path) {
	std::ifstream infile;

	infile.open(path, std::ios::binary);

	cereal::BinaryInputArchive in(infile);
	in >> asset.head;
	
	// Some basic validation of the file
	if (asset.head.header[0] != 'R' ||
		asset.head.header[1] != 'A' ||
		asset.head.header[2] != 'S' ||
		asset.head.header[3] != 'S') {
		// not valid header
		DisplayMessageBox("Open File Error", "Bad header for file", MESSAGEBOX_ERROR);
		infile.close();
		return;
	}

	// Iterate through all of the components
	for (int i = 0; i < asset.head.numberComponents; i++) {
		AssetComponent comp;
		in >> comp;
		decompressedAssets.push_back(comp);
	}

	infile.close();
}

void AssetFile::SaveFile(const char* destination) {
	std::ofstream outfile;
	outfile.open(destination, std::ios::binary);

	cereal::BinaryOutputArchive out(outfile);
	asset.head.numberComponents = decompressedAssets.size();
	out << asset.head;

	for (auto it = decompressedAssets.begin(); it != decompressedAssets.end(); ++it) {
		out << *it;
	}
	outfile.close();
}

void AssetFile::AddNewComponent(ComponentType t, const char* name) {
	AssetComponent comp;
	strcpy(comp.meta.componentName, name);
	comp.meta.componentType = t;
	comp.meta.decompressedSize = 0;
	switch (t) {
		case Asset_Undefined:
			comp.meta.componentVersion = 0;
			comp.data.undefinedComponent = nullptr;
			break;
		case Asset_Data:
			comp.meta.componentVersion = COMP_DATA_VERSION;
			comp.data.dataComponent = new ComponentData();
			strcpy(comp.data.dataComponent->head.mime, "text/plain");
			comp.data.dataComponent->data = nullptr;
			break;
		case Asset_Material:
			comp.meta.componentVersion = COMP_MATERIAL_VERSION;
			comp.data.materialComponent = new ComponentMaterial();
			memset(&comp.data.materialComponent->head, 0, sizeof(comp.data.materialComponent->head));
			comp.data.materialComponent->head.numDirections = 1;
			comp.data.materialComponent->head.fps = 10;
			comp.meta.decompressedSize = sizeof(ComponentMaterial::MaterialHeader);
			break;
		case Asset_Image:
			comp.meta.componentVersion = COMP_IMAGE_VERSION;
			comp.data.imageComponent = new ComponentImage();
			memset(&comp.data.imageComponent->head, 0, sizeof(comp.data.imageComponent->head));
			comp.meta.decompressedSize = sizeof(ComponentImage::ImageHeader);
			break;
		case Asset_Font:
			comp.meta.componentVersion = COMP_FONT_VERSION;
			comp.data.fontComponent = new ComponentFont();
			memset(&comp.data.fontComponent->head, 0, sizeof(comp.data.fontComponent->head));
			comp.data.fontComponent->head.pointSize = 12;
			comp.meta.decompressedSize = sizeof(ComponentFont::FontHeader);
			break;
		case Asset_Level:
			comp.meta.componentVersion = COMP_LEVEL_VERSION;
			comp.data.levelComponent = new ComponentLevel();
			memset(&comp.data.levelComponent->head, 0, sizeof(comp.data.levelComponent->head));
			comp.meta.decompressedSize = sizeof(ComponentLevel::LevelHeader);
			break;
		case Asset_Composition:
			comp.meta.componentVersion = COMP_ANIM_VERSION;
			comp.data.compComponent = new ComponentComp();
			memset(&comp.data.compComponent->head, 0, sizeof(comp.data.compComponent->head));
			comp.meta.decompressedSize = sizeof(ComponentComp::CompHeader);
			break;
		case Asset_Tile:
			comp.meta.componentVersion = COMP_TILE_VERSION;
			comp.data.tileComponent = (ComponentTile*)malloc(sizeof(ComponentTile));
			memset(comp.data.tileComponent, 0, sizeof(ComponentTile));
			break;
	}
	asset.head.numberComponents++;
	decompressedAssets.push_back(comp);
	fileChanged = true;
}

AssetFile::AssetFile(const char* name, const char* author, const char* dlc) {
	strncpy(asset.head.assetName, name, ASSET_NAMELEN);
	strncpy(asset.head.author, author, AUTHOR_NAMELEN);
	strncpy(asset.head.originalAuth, author, AUTHOR_NAMELEN);
	strncpy(asset.head.contentGroup, dlc, GROUP_NAMELEN);
	strncpy(asset.head.header, RASS_HEADER, 4);
	asset.head.compressionType = Compression_None;
	asset.head.compressionLevel = 0;
	asset.head.numberComponents = 0;
	asset.head.version = RASS_VERSION;
}

AssetFile::~AssetFile() {
	ResetPreviewImage();
	for (auto it = decompressedAssets.begin(); it != decompressedAssets.end(); ++it) {
		switch (it->meta.componentType) {
			case Asset_Undefined:
				if (it->data.undefinedComponent != nullptr) {
					free(it->data.undefinedComponent);
				}
				break;
			case Asset_Data:
				if (it->data.dataComponent != nullptr) {
					free(it->data.dataComponent);
				}
				break;
			case Asset_Material:
				if (it->data.materialComponent != nullptr) {
					free(it->data.materialComponent->depthPixels);
					free(it->data.materialComponent->diffusePixels);
					free(it->data.materialComponent->normalPixels);
					delete it->data.materialComponent;
				}
				break;
			case Asset_Image:
				if (it->data.imageComponent != nullptr) {
					free(it->data.imageComponent->pixels);
					delete it->data.imageComponent;
				}
				break;
			case Asset_Font:
				if (it->data.fontComponent != nullptr) {
					free(it->data.fontComponent->fontData);
					delete it->data.fontComponent;
				}
				break;
			case Asset_Level:
				if (it->data.levelComponent != nullptr) {
					free(it->data.levelComponent->ents);
					free(it->data.levelComponent->tiles);
					delete it->data.levelComponent;
				}
				break;
			case Asset_Composition:
				if (it->data.compComponent != nullptr) {
					free(it->data.compComponent->components);
					free(it->data.compComponent->keyframes);
					delete it->data.compComponent;
				}
				break;
			case Asset_Tile:
				if (it->data.tileComponent != nullptr) {
					free(it->data.tileComponent);
				}
				break;
		}
	}
	decompressedAssets.clear();
}

void AssetFile::DeleteComponent(int componentNum) {
	if (componentNum == -1) {
		return;
	}
	int i = 0;
	for (auto it = decompressedAssets.begin(); it != decompressedAssets.end(); it++) {
		if (i == componentNum) {
			decompressedAssets.erase(it);
			return;
		}
		i++;
	}
	fileChanged = true;
}