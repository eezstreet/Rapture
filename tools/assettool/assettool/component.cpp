#include "assettool.h"
#include <SDL_ttf.h>
#include <string>

const char* ComponentTypeToString(ComponentType t) {
	switch (t) {
		default:
		case Asset_Undefined:
			return "Unknown Type";
		case Asset_Data:
			return "Data";
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

static void DrawNumberedCheckbox(int number, uint32_t* field) {
	std::string s = std::to_string(number + 1);
	ImGui::CheckboxFlags(s.c_str(), (unsigned int*)field, (1 << number));
}

static void DrawMaterialComponent(ComponentMaterial* pMaterialComponent) {
	ImGui::TextWrapped("Set number of directions !first!, check boxes for maps and select all frames in dialog");
	if (ImGui::CheckboxFlags("Use Diffuse Map?", (unsigned int*)&pMaterialComponent->head.mapsPresent, Maptype_Diffuse)) {

	}
	if (ImGui::CheckboxFlags("Use Depth Map?", (unsigned int*)&pMaterialComponent->head.mapsPresent, Maptype_Depth)) {

	}
	if (ImGui::CheckboxFlags("Use Normal Map?", (unsigned int*)&pMaterialComponent->head.mapsPresent, Maptype_Normal)) {

	}
	ImGui::InputInt("Render X offset (pixels)", (int*)&pMaterialComponent->head.xoffset);
	ImGui::InputInt("Render Y offset (pixels)", (int*)&pMaterialComponent->head.yoffset);
	ImGui::InputInt("Number of directions", (int*)&pMaterialComponent->head.numDirections);
	ImGui::InputInt("Render FPS", (int*)&pMaterialComponent->head.fps);
	ImGui::Text("Frame Width: %i pixels / Frame Height: %i pixels", pMaterialComponent->head.frameWidth, pMaterialComponent->head.frameHeight);
	ImGui::Text("Total Width: %i pixels / Total Height: %i pixels", pMaterialComponent->head.width, pMaterialComponent->head.height);
	if (ImGui::Button("Preview Diffuse Map")) {

	}
	ImGui::SameLine();
	if (ImGui::Button("Preview Depth Map")) {

	}
	ImGui::SameLine();
	if (ImGui::Button("Preview Normal Map")) {

	}
}

static void DrawImageComponent(ComponentImage* pImageComponent) {
	ImGui::Text("Width: %i pixels", pImageComponent->head.width);
	ImGui::Text("Height: %i pixels", pImageComponent->head.height);
	if (ImGui::Button("Preview")) {

	}
}

static void DrawLevelComponent(ComponentLevel* pLevelComponent) {
	ImGui::Text("Use Component > Import to import level created ingame");
	ImGui::Text("Width: %i", pLevelComponent->head.width);
	ImGui::Text("Height: %i", pLevelComponent->head.height);
	ImGui::Text("Tile Count: %i", pLevelComponent->head.numTiles);
	ImGui::Text("Ent Count: %i", pLevelComponent->head.numEntities);
}

static void DrawCompComponent(ComponentComp* pAnimComponent) {
	
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

static TTF_Font* selectedTTFFont = nullptr;
void SelectedFontComponent(int selectedComponent) {
	if (currentFile == nullptr) {
		return;
	}
	auto comp = currentFile->decompressedAssets[selectedComponent];
	if (comp.data.fontComponent == nullptr) {
		return;
	}

	if (comp.data.fontComponent->fontData == nullptr) {
		return;
	}

	ComponentFont* ptFont = comp.data.fontComponent;
	size_t sizeTTF = comp.meta.decompressedSize - sizeof(comp.data) - sizeof(*ptFont);
	selectedTTFFont = TTF_OpenFontIndexRW(SDL_RWFromMem(ptFont->fontData, sizeTTF), 0, ptFont->head.pointSize, ptFont->head.fontFace);
}

void DrawFontData(ComponentFont* ptFontComponent) {
	if (ptFontComponent->fontData != nullptr) {
	}

	ImGui::InputInt("Point Size (ingame)", (int*)&ptFontComponent->head.pointSize);
	ImGui::CheckboxFlags("Bold", (unsigned int*)&ptFontComponent->head.style, TTF_STYLE_BOLD);
	ImGui::CheckboxFlags("Italic", (unsigned int*)&ptFontComponent->head.style, TTF_STYLE_ITALIC);
	ImGui::CheckboxFlags("Underline", (unsigned int*)&ptFontComponent->head.style, TTF_STYLE_UNDERLINE);
	ImGui::CheckboxFlags("Strikethrough", (unsigned int*)&ptFontComponent->head.style, TTF_STYLE_STRIKETHROUGH);
	ImGui::CheckboxFlags("Normal (?)", (unsigned int*)&ptFontComponent->head.style, TTF_STYLE_NORMAL);
	ImGui::InputInt("Font Face", (int*)&ptFontComponent->head.fontFace);

	ImGui::Spacing();
	if (ImGui::Button("Import TTF File")) {

	}
	if (ptFontComponent->fontData != nullptr) {
		ImGui::SameLine();
		if (ImGui::Button("Preview Font")) {

		}
		ImGui::SameLine();
		if (ImGui::Button("Refresh Font Data")) {

		}
	}
}

void DrawComponentData(int currentSelectedComponent) {
	if (currentFile == nullptr) {
		return;
	}

	auto comp = currentFile->decompressedAssets[currentSelectedComponent];
	ImGui::BeginGroup();
	ImGui::InputText("Component Name", comp.meta.componentName, sizeof(comp.meta.componentName));
	ImGui::Text("Decompressed size: %i bytes", comp.meta.decompressedSize);
	ImGui::EndGroup();
	ImGui::Spacing();
	ImGui::BeginGroup();
	switch (comp.meta.componentType) {
		case Asset_Undefined:
		case Asset_Data:
			// Don't draw anything at all in this panel
			break;
		case Asset_Font:
			if (comp.data.fontComponent) {
				DrawFontData(comp.data.fontComponent);
			}
			break;
		case Asset_Material:
			if (comp.data.materialComponent) {
				DrawMaterialComponent(comp.data.materialComponent);
			}
			break;
		case Asset_Image:
			if (comp.data.imageComponent) {
				DrawImageComponent(comp.data.imageComponent);
			}
			break;
		case Asset_Level:
			if (comp.data.levelComponent) {
				DrawLevelComponent(comp.data.levelComponent);
			}
			break;
		case Asset_Composition:
			if (comp.data.compComponent) {
				DrawCompComponent(comp.data.compComponent);
			}
			break;
		case Asset_Tile:
			if (comp.data.tileComponent) {
				DrawTileComponent(comp.data.tileComponent);
			}
			break;
	}
	ImGui::EndGroup();
}