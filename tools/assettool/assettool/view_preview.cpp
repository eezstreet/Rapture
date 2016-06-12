#include "assettool.h"

static SDL_Surface* currentPreview = nullptr;
ImVec2 previewImage(0, 0);
GLuint currentPreviewTexture = 0;

void InitPreviewMenu() {
	glGenTextures(1, &currentPreviewTexture);
}

void PreView() {
	// Image viewer (shows up for materials and images)
	ImVec2 previewStart(200, 200);
	ImGui::Begin("Preview", &previewVisible, previewStart, -1.0f, ImGuiWindowFlags_HorizontalScrollbar);
	if (currentPreview != nullptr) {
		ImGui::Image((ImTextureID)currentPreviewTexture, previewImage);
	}
	ImGui::End();
}

void AlterPreviewImage(SDL_Surface* pImage) {
	if (pImage == nullptr) {
		return;
	}

	SDL_LockSurface(pImage);
	glBindTexture(GL_TEXTURE_2D, currentPreviewTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, pImage->w, pImage->h, 0, GL_ABGR_EXT, GL_UNSIGNED_BYTE, pImage->pixels);
	SDL_UnlockSurface(pImage);

	currentPreview = pImage;
	previewImage.x = pImage->w;
	previewImage.y = pImage->h;

	// It's safe to free the surface after this function, because we don't actually use the contents of the surface
}

void ResetPreviewImage() {
	currentPreview = nullptr;
	previewImage.x = previewImage.y = 0;
}