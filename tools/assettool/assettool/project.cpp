#include "assettool.h"
#include <iostream>
#include <SDL_image.h>
#include <SDL_ttf.h>

#define MAX_PROJECTFILE_LEN	32967
#define MAX_JSON_ERRORLEN	128
#define MAX_ASSETFILE_LEN	131072
#define MAX_OUTFILE_PATH	160

#pragma warning(disable:4996)

// Reads a file for a component
bool ReadFileForComponent(void** buffer, uint64_t& decompressedSize, const char* filePath) {
	// Open the file
	FILE* fp = fopen(filePath, "rb+");
	if (fp == nullptr) {
		std::cout << "Failed to open " << filePath << " for reading!" << std::endl;
		return false;
	}

	// Figure out how big the file is
	decompressedSize = 0;
	fseek(fp, decompressedSize, SEEK_END);
	decompressedSize = ftell(fp);

	if (decompressedSize == 0) {
		std::cout << "File " << filePath << " is empty!" << std::endl;
		fclose(fp);
		return false;
	}

	// rewind and read, also allocate memory
	rewind(fp);
	*buffer = malloc(decompressedSize);
	size_t numRead = fread(*buffer, 1, decompressedSize, fp);
	fclose(fp);
	if (numRead == 0) {
		std::cout << "Failed to read " << filePath << std::endl;
		free(*buffer);
		return false;
	}

	return true;
}

// Makes absolutely positively certain that a surface is always in the correct format, always.
SDL_Surface* DoOverbearingSurfaceCheck(SDL_Surface* in) {
	static const SDL_PixelFormat format{
		SDL_MasksToPixelFormatEnum(32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF),
		nullptr,
		32,
		4,
		{ 0 },
		0xFF000000,
		0x00FF0000,
		0x0000FF00,
		0x000000FF,
		0
	};
	return SDL_ConvertSurface(in, &format, 0);
}

// Processes a single undefined component from an asset manifest file
void ProcessUndefinedComponent(AssetComponent& component, cJSON* json) {
	cJSON* child = nullptr;
	child = cJSON_GetObjectItem(json, "file");
	if (child) {
		const char* filePath = cJSON_ToString(child);
		std::cout << "Opening file " << filePath << std::endl;
		ReadFileForComponent(&component.data.undefinedComponent, component.meta.decompressedSize, filePath);
	}
	else {
		child = cJSON_GetObjectItem(json, "data");
		if (child) {
			const char* data = cJSON_ToString(child);
			component.meta.decompressedSize = strlen(data);
			component.data.undefinedComponent = malloc(component.meta.decompressedSize);
			strcpy((char*)component.data.undefinedComponent, data);
		}
	}
}

// Processes a single data component from an asset schema file
void ProcessDataComponent(AssetComponent& component, cJSON* json) {
	cJSON* child = nullptr;
	bool bAutoGuessMimeType = false;

	component.data.dataComponent = (ComponentData*)malloc(sizeof(ComponentData));
	ComponentData* pData = component.data.dataComponent;
	child = cJSON_GetObjectItem(json, "mimeType");
	if (child) {
		const char* text = cJSON_ToString(child);
		if (!stricmp(text, "auto")) {
			// Determine the MIME type based on the extension of the file automatically
			bAutoGuessMimeType = true;
		}
		else {
			strcpy(pData->head.mime, text);
		}
	}

	child = cJSON_GetObjectItem(json, "file");
	if (child) {
		const char* filePath = cJSON_ToString(child);
		ReadFileForComponent((void**)&pData->data, component.meta.decompressedSize, filePath);
	}
	else {
		child = cJSON_GetObjectItem(json, "data");
		if (child) {
			if (bAutoGuessMimeType) {
				std::cout << "Component '" << component.meta.componentName << "' has 'auto' mimetype but no file!!" << std::endl;
				std::cout << "text/plain will be used for this component instead." << std::endl;
				strcpy(pData->head.mime, "text/plain");
			}
			const char* text = cJSON_ToString(child);
			component.meta.decompressedSize = strlen(text);
			pData->data = (char*)malloc(component.meta.decompressedSize);
			strcpy(pData->data, text);
		}
	}
}

// Processes a single material component from an asset schema file
void ProcessMaterialComponent(AssetComponent& component, cJSON* json) {
	cJSON* child = nullptr;

	return; // TODO
	/*
	component.data.materialComponent = (ComponentMaterial*)malloc(sizeof(ComponentMaterial));
	memset(component.data.materialComponent, 0, sizeof(ComponentMaterial));

	ComponentMaterial* pMat = component.data.materialComponent;
	
	child = cJSON_GetObjectItem(json, "numDirections");
	if (child) {
		pMat->head.numDirections = cJSON_ToInteger(child);
	}

	child = cJSON_GetObjectItem(json, "xoffset");
	if (child) {
		pMat->head.xoffset = cJSON_ToInteger(child);
	}

	child = cJSON_GetObjectItem(json, "yoffset");
	if (child) {
		pMat->head.yoffset = cJSON_ToInteger(child);
	}

	child = cJSON_GetObjectItem(json, "fps");
	if (child) {
		pMat->head.fps = cJSON_ToInteger(child);
	}

	// The rest of the data from the header gets interpretted from the following
	child = cJSON_GetObjectItem(json, "diffuseFile");
	if (child) {
		const char* diffuseFile = cJSON_ToString(child);
		SDL_Surface* diffuseSurf = IMG_Load(diffuseFile);
		if (diffuseSurf == nullptr) {
			std::cout << "Couldn't load diffuseFile " << diffuseFile << std::endl;
			std::cout << "(Reason: " << IMG_GetError() << ")" << std::endl;
		}
		else {
			pMat->head.mapsPresent |= (1 << Maptype_Diffuse);
			pMat->head.width = diffuseSurf->w;
			pMat->head.height = diffuseSurf->h;
			pMat->head.frameWidth = diffuseSurf->w;
			pMat->head.frameHeight = diffuseSurf->h / pMat->head.numDirections;
			SDL_Surface* checkedSurf = DoOverbearingSurfaceCheck(diffuseSurf);

			size_t pixelsSize = sizeof(uint32_t) * pMat->head.width * pMat->head.height;
			pMat->diffusePixels = (uint32_t*)malloc(pixelsSize);
			SDL_LockSurface(checkedSurf);
			memcpy(pMat->diffusePixels, checkedSurf->pixels, pixelsSize);
			SDL_UnlockSurface(checkedSurf);

			SDL_FreeSurface(checkedSurf);
			SDL_FreeSurface(diffuseSurf);

			component.meta.decompressedSize += pixelsSize;
		}
	}
	else if (child = cJSON_GetObjectItem(json, "diffuseFiles")) {
		int numFiles = cJSON_GetArraySize(child);
		int framesPerDir = numFiles / pMat->head.numDirections;
		if (numFiles % pMat->head.numDirections) {
			std::cout << "diffuseFiles % numDirections!" << std::endl;
		}
		else {
			SDL_Surface* firstSurf = nullptr;
			SDL_Surface* endSurf = nullptr;
			for (int i = 0; i < numFiles; i++) {
				cJSON* arrayNode = cJSON_GetArrayItem(child, i);
				const char* filePath = cJSON_ToString(arrayNode);
				SDL_Surface* fileSurf = IMG_Load(filePath);

				if (firstSurf == nullptr && fileSurf) {
					firstSurf = fileSurf;
					pMat->head.frameWidth = firstSurf->w;
					pMat->head.frameHeight = firstSurf->h;
					pMat->head.width = firstSurf->w * framesPerDir;
					pMat->head.height = firstSurf->h * pMat->head.numDirections;
					endSurf = SDL_CreateRGBSurface(0, pMat->head.width, pMat->head.height, 32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);

					SDL_Rect destination{ 0, 0, firstSurf->w, firstSurf->h };
					SDL_BlitSurface(firstSurf, nullptr, endSurf, &destination);
				}
				else if (fileSurf == nullptr) {
					std::cout << "Couldn't load " << filePath << std::endl;
					continue;
				}
				else {
					SDL_FreeSurface(fileSurf);
				}
			}
		}
		// Load the first file to get an idea of the size
		cJSON* arrayNode = cJSON_GetArrayItem(child, 0);
		if (arrayNode) {
			const char* firstFile = cJSON_ToString(arrayNode);
			SDL_Surface* firstSurf = IMG_Load(firstFile);
			if (firstSurf) {

			}
			else {
				std::cout << "Could not load " << firstFile << std::endl;
			}
			SDL_Surface* destSurface;
		}
	}

	child = cJSON_GetObjectItem(json, "normalFile");
	if (child) {
		const char* normalFile = cJSON_ToString(child);
		SDL_Surface* normalSurf = IMG_Load(normalFile);
		if (normalSurf == nullptr) {
			std::cout << "Couldn't load normalFile " << normalFile << std::endl;
			std::cout << "(Reason: " << IMG_GetError() << ")" << std::endl;
		}
		else {
			pMat->head.mapsPresent |= (1 << Maptype_Diffuse);
			pMat->head.normalWidth = normalSurf->w;
			pMat->head.normalHeight = normalSurf->h;
			pMat->head.frameWidth = normalSurf->w;
			pMat->head.frameHeight = normalSurf->h / pMat->head.numDirections;
			SDL_Surface* checkedSurf = DoOverbearingSurfaceCheck(normalSurf);

			size_t pixelsSize = sizeof(uint32_t) * pMat->head.normalWidth * pMat->head.normalHeight;
			pMat->normalPixels = (uint32_t*)malloc(pixelsSize);
			SDL_LockSurface(checkedSurf);
			memcpy(pMat->normalPixels, checkedSurf->pixels, pixelsSize);
			SDL_UnlockSurface(checkedSurf);

			SDL_FreeSurface(checkedSurf);
			SDL_FreeSurface(normalSurf);

			component.meta.decompressedSize += pixelsSize;
		}
	}
	else if (child = cJSON_GetObjectItem(json, "normalFiles")) {

	}

	child = cJSON_GetObjectItem(json, "depthFile");
	if (child) {
		std::cout << "depthFile not yet supported" << std::endl;
		//const char* depthFile = cJSON_ToString(child);
		//SDL_Surface* depthSurf = IMG_Load(depthFile);
	}
	else if (child = cJSON_GetObjectItem(json, "depthFiles")) {
		std::cout << "depthFiles not yet supported" << std::endl;
	}
	*/
}

// Processes a single image component from an asset schema file
void ProcessImageComponent(AssetComponent& component, cJSON* json) {
	cJSON* child = nullptr;

	child = cJSON_GetObjectItem(json, "file");
	if (child) {
		const char* imgFile = cJSON_ToString(child);
		SDL_Surface* imgSurf = IMG_Load(imgFile);
		ComponentImage* img = (ComponentImage*)malloc(sizeof(ComponentImage));
		memset(img, 0, sizeof(ComponentImage));
		component.data.imageComponent = img;

		if (imgSurf == nullptr) {
			std::cout << "Couldn't load image file: " << imgFile << std::endl;
			std::cout << "(Reason: " << IMG_GetError() << ")" << std::endl;
			return;
		}
		SDL_Surface* checkedSurf = DoOverbearingSurfaceCheck(imgSurf);
		img->head.width = checkedSurf->w;
		img->head.height = checkedSurf->h;
		size_t size = sizeof(uint32_t) * img->head.width * img->head.height;
		SDL_LockSurface(checkedSurf);
		memcpy(img->pixels, checkedSurf->pixels, size);
		SDL_UnlockSurface(checkedSurf);
		SDL_FreeSurface(imgSurf);
		SDL_FreeSurface(checkedSurf);

		component.meta.decompressedSize = size;
	}
}

// Processes a single font component from an asset schema file
void ProcessFontComponent(AssetComponent& component, cJSON* json) {
	cJSON* child = nullptr;
	size_t fontSize = sizeof(ComponentFont);

	component.data.fontComponent = (ComponentFont*)malloc(fontSize);
	memset(component.data.fontComponent, 0, fontSize);

	ComponentFont* pFont = component.data.fontComponent;
	pFont->head.pointSize = 12;
	
	child = cJSON_GetObjectItem(json, "file");
	if (child) {
		const char* filePath = cJSON_ToString(child);
		ReadFileForComponent((void**)&pFont->fontData, component.meta.decompressedSize, filePath);
	}

	child = cJSON_GetObjectItem(json, "face");
	pFont->head.fontFace = cJSON_ToIntegerOpt(child, 0);

	child = cJSON_GetObjectItem(json, "style");
	if (child) {
		int numElements = cJSON_GetArraySize(child);
		for (int i = 0; i < numElements; i++) {
			cJSON* elem = cJSON_GetArrayItem(child, i);
			const char* elemName = cJSON_ToString(elem);
			if (!elemName) {
				continue;
			}

			if (!stricmp(elemName, "bold")) {
				pFont->head.style |= TTF_STYLE_BOLD;
			}
			else if (!stricmp(elemName, "italic")) {
				pFont->head.style |= TTF_STYLE_ITALIC;
			}
			else if (!stricmp(elemName, "underline")) {
				pFont->head.style |= TTF_STYLE_UNDERLINE;
			}
			else if (!stricmp(elemName, "strikethrough")) {
				pFont->head.style |= TTF_STYLE_STRIKETHROUGH;
			}
			else if (!stricmp(elemName, "normal")) {
				pFont->head.style |= TTF_STYLE_NORMAL;
			}
		}
	}

	child = cJSON_GetObjectItem(json, "size");
	pFont->head.pointSize = cJSON_ToIntegerOpt(child, 12);

	component.meta.decompressedSize += fontSize;
}

// Processes a single level component from an asset schema file
void ProcessLevelComponent(AssetComponent& component, cJSON* json) {
	cJSON* child = nullptr;

	// TODO
}

// Processes a composition component from an asset schema file
void ProcessCompositionComponent(AssetComponent& component, cJSON* json) {
	cJSON* child = nullptr;

	// TODO
}

// Processes a single tile component from an asset schema file
void ProcessTileComponent(AssetComponent& component, cJSON* json) {
	cJSON* child = nullptr;

	size_t tileSize = sizeof(ComponentTile);
	component.data.tileComponent = (ComponentTile*)malloc(tileSize);
	component.meta.decompressedSize = tileSize;
	ComponentTile* pTile = component.data.tileComponent;

	child = cJSON_GetObjectItem(json, "walkmask");
	pTile->walkmask = cJSON_ToIntegerOpt(child, 0);

	child = cJSON_GetObjectItem(json, "jumpmask");
	pTile->jumpmask = cJSON_ToIntegerOpt(child, 0);

	child = cJSON_GetObjectItem(json, "shotmask");
	pTile->shotmask = cJSON_ToIntegerOpt(child, 0);

	child = cJSON_GetObjectItem(json, "lightmask");
	pTile->lightmask = cJSON_ToIntegerOpt(child, 0);

	child = cJSON_GetObjectItem(json, "warpmask");
	pTile->warpmask = cJSON_ToIntegerOpt(child, 0);

	child = cJSON_GetObjectItem(json, "becomeTransparent");
	pTile->becomeTransparent = cJSON_ToBooleanOpt(child, 0);

	child = cJSON_GetObjectItem(json, "autoTransX");
	pTile->autoTransX = cJSON_ToIntegerOpt(child, 0);

	child = cJSON_GetObjectItem(json, "autoTransY");
	pTile->autoTransY = cJSON_ToIntegerOpt(child, 0);

	child = cJSON_GetObjectItem(json, "autoTransW");
	pTile->autoTransW = cJSON_ToIntegerOpt(child, 0);

	child = cJSON_GetObjectItem(json, "autoTransH");
	pTile->autoTransH = cJSON_ToIntegerOpt(child, 0);

	child = cJSON_GetObjectItem(json, "animated");
	pTile->animated = cJSON_ToBooleanOpt(child, 0);

	child = cJSON_GetObjectItem(json, "frameNum");
	pTile->frameNum = cJSON_ToIntegerOpt(child, 0);

	child = cJSON_GetObjectItem(json, "transFrameNum");
	pTile->transFrameNum = cJSON_ToIntegerOpt(child, 0);

	child = cJSON_GetObjectItem(json, "material");
	strcpy(pTile->materialName, cJSON_ToStringOpt(child, ""));

	child = cJSON_GetObjectItem(json, "transMaterial");
	strcpy(pTile->transMaterialName, cJSON_ToStringOpt(child, ""));

	child = cJSON_GetObjectItem(json, "depthscoreOffset");
	pTile->depthscoreOffset = cJSON_ToNumberOpt(child, 0);
}

// Processes a single asset schema .json file
void ProcessAssetSchemaFile(const AssetToolArguments& args, const char* assetFile, const char* assetSchema) {
	char szSchemaText[MAX_ASSETFILE_LEN];
	char szErrorText[MAX_JSON_ERRORLEN];
	std::vector<AssetComponent> vDecompressedAssets;

	FILE* fp = fopen(assetSchema, "rb+");
	if (fp == nullptr) {
		std::cout << "Failed to open file " << assetSchema << std::endl;
		return;
	}
	size_t numRead = fread(szSchemaText, sizeof(char), MAX_ASSETFILE_LEN, fp);
	fclose(fp);
	if (numRead == 0) {
		std::cout << "Failed to read file " << assetSchema << std::endl;
		return;
	}

	cJSON* rootNode = cJSON_ParsePooled(szSchemaText, szErrorText, MAX_JSON_ERRORLEN);
	if (rootNode == nullptr) {
		std::cout << "Failed to parse schema file " << assetSchema << " (" << szErrorText << ")" << std::endl;
		return;
	}

	cJSON* child;
	const char* assetName;
	const char* author;
	const char* contentGroup;
	child = cJSON_GetObjectItem(rootNode, "assetName");
	assetName = cJSON_ToStringOpt(child, "");

	child = cJSON_GetObjectItem(rootNode, "author");
	author = cJSON_ToStringOpt(child, "");

	child = cJSON_GetObjectItem(rootNode, "contentGroup");
	contentGroup = cJSON_ToStringOpt(child, "");

	AssetFile* pAsset = new AssetFile(assetName, author, contentGroup);

	child = cJSON_GetObjectItem(rootNode, "components");
	if (child) {
		for (cJSON* item = cJSON_GetFirstItem(child); item; item = cJSON_GetNextItem(item)) {
			AssetComponent component{ { { 0 }, Asset_Undefined, 0, 0 } };
			const char* componentName = cJSON_GetItemKey(item);

			strcpy(component.meta.componentName, componentName);
			cJSON* subchild = cJSON_GetObjectItem(item, "componentType");
			if (subchild) {
				const char* componentType = cJSON_ToString(subchild);
				ComponentType t = ComponentTypeFromString(componentType);
				component.meta.componentType = t;
				switch (t) {
					case Asset_Undefined:
						component.meta.componentVersion = 0;
						ProcessUndefinedComponent(component, item);
						break;
					case Asset_Data:
						component.meta.componentVersion = COMP_DATA_VERSION;
						ProcessDataComponent(component, item);
						break;
					case Asset_Material:
						component.meta.componentVersion = COMP_MATERIAL_VERSION;
						ProcessMaterialComponent(component, item);
						break;
					case Asset_Image:
						component.meta.componentVersion = COMP_IMAGE_VERSION;
						ProcessImageComponent(component, item);
						break;
					case Asset_Font:
						component.meta.componentVersion = COMP_FONT_VERSION;
						ProcessFontComponent(component, item);
						break;
					case Asset_Level:
						component.meta.componentVersion = COMP_LEVEL_VERSION;
						ProcessLevelComponent(component, item);
						break;
					case Asset_Composition:
						component.meta.componentVersion = COMP_ANIM_VERSION;
						ProcessCompositionComponent(component, item);
						break;
					case Asset_Tile:
						component.meta.componentVersion = COMP_TILE_VERSION;
						ProcessTileComponent(component, item);
						break;
				}
				pAsset->decompressedAssets.push_back(component);
			}
		}

		std::cout << "Parsed " << pAsset->decompressedAssets.size() << " components." << std::endl;
		pAsset->asset.head.numberComponents = pAsset->decompressedAssets.size();
	}

	char outPath[MAX_OUTFILE_PATH]{0};
	if (args.pfOutputDir.bFlagPresent) {
		sprintf(outPath, "%s/%s.asset", args.pfOutputDir.szFlagText, assetFile);
	}
	else {
		sprintf(outPath, "%s.asset", assetFile);
	}

	pAsset->SaveFile(outPath);
	delete pAsset;
}

// Processes an entire project .json file
void ProcessProjectFile(const AssetToolArguments& args) {
	char szProjectText[MAX_PROJECTFILE_LEN];
	char szErrorText[MAX_JSON_ERRORLEN];

	// Open the project file
	const char* szProjectPath = args.pfProjectFile.szFlagText;
	FILE* fp = fopen(szProjectPath, "rb+");
	if (fp == nullptr) {
		std::cout << "Couldn't open project file: " << szProjectPath << std::endl;
		return;
	}

	// Read the project file and run it through the cJSON parser.
	size_t numRead = fread(szProjectText, sizeof(char), MAX_PROJECTFILE_LEN, fp);
	if (numRead == 0) {
		std::cout << "Couldn't read text from project file: " << szProjectPath << std::endl;
		fclose(fp);
		return;
	}
	fclose(fp);

	cJSON* rootNode = cJSON_ParsePooled(szProjectText, szErrorText, MAX_JSON_ERRORLEN);
	if (rootNode == nullptr) {
		std::cout << "Couldn't parse text in project file " << szProjectPath << " (" << szErrorText << ")" << std::endl;
		return;
	}

	std::cout << "Project file parsed successfully" << std::endl;
	cJSON* filesArray = cJSON_GetObjectItem(rootNode, "assets");
	int numParsed = 0;
	for (cJSON* item = cJSON_GetFirstItem(filesArray); item; item = cJSON_GetNextItem(item)) {
		const char* itemKey = cJSON_GetItemKey(item);
		const char* itemVal = cJSON_ToString(item);
		std::cout << itemKey << ".asset: " << itemVal << std::endl;
		ProcessAssetSchemaFile(args, itemKey, itemVal);
		numParsed++;
	}
	std::cout << "Parsed " << numParsed << " asset files in project" << std::endl;
}