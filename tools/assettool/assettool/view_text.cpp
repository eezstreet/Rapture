#include "assettool.h"

void TextView() {
	// Text editor (only shows up for data files)
	ImVec2 textStart(300, 400);
	ImGui::Begin("Text Editor", &textViewVisible, textStart, -1.0f, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_HorizontalScrollbar);
	ImGui::BeginMenuBar();
	if (ImGui::BeginMenu("File")) {
		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("Edit")) {
		ImGui::EndMenu();
	}
	ImGui::EndMenuBar();
	ImGui::End();
}