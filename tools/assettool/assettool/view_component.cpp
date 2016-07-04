#include "assettool.h"

#pragma warning (disable:4996)

int currentSelectedComponent = -1;
bool componentSelected = false;

static char componentNameBuffer[COMP_NAMELEN];
static int componentTypeSelection = 1;

bool *componentSelectedArray = nullptr;
int numComponentsAllocated = 0;

static const char* componentTypes[] = {
	ComponentTypeToString(Asset_Undefined),
	ComponentTypeToString(Asset_Data),
	ComponentTypeToString(Asset_Material),
	ComponentTypeToString(Asset_Image),
	ComponentTypeToString(Asset_Font),
	ComponentTypeToString(Asset_Level),
	ComponentTypeToString(Asset_Composition),
	ComponentTypeToString(Asset_Tile)
};

void ComponentView() {
	int i;
	// Component view (shows the asset file and the components contained as a tree view)
	bool createNewComponent = false;
	bool importFile = false;
	ImVec2 componentStart(300, 400);
	ImGui::Begin("Component View", &componentViewVisible, componentStart, -1.0f, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_HorizontalScrollbar);

	// Menu..
	ImGui::BeginMenuBar();
	if (ImGui::BeginMenu("Component")) {
		if (ImGui::MenuItem("New Component", "Ctrl+K", nullptr, fileOpen)) {
			createNewComponent = true;
		}
		if (ImGui::MenuItem("Delete Component", "Del", nullptr, fileOpen && componentSelected)) {
			currentFile->DeleteComponent(currentSelectedComponent);
			if (currentSelectedComponent == 0) {
				currentSelectedComponent = -1;
				componentSelected = false;
			}
			else {
				currentSelectedComponent--;
			}
		}
		ImGui::EndMenu();
	}
	ImGui::EndMenuBar();

	if (createNewComponent) {
		ImGui::OpenPopup("Create New Component");
	}
	if (ImGui::BeginPopupModal("Create New Component", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::InputText("Component Name", componentNameBuffer, COMP_NAMELEN);
		ImGui::ListBox("Component Type", &componentTypeSelection, componentTypes, Asset_Tile + 1, 5);
		if (ImGui::Button("CANCEL")) {
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("OK")) {
			currentFile->AddNewComponent((ComponentType)componentTypeSelection, componentNameBuffer);
			componentNameBuffer[0] = '\0';
			componentTypeSelection = Asset_Data;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	if (numComponentsAllocated != currentFile->asset.head.numberComponents) {
		if (componentSelectedArray != nullptr) {
			delete[] componentSelectedArray;
		}
		numComponentsAllocated = currentFile->asset.head.numberComponents;
		componentSelectedArray = new bool[numComponentsAllocated];
		for (i = 0; i < numComponentsAllocated; i++) {
			componentSelectedArray[i] = false;
		}
		if (componentSelected && currentSelectedComponent < numComponentsAllocated) {
			componentSelectedArray[currentSelectedComponent] = true;
		}
		else {
			componentSelected = false;
			currentSelectedComponent = -1;
		}
	}

	ImGui::Columns(2);
	componentSelected = false;
	currentSelectedComponent = -1;
	if (fileOpen) {
		if (ImGui::TreeNode(currentFile->asset.head.assetName)) {
			// Iterate through all of the components and draw a selectable for each one
			for (i = 0; i < currentFile->decompressedAssets.size(); i++) {
				char textBuffer[1024];
				auto it = currentFile->decompressedAssets[i];
				if (it.meta.componentType == Asset_Data && it.data.dataComponent != nullptr) {
					// List the MIME type in the component name
					ComponentData* dataComp = it.data.dataComponent;
					sprintf(textBuffer, "%s (%s - %s)", it.meta.componentName, ComponentTypeToString(Asset_Data), dataComp->head.mime);
				}
				else {
					sprintf(textBuffer, "%s (%s)", it.meta.componentName, ComponentTypeToString(it.meta.componentType));
				}
				if (ImGui::Selectable(textBuffer, &componentSelectedArray[i])) {
					// deselect all other ones
					for (int j = 0; j < numComponentsAllocated; j++) {
						if (i != j) {
							componentSelectedArray[j] = false;
						}
					}
				}
				if (componentSelectedArray[i]) {
					componentSelected = true;
					currentSelectedComponent = i;
				}
			}
			ImGui::TreePop();
		}
		else {
			componentSelected = false;
			currentSelectedComponent = -1;
		}
	}

	// Check and see if we've got a selected component
	ImGui::NextColumn();
	if (componentSelected) {
		char textBuffer[1024] = { 0 };
		sprintf(textBuffer, "%s component", ComponentTypeToString(currentFile->decompressedAssets[currentSelectedComponent].meta.componentType));
		ImGui::Text(textBuffer);
		DrawComponentData(currentSelectedComponent);
	}
	ImGui::End();
}

void FreeComponentView() {
	if (componentSelectedArray != nullptr) {
		delete[] componentSelectedArray;
	}
	componentSelectedArray = nullptr;
}