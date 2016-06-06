#include "assettool.h"
#include <iostream>
#include <fstream>

#pragma warning (disable:4996)

// Reading each type of component
ComponentData* LoadDataComponent(std::ifstream& infile, uint64_t decompSize) {
	ComponentData* data = (ComponentData*)malloc(decompSize);
	infile.read((char*)data, decompSize);
	return data;
}

ComponentMaterial* LoadMaterialComponent(std::ifstream& infile, uint64_t decompSize) {
	ComponentMaterial* mat = new ComponentMaterial();
	infile.read((char*)&mat->head, sizeof(ComponentMaterial::MaterialHeader));
	
	if (mat->head.mapsPresent & (1 << Maptype_Diffuse)) {
		size_t diffuseSize = mat->head.width * mat->head.height * sizeof(uint32_t);
		if (diffuseSize > 0) {
			mat->diffusePixels = (uint32_t*)malloc(diffuseSize);
			infile.read((char*)mat->diffusePixels, diffuseSize);
		}
	}
	if (mat->head.mapsPresent & (1 << Maptype_Normal)) {
		size_t normalSize = mat->head.width * mat->head.height * sizeof(uint32_t);
		if (normalSize > 0) {
			mat->normalPixels = (uint32_t*)malloc(normalSize);
			infile.read((char*)mat->normalPixels, normalSize);
		}
	}
	if (mat->head.mapsPresent & (1 << Maptype_Depth)) {
		size_t depthSize = mat->head.width * mat->head.height * sizeof(uint8_t);
		if (depthSize > 0) {
			mat->depthPixels = (uint8_t*)malloc(depthSize);
			infile.read((char*)mat->depthPixels, depthSize);
		}
	}
	return mat;
}

ComponentImage* LoadImageComponent(std::ifstream& infile, uint64_t decompSize) {
	ComponentImage* img = new ComponentImage();
	infile.read((char*)&img->head, sizeof(ComponentImage::ImageHeader));

	size_t pixelsSize = img->head.width * img->head.height * sizeof(uint32_t);
	if (pixelsSize > 0) {
		img->pixels = (uint32_t*)malloc(pixelsSize);
		infile.read((char*)img->pixels, pixelsSize);
	}
	return img;
}

ComponentFont* LoadFontComponent(std::ifstream& infile, uint64_t decompSize) {
	ComponentFont* font = new ComponentFont();
	infile.read((char*)&font->head, sizeof(ComponentFont::FontHeader));
	if (decompSize > 0) {
		size_t fontFileSize = decompSize - sizeof(ComponentFont::FontHeader);

		if (fontFileSize > 0) {
			font->fontData = (uint8_t*)malloc(fontFileSize);
			infile.read((char*)font->fontData, fontFileSize);
		}
	}
	return font;
}

ComponentLevel* LoadLevelComponent(std::ifstream& infile, uint64_t decompSize) {
	ComponentLevel* lvl = new ComponentLevel();
	infile.read((char*)&lvl->head, sizeof(ComponentLevel::LevelHeader));
	size_t tileSize = sizeof(ComponentLevel::TileEntry) * lvl->head.numTiles;
	size_t entSize = sizeof(ComponentLevel::EntityEntry) * lvl->head.numEntities;
	if (tileSize > 0) {
		lvl->tiles = (ComponentLevel::TileEntry*)malloc(tileSize);
		infile.read((char*)lvl->tiles, tileSize);
	}
	if (entSize > 0) {
		lvl->ents = (ComponentLevel::EntityEntry*)malloc(entSize);
		infile.read((char*)lvl->ents, entSize);
	}

	return lvl;
}

ComponentComp* LoadCompComponent(std::ifstream& infile, uint64_t decompSize) {
	ComponentComp* comp = new ComponentComp();
	infile.read((char*)&comp->head, sizeof(ComponentComp::CompHeader));
	
	size_t compSize = comp->head.numComponents * sizeof(ComponentComp::CompComponent);
	size_t kfSize = comp->head.numKeyframes * sizeof(ComponentComp::CompKeyframe);
	if (compSize > 0) {
		comp->components = (ComponentComp::CompComponent*)malloc(compSize);
		infile.read((char*)comp->components, compSize);
	}
	if (kfSize > 0) {
		comp->keyframes = (ComponentComp::CompKeyframe*)malloc(kfSize);
		infile.read((char*)comp->keyframes, kfSize);
	}
	return comp;
}

ComponentTile* LoadTileComponent(std::ifstream& infile, uint64_t decompSize) {
	ComponentTile* tile = (ComponentTile*)malloc(sizeof(ComponentTile));
	infile.read((char*)tile, sizeof(ComponentTile));
	return tile;
}

// Writing each type of component
void WriteMaterialComponent(std::ofstream& outfile, AssetComponent* ptComponent) {
	ComponentMaterial* mat = ptComponent->data.materialComponent;
	if (mat != nullptr) {
		size_t imageSize = mat->head.width * mat->head.height;
		outfile.write((const char*)&mat->head, sizeof(ComponentMaterial::MaterialHeader));
		if (mat->head.mapsPresent & (1 << Maptype_Diffuse) && imageSize > 0) {
			outfile.write((const char*)mat->diffusePixels, imageSize * sizeof(uint32_t));
		}
		if (mat->head.mapsPresent & (1 << Maptype_Normal) && imageSize > 0) {
			outfile.write((const char*)mat->normalPixels, imageSize * sizeof(uint32_t));
		}
		if (mat->head.mapsPresent & (1 << Maptype_Depth) && imageSize > 0) {
			outfile.write((const char*)mat->depthPixels, imageSize * sizeof(uint8_t));
		}
	}
}

void WriteImageComponent(std::ofstream& outfile, AssetComponent* ptComponent) {
	ComponentImage* img = ptComponent->data.imageComponent;
	if (img != nullptr) {
		size_t imageSize = img->head.width * img->head.height;
		outfile.write((const char*)&img->head, sizeof(ComponentImage::ImageHeader));
		if (imageSize > 0) {
			outfile.write((const char*)img->pixels, imageSize * sizeof(uint32_t));
		}
	}
}

void WriteFontComponent(std::ofstream& outfile, AssetComponent* ptComponent) {
	ComponentFont* font = ptComponent->data.fontComponent;
	if (font != nullptr) {
		outfile.write((const char*)&font->head, sizeof(ComponentFont::FontHeader));

		size_t fontSize = ptComponent->meta.decompressedSize - sizeof(ComponentFont::FontHeader);
		if (fontSize > 0) {
			outfile.write((const char*)font->fontData, fontSize);
		}
	}
}

void WriteLevelComponent(std::ofstream& outfile, AssetComponent* ptComponent) {
	ComponentLevel* lvl = ptComponent->data.levelComponent;
	if (lvl != nullptr) {
		size_t tileSize = sizeof(ComponentLevel::TileEntry) * lvl->head.numTiles;
		size_t entSize = sizeof(ComponentLevel::EntityEntry) * lvl->head.numEntities;
		outfile.write((const char*)&lvl->head, sizeof(ComponentLevel::LevelHeader));
		if (tileSize > 0) {
			outfile.write((const char*)lvl->tiles, tileSize);
		}
		if (entSize > 0) {
			outfile.write((const char*)lvl->ents, entSize);
		}
	}
}

void WriteCompComponent(std::ofstream& outfile, AssetComponent* ptComponent) {
	ComponentComp* comp = ptComponent->data.compComponent;

	if (comp != nullptr) {
		size_t compSize = sizeof(ComponentComp::CompComponent) * comp->head.numComponents;
		size_t kfSize = sizeof(ComponentComp::CompKeyframe) * comp->head.numKeyframes;

		outfile.write((const char*)&comp->head, sizeof(ComponentComp::CompHeader));
		if (compSize > 0) {
			outfile.write((const char*)comp->components, compSize);
		}
		if (kfSize > 0) {
			outfile.write((const char*)comp->keyframes, kfSize);
		}
	}
}

void WriteTileComponent(std::ofstream& outfile, AssetComponent* ptComponent) {
	ComponentTile* tile = ptComponent->data.tileComponent;
	if (tile != nullptr) {
		outfile.write((const char*)tile, sizeof(ComponentTile));
	}
}

AssetFile::AssetFile(const char* path) {
	std::ifstream infile;

	hasErrors = false;
	infile.open(path);

	infile.read((char*)&asset.head, sizeof(asset.head));
	
	// Some basic validation of the file
	if (asset.head.header[0] != 'R' ||
		asset.head.header[1] != 'A' ||
		asset.head.header[2] != 'S' ||
		asset.head.header[3] != 'S') {
		// not valid header
		DisplayMessageBox("Open File Error", "Bad header for file", MESSAGEBOX_ERROR);
		infile.close();
		hasErrors = true;
		return;
	}

	if (asset.head.version < RASS_VERSION) {
		DisplayMessageBox("Outdated File",
			"This asset file has an outdated version. The file will load, but there may be instability and crashes.", MESSAGEBOX_WARNING);
	}

	if (asset.head.version > RASS_VERSION) {
		DisplayMessageBox("Outdated Editor",
			"The RAT has detected that your editor is out of date for this file. It is recommended that you upgrade as soon as possible.", MESSAGEBOX_WARNING);
	}

	if (asset.head.numberComponents <= 0) {
		infile.close();
		return;
	}

	// Iterate through all of the components
	for (int i = 0; i < asset.head.numberComponents; i++) {
		AssetComponent comp;
		infile.read((char*)&comp.meta, sizeof(comp.meta));

		switch (comp.meta.componentType) {
			case Asset_Undefined:
				if (comp.meta.decompressedSize > 0) {
					comp.data.undefinedComponent = malloc(comp.meta.decompressedSize);
					infile.read((char*)comp.data.undefinedComponent, comp.meta.decompressedSize);
				}
				break;
			case Asset_Data:
				// slight hack
				if (comp.meta.componentVersion < COMP_DATA_VERSION) {
					DisplayMessageBox("Outdated Component", "Found a data component that is outdated. RAT will read it, but the data may not be correct.", MESSAGEBOX_INFO);
				}
				else if (comp.meta.componentVersion > COMP_DATA_VERSION) {
					DisplayMessageBox("Outdated Tool", "The RAT detected a data component that is newer than the tool. RAT will read it, but the data may not be correct.", MESSAGEBOX_INFO);
				}
				if (comp.meta.decompressedSize > 0) {
					comp.data.dataComponent = (ComponentData*)malloc(comp.meta.decompressedSize);
					infile.read((char*)comp.data.dataComponent, comp.meta.decompressedSize);
				}
				break;
			case Asset_Material:
				if (comp.meta.componentVersion < COMP_MATERIAL_VERSION) {
					DisplayMessageBox("Outdated Component", "Found a material component that is outdated. RAT will read it, but the data may not be correct.", MESSAGEBOX_INFO);
				}
				else if (comp.meta.componentVersion > COMP_MATERIAL_VERSION) {
					DisplayMessageBox("Outdated Tool", "The RAT detected a material component that is newer than the tool. RAT will read it, but the data may not be correct.", MESSAGEBOX_INFO);
				}
				comp.data.materialComponent = LoadMaterialComponent(infile, comp.meta.decompressedSize);
				break;
			case Asset_Image:
				if (comp.meta.componentVersion < COMP_IMAGE_VERSION) {
					DisplayMessageBox("Outdated Component", "Found an image component that is outdated. RAT will read it, but the data may not be correct.", MESSAGEBOX_INFO);
				}
				else if (comp.meta.componentVersion > COMP_IMAGE_VERSION) {
					DisplayMessageBox("Outdated Tool", "The RAT detected an image component that is newer than the tool. RAT will read it, but the data may not be correct.", MESSAGEBOX_INFO);
				}
				comp.data.imageComponent = LoadImageComponent(infile, comp.meta.decompressedSize);
				break;
			case Asset_Font:
				if (comp.meta.componentVersion < COMP_FONT_VERSION) {
					DisplayMessageBox("Outdated Component", "Found a font component that is outdated. RAT will read it, but the data may not be correct.", MESSAGEBOX_INFO);
				}
				else if (comp.meta.componentVersion > COMP_FONT_VERSION) {
					DisplayMessageBox("Outdated Tool", "The RAT detected a font component that is newer than the tool. RAT will read it, but the data may not be correct.", MESSAGEBOX_INFO);
				}
				comp.data.fontComponent = LoadFontComponent(infile, comp.meta.decompressedSize);
				break;
			case Asset_Level:
				if (comp.meta.componentVersion < COMP_LEVEL_VERSION) {
					DisplayMessageBox("Outdated Component", "Found a level component that is outdated. RAT will read it, but the data may not be correct.", MESSAGEBOX_INFO);
				}
				else if (comp.meta.componentVersion > COMP_LEVEL_VERSION) {
					DisplayMessageBox("Outdated Tool", "The RAT detected a level component that is newer than the tool. RAT will read it, but the data may not be correct.", MESSAGEBOX_INFO);
				}
				comp.data.levelComponent = LoadLevelComponent(infile, comp.meta.decompressedSize);
				break;
			case Asset_Composition:
				if (comp.meta.componentVersion < COMP_ANIM_VERSION) {
					DisplayMessageBox("Outdated Component", "Found an animation component that is outdated. RAT will read it, but the data may not be correct.", MESSAGEBOX_INFO);
				}
				else if (comp.meta.componentVersion > COMP_ANIM_VERSION) {
					DisplayMessageBox("Outdated Tool", "The RAT detected an animation component that is newer than the tool. RAT will read it, but the data may not be correct.", MESSAGEBOX_INFO);
				}
				comp.data.compComponent = LoadCompComponent(infile, comp.meta.decompressedSize);
				break;
			case Asset_Tile:
				if (comp.meta.componentVersion < COMP_TILE_VERSION) {
					DisplayMessageBox("Outdated Component", "Found a tile component that is outdated. RAT will read it, but the data may not be correct.", MESSAGEBOX_INFO);
				}
				else if (comp.meta.componentVersion > COMP_TILE_VERSION) {
					DisplayMessageBox("Outdated Tool", "The RAT detected a tile component that is newer than the tool. RAT will read it, but the data may not be correct.", MESSAGEBOX_INFO);
				}
				comp.data.tileComponent = LoadTileComponent(infile, comp.meta.decompressedSize);
				break;
			default:
				DisplayMessageBox("Warning", "Found a component to this asset which we can't identify...alert eezstreet", MESSAGEBOX_INFO);
				break;
		}
		decompressedAssets.push_back(comp);
	}

	infile.close();
}

void AssetFile::SaveFile(const char* destination) {
	std::ofstream outfile;
	outfile.open(destination);

	outfile.write((const char*)&asset.head, sizeof(asset.head));

	for (auto it = decompressedAssets.begin(); it != decompressedAssets.end(); ++it) {
		outfile.write((const char*)&it->meta, sizeof(it->meta));

		switch (it->meta.componentType) {
			case Asset_Undefined:
				if (it->data.undefinedComponent != nullptr) {
					outfile.write((const char*)it->data.undefinedComponent, it->meta.decompressedSize);
				}
				break;
			case Asset_Data:
				if (it->data.dataComponent != nullptr) {
					outfile.write((const char*)it->data.dataComponent, it->meta.decompressedSize);
				}
				break;
			case Asset_Material:
				WriteMaterialComponent(outfile, &(*it));
				break;
			case Asset_Image:
				WriteImageComponent(outfile, &(*it));
				break;
			case Asset_Font:
				WriteFontComponent(outfile, &(*it));
				break;
			case Asset_Level:
				WriteLevelComponent(outfile, &(*it));
				break;
			case Asset_Composition:
				WriteCompComponent(outfile, &(*it));
				break;
			case Asset_Tile:
				WriteTileComponent(outfile, &(*it));
				break;
		}
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
			comp.data.dataComponent = nullptr;
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