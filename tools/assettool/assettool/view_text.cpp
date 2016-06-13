#include "assettool.h"

extern int currentSelectedComponent;
extern bool componentSelected;

void TextView() {
	// Text editor (only shows up for data files)
	// TODO: Make this an actual editor and not just a viewer
	if (currentFile == nullptr) {
		return;
	}
	if (componentSelected == false) {
		return;
	}

	ImVec2 textStart(300, 400);
	ImGui::Begin("Text Preview", &textViewVisible, textStart, -1.0f, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_HorizontalScrollbar);
	ImGui::BeginMenuBar();
	/*
	if (ImGui::BeginMenu("File")) {
		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("Edit")) {
		ImGui::EndMenu();
	}
	*/
	ImGui::EndMenuBar();

	AssetComponent* pComponent = &currentFile->decompressedAssets[currentSelectedComponent];
	switch (pComponent->meta.componentType) {
		case Asset_Undefined:
			if (pComponent->data.undefinedComponent) {
				ImGui::TextWrapped((const char*)pComponent->data.undefinedComponent);
			}
			break;
		case Asset_Data:
			if (pComponent->data.dataComponent) {
				ImGui::TextWrapped((const char*)pComponent->data.dataComponent);
			}
			break;
		case Asset_Image:
			if (pComponent->data.imageComponent->pixels) {
				ImGui::TextWrapped((const char*)pComponent->data.imageComponent->pixels);
			}
			break;
		case Asset_Font:
			if (pComponent->data.fontComponent->fontData) {
				ImGui::TextWrapped((const char*)pComponent->data.fontComponent->fontData);
			}
			break;
		default:
			break;
	}
	ImGui::End();
}