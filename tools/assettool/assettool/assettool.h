#pragma once
#include <../../common/RaptureAsset.h>
#include "../imgui/imgui.h"
#include <SDL.h>
#include <SDL_opengl.h>
#include <vector>

#define PROGRAM_NAME		"Rapture Asset Tool"

extern bool componentViewVisible;
extern bool textViewVisible;
extern bool previewVisible;
extern bool assetPropertiesVisible;

extern bool fileOpen;
extern bool fileChanged;

// view_component.cpp
void ComponentView();
void FreeComponentView();

// view_text.cpp
void TextView();

// view_preview.cpp
void PreView();

// view_assetproperties.cpp
void AssetPropertiesView();

// component.cpp
const char* ComponentTypeToString(ComponentType t);
void DrawComponentData(int currentSelectedComponent);

class AssetFile {
private:
	bool hasErrors;
public:
	RaptureAsset asset;
	std::vector<AssetComponent> decompressedAssets;

	void SaveFile(const char* destination);
	void AddNewComponent(ComponentType t, const char* name);
	void DeleteComponent(int componentNum);
	void ExportComponent(int componentNum);
	void ImportComponent(int componentNum);

	AssetFile(const char* name, const char* author, const char* dlc);
	AssetFile(const char* path);
	~AssetFile();

	bool HasErrors() { return hasErrors; }
};
extern AssetFile* currentFile;


// platform.cpp
typedef enum {
	MESSAGEBOX_INFO,
	MESSAGEBOX_WARNING,
	MESSAGEBOX_ERROR
};

#define OFFLAG_MULTISELECT	1

void DisplayMessageBox(const char* title, const char* msg, int type);
const char* OpenFileDialog(const char* filter, int flags);
const char* SaveAsDialog(const char* filter, const char* extension);