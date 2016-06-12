#include "assettool.h"

void AssetPropertiesView() {
	// Property editor (only shows up for a few types of component, or when we have the asset selected)
	ImVec2 assetProperties(400, 500);
	ImGui::Begin("Asset Properties", &assetPropertiesVisible, assetProperties, -1.0f, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_AlwaysAutoResize);
	if (currentFile == nullptr) {
		ImGui::End();
		return;
	}
	ImGui::InputText("Asset Name", currentFile->asset.head.assetName, sizeof(currentFile->asset.head.assetName));
	ImGui::NewLine();
	ImGui::InputText("Author", currentFile->asset.head.author, sizeof(currentFile->asset.head.author));
	ImGui::Text("Original Author: %s", currentFile->asset.head.originalAuth);
	ImGui::NewLine();
	ImGui::Text("Asset Version: %i", currentFile->asset.head.version);
	ImGui::Text("Content Group: %s", currentFile->asset.head.contentGroup);
	ImGui::NewLine();
	ImGui::Checkbox("Use Compression", (bool*)&currentFile->asset.head.compressionType);
	if (currentFile->asset.head.compressionType) {
		ImGui::SliderInt("Compression Level", (int*)&currentFile->asset.head.compressionLevel, 1, 9);
	}
	ImGui::End();
}