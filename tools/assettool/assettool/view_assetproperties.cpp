#include "assettool.h"

void AssetPropertiesView() {
	// Property editor (only shows up for a few types of component, or when we have the asset selected)
	ImVec2 assetProperties(400, 500);
	ImGui::Begin("Asset Properties", &assetPropertiesVisible, assetProperties, -1.0f, ImGuiWindowFlags_HorizontalScrollbar);
	ImGui::End();
}