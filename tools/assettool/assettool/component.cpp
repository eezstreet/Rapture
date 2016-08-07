#include "assettool.h"
#include <SDL_ttf.h>
#include <string>
#include <fstream>

#pragma warning(disable:4996)

const char* ComponentTypeToString(ComponentType t) {
	switch (t) {
		default:
		case Asset_Undefined:
			return "Unspecified Type";
		case Asset_Data:
			return "Raw";
		case Asset_Composition:
			return "Composition";
		case Asset_Font:
			return "Font";
		case Asset_Image:
			return "Image";
		case Asset_Level:
			return "Preset";
		case Asset_Material:
			return "Material";
		case Asset_Tile:
			return "Tile";
	}
}

ComponentType ComponentTypeFromString(const char* str) {
	if (!stricmp(str, "Asset_Data") || !stricmp(str, "Raw") || !stricmp(str, "Data")) {
		return Asset_Data;
	}
	else if (!stricmp(str, "Asset_Composition") || !stricmp(str, "Composition")) {
		return Asset_Composition;
	}
	else if (!stricmp(str, "Asset_Font") || !stricmp(str, "Font")) {
		return Asset_Font;
	}
	else if (!stricmp(str, "Asset_Image") || !stricmp(str, "Image")) {
		return Asset_Image;
	}
	else if (!stricmp(str, "Asset_Level") || !stricmp(str, "Level") || !stricmp(str, "Preset")) {
		return Asset_Level;
	}
	else if (!stricmp(str, "Asset_Material") || !stricmp(str, "Material")) {
		return Asset_Material;
	}
	else if (!stricmp(str, "Asset_Tile") || !stricmp(str, "Tile")) {
		return Asset_Tile;
	}
	return Asset_Undefined;
}

bool ComponentChanged(ComponentData* pPrevComponent, ComponentData* pNewComponent) {
	return strcmp(pPrevComponent->head.mime, pNewComponent->head.mime);
}

bool ComponentChanged(ComponentMaterial* pPrevComponent, ComponentMaterial* pNewComponent) {
	return memcmp(&pPrevComponent->head, &pNewComponent->head, sizeof(ComponentMaterial::MaterialHeader));
}

bool ComponentChanged(ComponentLevel* pPrevComponent, ComponentLevel* pNewComponent) {
	return false; // TODO
}

bool ComponentChanged(ComponentComp* pPrevComponent, ComponentComp* pNewComponent) {
	return false; // TODO
}

bool ComponentChanged(ComponentTile* pPrevComponent, ComponentTile* pNewComponent) {
	return memcmp(pPrevComponent, pNewComponent, sizeof(ComponentTile));
}

bool ComponentChanged(ComponentFont* pPrevComponent, ComponentFont* pNewComponent) {
	return memcmp(&pPrevComponent->head, &pNewComponent->head, sizeof(ComponentFont::FontHeader));
}

static void DrawNumberedCheckbox(int number, uint32_t* field) {
	std::string s = std::to_string(number + 1);
	ImGui::CheckboxFlags(s.c_str(), (unsigned int*)field, (1 << number));
}

static void DrawUndefinedComponent(void** component, size_t* decompressedSize) {
	if (ImGui::Button("Import Data")) {
		const char* path = OpenFileDialog("All Files\0*.*", 0);
		if (path[0] == '\0') {
			return;	// User canceled dialog box
		}

		std::ifstream infile(path, std::ios::binary);
		if (infile.bad()) {
			DisplayMessageBox("Import Error", "Couldn't import file (is it in use? no permission?", MESSAGEBOX_ERROR);
			infile.close();
			return;
		}
		std::streampos fileSize = infile.tellg();
		infile.seekg(0, std::ios::end);
		fileSize = infile.tellg() - fileSize;
		infile.clear();
		infile.seekg(0, std::ios::beg);

		*decompressedSize = fileSize;
		if (*component) {
			free(*component);
		}
		*component = malloc(*decompressedSize);
		infile.read((char*)*component, *decompressedSize);
		infile.close();

		fileChanged = true;
	}
	ImGui::SameLine();
	if (ImGui::Button("Export Data")) {
		if (*component == nullptr || *decompressedSize <= 0) {
			DisplayMessageBox("No Data", "There's no data to export", MESSAGEBOX_INFO);
			return;
		}

		const char* path = SaveAsDialog("All Files\0*.*", "");
		if (path[0] == '\0') {
			return;
		}

		std::ofstream outfile(path, std::ios::binary);
		if (outfile.bad()) {
			DisplayMessageBox("Export Error", "Couldn't export the data. Try running as administrator?", MESSAGEBOX_ERROR);
			return;
		}

		outfile.write((char*)*component, *decompressedSize);
		outfile.close();
	}
}

static void DrawDataComponent(ComponentData* pDataComponent, size_t* decompressedSize) {
	ImGui::TextWrapped("Import a file to auto-guess MIME type");
	if (ImGui::Button("Import Data")) {
		const char* path = OpenFileDialog("All Files\0*.*", 0);
		if (path[0] == '\0') {
			return;	// User canceled dialog box
		}

		std::ifstream infile(path, std::ios::binary);
		if (infile.bad()) {
			DisplayMessageBox("Import Error", "Couldn't import file (is it in use? no permission?", MESSAGEBOX_ERROR);
			infile.close();
			return;
		}
		std::streampos fileSize = infile.tellg();
		infile.seekg(0, std::ios::end);
		fileSize = infile.tellg() - fileSize;
		infile.clear();
		infile.seekg(0, std::ios::beg);

		*decompressedSize = fileSize;
		if (pDataComponent->data) {
			free(pDataComponent->data);
		}
		pDataComponent->data = (char*)malloc(*decompressedSize);
		infile.read(pDataComponent->data, *decompressedSize);
		infile.close();

		// Try to guess the mime type
		std::string stlpath(path);
		std::string ext = stlpath.substr(stlpath.find_last_of('.'));
		char* mime = pDataComponent->head.mime;
		TryGuessMimeType(mime, ext.c_str());

		fileChanged = true;
	}
	ImGui::SameLine();
	if (ImGui::Button("Export Data")) {
		if (!pDataComponent->data || *decompressedSize <= 0) {
			DisplayMessageBox("No Data", "There's no data to export", MESSAGEBOX_INFO);
			return;
		}

		const char* path = SaveAsDialog("All Files\0*.*", "");
		if (path[0] == '\0') {
			return;
		}

		std::ofstream outfile(path, std::ios::binary);
		if (outfile.bad()) {
			DisplayMessageBox("Export Error", "Couldn't export the data. Try running as administrator?", MESSAGEBOX_ERROR);
			return;
		}

		outfile.write(pDataComponent->data, *decompressedSize);
		outfile.close();
	}

	ImGui::InputText("MIME Type", pDataComponent->head.mime, sizeof(pDataComponent->head.mime));
}

char textureBuffer[32967];
static void DrawMaterialComponent(ComponentMaterial* pMaterialComponent) {
	ImGui::TextWrapped("Set number of directions before adding frames. For instance, if you have 8 directions and select 8 files, it will add 1 frame to each direction.");

	ImGui::CheckboxFlags("Use Diffuse Map?", (unsigned int*)&pMaterialComponent->head.mapsPresent, (1 << Maptype_Diffuse));
	ImGui::CheckboxFlags("Use Depth Map?", (unsigned int*)&pMaterialComponent->head.mapsPresent, (1 << Maptype_Depth));
	ImGui::CheckboxFlags("Use Normal Map?", (unsigned int*)&pMaterialComponent->head.mapsPresent, (1 << Maptype_Normal));

	if (pMaterialComponent->head.mapsPresent & (1 << Maptype_Diffuse)) {
		if (ImGui::TreeNode("Diffuse Map Options")) {
			if (ImGui::Button("Add Frames")) {
				ImportRGBASprite(&pMaterialComponent->diffusePixels, pMaterialComponent->head.numDirections,
					&pMaterialComponent->head.frameWidth, &pMaterialComponent->head.frameHeight,
					&pMaterialComponent->head.width, &pMaterialComponent->head.height);
				fileChanged = true;
			}
			ImGui::SameLine();
			if (ImGui::Button("Preview")) {
				if (pMaterialComponent->diffusePixels) {
					// Create the texture
					SDL_Surface* pDiffuse = SDL_CreateRGBSurfaceFrom(pMaterialComponent->diffusePixels,
						pMaterialComponent->head.width, pMaterialComponent->head.height, 32,
						pMaterialComponent->head.width * sizeof(*pMaterialComponent->diffusePixels),
						0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
					AlterPreviewImage(pDiffuse);
					SDL_FreeSurface(pDiffuse);

					// Display the preview menu
					previewVisible = true;
				}
				else {
					DisplayMessageBox("No pixels", "Cannot preview a blank diffuse map!", MESSAGEBOX_INFO);
				}
			}
			ImGui::SameLine();
			if (ImGui::Button("Export")) {
				ExportRGBASprite(pMaterialComponent->diffusePixels, pMaterialComponent->head.width, pMaterialComponent->head.height);
			}
			ImGui::SameLine();
			if (ImGui::Button("Reset")) {
				previewVisible = false;
				if (pMaterialComponent->diffusePixels != nullptr) {
					free(pMaterialComponent->diffusePixels);
					pMaterialComponent->diffusePixels = nullptr;
					pMaterialComponent->head.width = pMaterialComponent->head.height = 0;
					fileChanged = true;
				}
			}
			ImGui::Text("Width: %i pixels / Height: %i pixels", pMaterialComponent->head.width, pMaterialComponent->head.height);
			ImGui::TreePop();
		}
	}

	if (pMaterialComponent->head.mapsPresent & (1 << Maptype_Depth)) {
		if (ImGui::TreeNode("Depth Map Options")) {
			if (ImGui::Button("Add Frames")) {
				ImportMonochromeSprite(&pMaterialComponent->depthPixels, pMaterialComponent->head.numDirections,
					&pMaterialComponent->head.frameWidth, &pMaterialComponent->head.frameHeight,
					&pMaterialComponent->head.depthWidth, &pMaterialComponent->head.depthHeight);
				fileChanged = true;
			}
			ImGui::SameLine();
			if (ImGui::Button("Preview")) {
				if (pMaterialComponent->depthPixels) {

				}
				else {
					DisplayMessageBox("No pixels", "Cannot display a blank depth map!", MESSAGEBOX_INFO);
				}
			}
			ImGui::SameLine();
			if (ImGui::Button("Export")) {
				ExportMonochromeSprite(pMaterialComponent->depthPixels, pMaterialComponent->head.depthWidth, pMaterialComponent->head.depthHeight);
			}
			ImGui::SameLine();
			if (ImGui::Button("Reset")) {
				previewVisible = false;
				if (pMaterialComponent->depthPixels != nullptr) {
					free(pMaterialComponent->depthPixels);
					pMaterialComponent->depthPixels = nullptr;
					pMaterialComponent->head.depthWidth = pMaterialComponent->head.depthHeight;
					fileChanged = true;
				}
			}
			ImGui::Text("Width: %i pixels / Height: %i pixels", pMaterialComponent->head.depthWidth, pMaterialComponent->head.depthHeight);
			ImGui::TreePop();
		}
	}

	if (pMaterialComponent->head.mapsPresent & (1 << Maptype_Normal)) {
		if (ImGui::TreeNode("Normal Map Options")) {
			if (ImGui::Button("Add Frames")) {
				ImportRGBASprite(&pMaterialComponent->normalPixels, pMaterialComponent->head.numDirections,
					&pMaterialComponent->head.frameWidth, &pMaterialComponent->head.frameHeight,
					&pMaterialComponent->head.normalWidth, &pMaterialComponent->head.normalHeight);
				fileChanged = true;
			}
			ImGui::SameLine();
			if (ImGui::Button("Preview")) {
				if (pMaterialComponent->normalPixels) {
					// Create the texture
					SDL_Surface* pDiffuse = SDL_CreateRGBSurfaceFrom(pMaterialComponent->normalPixels,
						pMaterialComponent->head.normalWidth, pMaterialComponent->head.normalHeight, 32,
						pMaterialComponent->head.normalWidth * sizeof(*pMaterialComponent->normalPixels),
						0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
					AlterPreviewImage(pDiffuse);
					SDL_FreeSurface(pDiffuse);

					// Display the preview menu
					previewVisible = true;
				}
				else {
					DisplayMessageBox("No pixels", "Cannot display a blank normal map!", MESSAGEBOX_INFO);
				}
			}
			ImGui::SameLine();
			if (ImGui::Button("Export")) {
				ExportRGBASprite(pMaterialComponent->normalPixels, pMaterialComponent->head.normalWidth, pMaterialComponent->head.normalHeight);
			}
			ImGui::SameLine();
			if (ImGui::Button("Reset")) {
				previewVisible = false;
				if (pMaterialComponent->normalPixels != nullptr) {
					fileChanged = true;
					free(pMaterialComponent->normalPixels);
					pMaterialComponent->normalPixels = nullptr;
					pMaterialComponent->head.normalWidth = pMaterialComponent->head.normalHeight = 0;
				}
			}
			ImGui::Text("Width: %i pixels / Height: %i pixels", pMaterialComponent->head.normalWidth, pMaterialComponent->head.normalHeight);
			ImGui::TreePop();
		}
	}

	ImGui::InputInt("Render X offset (pixels)", (int*)&pMaterialComponent->head.xoffset);
	ImGui::InputInt("Render Y offset (pixels)", (int*)&pMaterialComponent->head.yoffset);
	ImGui::InputInt("Number of directions", (int*)&pMaterialComponent->head.numDirections);
	ImGui::InputInt("Render FPS", (int*)&pMaterialComponent->head.fps);
	ImGui::Text("Frame Width: %i pixels / Frame Height: %i pixels", pMaterialComponent->head.frameWidth, pMaterialComponent->head.frameHeight);
}

static void DrawImageComponent(ComponentImage* pImageComponent) {
	ImGui::Text("Width: %i pixels", pImageComponent->head.width);
	ImGui::Text("Height: %i pixels", pImageComponent->head.height);
	if (ImGui::Button("Import")) {
		ImportImage(&pImageComponent->pixels, &pImageComponent->head.width, &pImageComponent->head.height);
		fileChanged = true;
	}
	ImGui::SameLine();
	if (ImGui::Button("Preview")) {
		if (pImageComponent->pixels) {
			SDL_Surface* pImage = SDL_CreateRGBSurfaceFrom(pImageComponent->pixels, pImageComponent->head.width, pImageComponent->head.height, 32,
				pImageComponent->head.width * sizeof(uint32_t), 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
			AlterPreviewImage(pImage);
			SDL_FreeSurface(pImage);
			previewVisible = true;
		}
		else {
			DisplayMessageBox("No pixels", "Cannot display a blank image!", MESSAGEBOX_INFO);
		}
	}
}

static void DrawLevelComponent(ComponentLevel* pLevelComponent) {
	ImGui::Text("Width: %i", pLevelComponent->head.width);
	ImGui::Text("Height: %i", pLevelComponent->head.height);
	ImGui::Text("Tile Count: %i", pLevelComponent->head.numTiles);
	ImGui::Text("Ent Count: %i", pLevelComponent->head.numEntities);
	// TODO Shouldn't we have an import button here...?
}

static void DrawCompComponent(ComponentComp* pAnimComponent) {
	// TODO
}

static void DrawTileComponent(ComponentTile* pTileComponent) {
	if (ImGui::TreeNode("Material Options")) {
		ImGui::InputText("Material", pTileComponent->materialName, sizeof(pTileComponent->materialName));
		ImGui::InputFloat("Depth Score Offset", &pTileComponent->depthscoreOffset, 20.0f, 80.0f, 2);
		ImGui::InputInt("Starting Frame", (int*)&pTileComponent->frameNum);
		ImGui::Checkbox("Animated Tile", (bool*)&pTileComponent->animated);
		ImGui::TreePop();
	}
	ImGui::Checkbox("Become transparent", (bool*)&pTileComponent->becomeTransparent);
	if (pTileComponent->becomeTransparent) {
		if (ImGui::TreeNode("Transparency Options")) {
			ImGui::InputText("Transparent Material", pTileComponent->transMaterialName, sizeof(pTileComponent->transMaterialName));
			ImGui::InputInt("Auto Transparency X (pixels)", (int*)&pTileComponent->autoTransX, 1, 25);
			ImGui::InputInt("Auto Transparency Y (pixels)", (int*)&pTileComponent->autoTransY, 1, 25);
			ImGui::InputInt("Auto Transparency W (pixels)", (int*)&pTileComponent->autoTransW, 1, 25);
			ImGui::InputInt("Auto Transparency H (pixels)", (int*)&pTileComponent->autoTransH, 1, 25);
			ImGui::InputInt("Starting Frame", (int*)&pTileComponent->transFrameNum);
			ImGui::TreePop();
		}
	}
	ImGui::Spacing();

	if (ImGui::TreeNode("Subtile Options")) {
		ImGui::TextWrapped("Each subtile corresponds to 1/25th of a tile. First subtile is the top corner, last subtile is bottom corner");

		ImGui::Text("Block Movement");
		for (int i = 1; i < 26; i++) {
			DrawNumberedCheckbox(i - 1, &pTileComponent->walkmask);
			if (i % 5) {
				ImGui::SameLine();
			}
		}
		ImGui::Spacing();

		ImGui::Text("Block Jump");
		for (int i = 1; i < 26; i++) {
			DrawNumberedCheckbox(i - 1, &pTileComponent->jumpmask);
			if (i % 5) {
				ImGui::SameLine();
			}
		}
		ImGui::Spacing();

		ImGui::Text("Block Shots");
		for (int i = 1; i < 26; i++) {
			DrawNumberedCheckbox(i - 1, &pTileComponent->shotmask);
			if (i % 5) {
				ImGui::SameLine();
			}
		}
		ImGui::Spacing();

		ImGui::Text("Block Light (SDL)");
		for (int i = 1; i < 26; i++) {
			DrawNumberedCheckbox(i - 1, &pTileComponent->lightmask);
			if (i % 5) {
				ImGui::SameLine();
			}
		}
		ImGui::Spacing();

		ImGui::Text("Block LOS");
		for (int i = 1; i < 26; i++) {
			DrawNumberedCheckbox(i - 1, &pTileComponent->vismask);
			if (i % 5) {
				ImGui::SameLine();
			}
		}
		ImGui::Spacing();

		ImGui::Text("Interaction");
		for (int i = 1; i < 26; i++) {
			DrawNumberedCheckbox(i - 1, &pTileComponent->warpmask);
			if (i % 5) {
				ImGui::SameLine();
			}
		}
		ImGui::TreePop();
	}
}

void DrawFontData(ComponentFont* ptFontComponent, size_t* decompSize) {
	ImGui::InputInt("Point Size (ingame)", (int*)&ptFontComponent->head.pointSize);
	ImGui::CheckboxFlags("Bold", (unsigned int*)&ptFontComponent->head.style, TTF_STYLE_BOLD);
	ImGui::CheckboxFlags("Italic", (unsigned int*)&ptFontComponent->head.style, TTF_STYLE_ITALIC);
	ImGui::CheckboxFlags("Underline", (unsigned int*)&ptFontComponent->head.style, TTF_STYLE_UNDERLINE);
	ImGui::CheckboxFlags("Strikethrough", (unsigned int*)&ptFontComponent->head.style, TTF_STYLE_STRIKETHROUGH);
	ImGui::CheckboxFlags("Normal (?)", (unsigned int*)&ptFontComponent->head.style, TTF_STYLE_NORMAL);
	ImGui::InputInt("Font Face", (int*)&ptFontComponent->head.fontFace);

	ImGui::Spacing();
	if (ImGui::Button("Import TTF File")) {
		ImportFontFileTTF(&ptFontComponent->fontData, decompSize, ptFontComponent->head.pointSize, ptFontComponent->head.fontFace);
		*decompSize += sizeof(ptFontComponent->head);
	}
	if (ptFontComponent->fontData != nullptr) {
		ImGui::SameLine();
		if (ImGui::Button("Preview Font")) {
			uint64_t fontDataSize = *decompSize - sizeof(ptFontComponent->head);
			PreviewFont(ptFontComponent->fontData, fontDataSize, ptFontComponent->head.pointSize, ptFontComponent->head.style, ptFontComponent->head.fontFace);
		}
	}
}

void DrawComponentData(int currentSelectedComponent) {
	if (currentFile == nullptr) {
		return;
	}

	auto comp = currentFile->decompressedAssets[currentSelectedComponent];
	AssetComponent::AssetCompHead prevMeta = comp.meta;

	ImGui::BeginGroup();
	ImGui::InputText("Component Name", comp.meta.componentName, sizeof(comp.meta.componentName));
	ImGui::Text("Decompressed size: %i bytes", comp.meta.decompressedSize);
	ImGui::EndGroup();
	ImGui::Spacing();
	ImGui::BeginGroup();

	switch (comp.meta.componentType) {
		case Asset_Undefined:
			DrawUndefinedComponent(&comp.data.undefinedComponent, &comp.meta.decompressedSize);
			currentFile->decompressedAssets[currentSelectedComponent].data.undefinedComponent = comp.data.undefinedComponent;
			break;
		case Asset_Data:
			if (comp.data.dataComponent) {
				ComponentData prevComponent = *comp.data.dataComponent;
				DrawDataComponent(comp.data.dataComponent, &comp.meta.decompressedSize);
				currentFile->decompressedAssets[currentSelectedComponent].data.dataComponent = comp.data.dataComponent;
				ComponentData newComponent = *comp.data.dataComponent;
				if (ComponentChanged(&prevComponent, &newComponent)) {
					fileChanged = true;
				}
			}
			break;
		case Asset_Font:
			if (comp.data.fontComponent) {
				ComponentFont prevComponent = *comp.data.fontComponent;
				DrawFontData(comp.data.fontComponent, &comp.meta.decompressedSize);
				ComponentFont newComponent = *comp.data.fontComponent;
				if (ComponentChanged(&prevComponent, &newComponent)) {
					fileChanged = true;
				}
			}
			currentFile->decompressedAssets[currentSelectedComponent].data.fontComponent = comp.data.fontComponent;
			break;
		case Asset_Material:
			if (comp.data.materialComponent) {
				ComponentMaterial prevComponent = *comp.data.materialComponent;
				DrawMaterialComponent(comp.data.materialComponent);
				ComponentMaterial newComponent = *comp.data.materialComponent;
				if (ComponentChanged(&prevComponent, &newComponent)) {
					fileChanged = true;
				}
			}
			break;
		case Asset_Image:
			if (comp.data.imageComponent) {
				DrawImageComponent(comp.data.imageComponent);
			}
			break;
		case Asset_Level:
			if (comp.data.levelComponent) {
				ComponentLevel prevComponent = *comp.data.levelComponent;
				DrawLevelComponent(comp.data.levelComponent);
				ComponentLevel newComponent = *comp.data.levelComponent;
				if (ComponentChanged(&prevComponent, &newComponent)) {
					fileChanged = true;
				}
			}
			break;
		case Asset_Composition:
			if (comp.data.compComponent) {
				ComponentComp prevComponent = *comp.data.compComponent;
				DrawCompComponent(comp.data.compComponent);
				ComponentComp newComponent = *comp.data.compComponent;
				if (ComponentChanged(&prevComponent, &newComponent)) {
					fileChanged = true;
				}
			}
			break;
		case Asset_Tile:
			if (comp.data.tileComponent) {
				ComponentTile prevComponent = *comp.data.tileComponent;
				DrawTileComponent(comp.data.tileComponent);
				ComponentTile newComponent = *comp.data.tileComponent;
				if (ComponentChanged(&prevComponent, &newComponent)) {
					fileChanged = true;
				}
			}
			break;
	}
	// Sometimes this data doesn't carry over properly for some reason.
	currentFile->decompressedAssets[currentSelectedComponent].meta.decompressedSize = comp.meta.decompressedSize;
	strncpy(currentFile->decompressedAssets[currentSelectedComponent].meta.componentName, comp.meta.componentName, COMP_NAMELEN);
	ImGui::EndGroup();

	AssetComponent::AssetCompHead newMeta = comp.meta;
	if (memcmp(&prevMeta, &newMeta, sizeof(AssetComponent::AssetCompHead))) {
		fileChanged = true;
	}
}