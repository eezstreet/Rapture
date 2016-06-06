#include "assettool.h"

void PreView() {
	// Image viewer (shows up for materials and images)
	ImVec2 previewStart(200, 200);
	ImGui::Begin("Preview", &previewVisible, previewStart, -1.0f, ImGuiWindowFlags_HorizontalScrollbar);
	ImGui::End();
}