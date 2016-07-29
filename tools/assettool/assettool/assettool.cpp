#include <stdio.h>
#include "assettool.h"
#include "imgui_impl_sdl.h"
#include <SDL_ttf.h>
#include <SDL_image.h>

#pragma warning(disable:4996)

bool componentViewVisible = false;
bool textViewVisible = false;
bool previewVisible = false;
bool assetPropertiesVisible = false;

bool fileOpen = false;
bool fileChanged = false;
bool fileHasPath = false;

bool done = false;

AssetFile* currentFile = nullptr;

static char currentFileName[ASSET_NAMELEN] = { "" };

static char nameBuffer[ASSET_NAMELEN] = { "" };
static char authorBuffer[AUTHOR_NAMELEN] = { "" };

SDL_Window* window;

void UpdateWindowTitle() {
	char buffer[ASSET_NAMELEN * 2];

	if (fileOpen == false) {
		FreeComponentView();
		sprintf(buffer, "%s", PROGRAM_NAME);
	}
	else if (fileChanged) {
		sprintf(buffer, "%s - %s *", PROGRAM_NAME, currentFileName);
	}
	else {
		sprintf(buffer, "%s - %s", PROGRAM_NAME, currentFileName);
	}

	SDL_SetWindowTitle(window, buffer);
}

void FileModified() {
	fileChanged = true;
	UpdateWindowTitle();
}

static bool newAssetFile = false;
static bool openAssetFile = false;
static bool closeAssetFile = false;
static bool continueWithoutSaving = false;

const char* dlc_packages[] = {
	"FreeDLC",
	"BaseGame",
	"UserMod"
};
void MainView() {
	bool about = false;
	const char* file;

	ImGui::BeginMainMenuBar();
	if (ImGui::BeginMenu("File")) {
		if (ImGui::MenuItem("New Asset File", "Ctrl+N")) {
			// Create new asset file
			newAssetFile = true;
		}
		if (ImGui::MenuItem("Open", "Ctrl+O")) {
			// Open asset file
			openAssetFile = true;
		}
		if (ImGui::MenuItem("Save", "Ctrl+S", nullptr, fileOpen && fileChanged)) {
			if (!fileHasPath) {
				// Open the save as dialog box
				file = SaveAsDialog("All Files\0*.*\0Asset Files (.asset)\0*.ASSET\0", ".asset");
				strcpy(currentFileName, file);
				fileHasPath = true;
				fileChanged = false;
			}
			// Save the actual file
			currentFile->SaveFile(currentFileName);
			fileChanged = false;
		}
		if (ImGui::MenuItem("Save As", "Ctrl+Shift+S", nullptr, fileOpen)) {
			file = SaveAsDialog("All Files\0*.*\0Asset Files (.asset)\0*.asset\0", "asset");
			strcpy(currentFileName, file);
			fileHasPath = true;
			currentFile->SaveFile(currentFileName);
			fileChanged = false;
		}
		if (ImGui::MenuItem("Close File", "Ctrl+P", nullptr, fileOpen)) {
			closeAssetFile = true;
		}
		if (ImGui::MenuItem("Exit", "Alt+F4")) {
			done = true;
		}
		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("Window")) {
		if (ImGui::MenuItem("Toggle Component View", "Alt+C", nullptr, fileOpen)) {
			componentViewVisible = !componentViewVisible;
		}
		if (ImGui::MenuItem("Toggle Text Editor", "Alt+T", nullptr, fileOpen)) {
			textViewVisible = !textViewVisible;
		}
		if (ImGui::MenuItem("Toggle Preview", "Alt+P", nullptr, fileOpen)) {
			previewVisible = !previewVisible;
		}
		if (ImGui::MenuItem("Toggle Asset Properties View", "Alt+A", nullptr, fileOpen)) {
			assetPropertiesVisible = !assetPropertiesVisible;
		}
		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("Help")) {
		ImGui::MenuItem("About", nullptr, &about);
		ImGui::EndMenu();
	}

	ImGui::EndMainMenuBar();

	if (about) {
		ImGui::OpenPopup("About");
	}
	if (ImGui::BeginPopupModal("About")) {
		ImGui::Text("Rapture Asset Tool");
		ImGui::Text("Developed by Nicholas Whitlock (\"eezstreet\") for Rapture");
		ImGui::Text("Copyright (c) 2016 <whatever our company is called>");
		if (ImGui::Button("Close")) {
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	if (fileChanged) {
		if (newAssetFile || closeAssetFile || openAssetFile || done) {
			ImGui::OpenPopup("Unsaved File");
		}
	}
	else if (newAssetFile || closeAssetFile || openAssetFile || done) {
		continueWithoutSaving = true;
	}

	if (ImGui::BeginPopupModal("Unsaved File", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::Text("File has unsaved changes, continue?");
		if (ImGui::Button("Yes")) {
			continueWithoutSaving = true;
			ImGui::CloseCurrentPopup();
		}
		else {
			continueWithoutSaving = false;
		}
		ImGui::SameLine();
		if (ImGui::Button("No")) {
			done = newAssetFile = openAssetFile = closeAssetFile = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
	else {
		continueWithoutSaving = true;
	}

	if (!continueWithoutSaving) {
		// FIXME
	}
	else if (newAssetFile) {
		ImGui::OpenPopup("New Asset File");
	}
	else if (closeAssetFile) {
		delete currentFile;
		fileOpen = fileChanged = false;
		componentViewVisible = textViewVisible = previewVisible = assetPropertiesVisible = false;
		currentFileName[0] = '\0';
		closeAssetFile = false;
		currentFile = nullptr;
	}
	else if (openAssetFile) {
		if (currentFile != nullptr) {
			delete currentFile;
		}
		fileChanged = false;
		file = OpenFileDialog("All Files\0*.*\0Asset Files (.asset)\0*.ASSET\0", 0);
		if (file[0] != '\0') {
			fileHasPath = true;
			strcpy(currentFileName, file);
			fileOpen = true;
			currentFile = new AssetFile(currentFileName);
			openAssetFile = false;
			if (currentFile->HasErrors()) {
				currentFileName[0] = '\0';
				fileOpen = false;
				delete currentFile;
				currentFile = nullptr;
				componentViewVisible = textViewVisible = previewVisible = assetPropertiesVisible = false;
			}
			else {
				componentViewVisible = true;
			}
		}
		else {
			openAssetFile = false;
		}
	}

	if (ImGui::BeginPopupModal("New Asset File", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::InputText("Name of Asset File", nameBuffer, ASSET_NAMELEN);
		ImGui::InputText("Author (your name)", authorBuffer, AUTHOR_NAMELEN);
		
		static int dlc_current = 1;
		ImGui::ListBox("DLC", &dlc_current, dlc_packages, 3, 8);
		
		if (ImGui::Button("CANCEL")) {
			ImGui::CloseCurrentPopup();
			newAssetFile = false;
		}
		ImGui::SameLine();

		bool badAssetName = false;
		bool badAuthorName = false;
		if (ImGui::Button("OK")) {
			if (strlen(nameBuffer) == 0) {
				badAssetName = true;
			}
			if (strlen(authorBuffer) == 0) {
				badAuthorName = true;
			}
			else if (!badAssetName && !badAuthorName) {
				// File successful, make a new one, but also clear the options from the previous screen
				currentFile = new AssetFile(nameBuffer, authorBuffer, dlc_packages[dlc_current]);
				fileOpen = true;
				fileChanged = true;

				strcpy(currentFileName, "< unsaved file >");

				componentViewVisible = true;
				dlc_current = 1;
				nameBuffer[0] = '\0';
				authorBuffer[0] = '\0';

				UpdateWindowTitle();

				ImGui::CloseCurrentPopup();
				newAssetFile = false;
			}
		}
		if (badAssetName) {
			ImGui::OpenPopup("Invalid Asset Name");
		}
		else if (badAuthorName) {
			ImGui::OpenPopup("Invalid Author Name");
		}

		if (ImGui::BeginPopupModal("Invalid Asset Name", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
			ImGui::Text("You need to specify an asset name.");
			if (ImGui::Button("OK")) {
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

		if (ImGui::BeginPopupModal("Invalid Author Name", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
			ImGui::Text("You need to specify an author.");
			if (ImGui::Button("OK")) {
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
		ImGui::EndPopup();
	}
	UpdateWindowTitle();
}

void RunGUI() {
	MainView();
	if (componentViewVisible) {
		ComponentView();
	}
	if (textViewVisible) {
		TextView();
	}
	if (previewVisible) {
		PreView();
	}
	if (assetPropertiesVisible) {
		AssetPropertiesView();
	}
}

void ParseCommandline(AssetToolArguments& args, int argc, char** argv) {
	bool bPreviousWasProjectFile = false;
	bool bPreviousWasOutput = false;
	for (int i = 0; i < argc; i++) {
		char* arg = argv[i];

		// Check for the parameter for any previous flags
		if (bPreviousWasProjectFile) {
			args.pfProjectFile.bFlagPresent = true;
			strcpy(args.pfProjectFile.szFlagText, arg);
			bPreviousWasProjectFile = false;
		}
		else if (bPreviousWasOutput) {
			args.pfOutputDir.bFlagPresent = true;
			strcpy(args.pfOutputDir.szFlagText, arg);
			bPreviousWasOutput = false;
		}

		// Check for argument type
		else if (arg[0] == '-') {
			// flag
			if (arg[1] == '-') {
				// non-parametered flag
			}
			else {
				// parametered flag
				if (!stricmp(arg, "-project")) {
					bPreviousWasProjectFile = true;
				}
				else if (!stricmp(arg, "-outdir")) {
					bPreviousWasOutput = true;
				}
			}
		}
	}
}

int main(int argc, char** argv) {
	AssetToolArguments args{ { 0 } };
	ParseCommandline(args, argc, argv);

	if (args.pfProjectFile.bFlagPresent) {
		if (SDL_Init(SDL_INIT_VIDEO)) {
			printf("Error: %s\n", SDL_GetError());
			return -1;
		}
		TTF_Init();
		IMG_Init(IMG_INIT_PNG);

		ProcessProjectFile(args);

		IMG_Quit();
		TTF_Quit();
		SDL_Quit();
	}
	else {
		if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS)) {
			printf("Error: %s\n", SDL_GetError());
			return -1;
		}
		TTF_Init();

		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
		SDL_DisplayMode current;
		SDL_GetCurrentDisplayMode(0, &current);
		window = SDL_CreateWindow(PROGRAM_NAME, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1024, 768, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
		SDL_GLContext glcontext = SDL_GL_CreateContext(window);

		ImGui_ImplSdl_Init(window);

		ImVec4 clear_color = ImColor(114, 144, 154);

		InitPreviewMenu();

		while (!done) {
			SDL_Event event;
			while (SDL_PollEvent(&event)) {
				ImGui_ImplSdl_ProcessEvent(&event);
				if (event.type == SDL_QUIT) {
					done = true;
				}
			}
			ImGui_ImplSdl_NewFrame(window);

			RunGUI();

			glViewport(0, 0, (int)ImGui::GetIO().DisplaySize.x, (int)ImGui::GetIO().DisplaySize.y);
			glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
			glClear(GL_COLOR_BUFFER_BIT);
			ImGui::Render();
			SDL_GL_SwapWindow(window);
		}

		if (currentFile != nullptr) {
			delete currentFile;
		}

		TTF_Quit();
		ImGui_ImplSdl_Shutdown();
		SDL_GL_DeleteContext(glcontext);
		SDL_DestroyWindow(window);
		SDL_Quit();
	}
	return 0;
}