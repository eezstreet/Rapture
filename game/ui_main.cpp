#include "ui_local.h"

using namespace UI;

static UIFileInterface *uiFS = NULL;
static UIRenderInterface *uiTR = NULL;
static UISystemInterface *uiSYS = NULL;

static Rocket::Core::Context *cContext;

void UI::Initialize() {
	uiFS = new UIFileInterface();
	uiTR = new UIRenderInterface();
	uiSYS = new UISystemInterface();

	Rocket::Core::SetFileInterface(uiFS);
	Rocket::Core::SetRenderInterface(uiTR);
	Rocket::Core::SetSystemInterface(uiSYS);

	Rocket::Core::Initialise();
	Rocket::Controls::Initialise();

	Rocket::Core::FontDatabase::LoadFontFace("/fonts/Delicious-Bold.otf");

	cContext = Rocket::Core::CreateContext("default", Rocket::Core::Vector2i(1024, 768));
}

static unordered_map<string, Rocket::Core::Context*> contexts;

void UI::RegisterContext(const string& name, int width, int height) {
	contexts[name] = Rocket::Core::CreateContext(name.c_str(), Rocket::Core::Vector2i(width, height));
}

void UI::DestroyContext(const string& name) {
	contexts[name]->RemoveReference();
	contexts.erase(name);
}

void UI::SendInput() {
	for(auto it = contexts.begin(); it != contexts.end(); ++it) {
		it->second->Update();
	}
}

void UI::Render() {
	cContext->Render();
	for(auto it = contexts.begin(); it != contexts.end(); ++it) {
		it->second->Render();
	}
}

void UI::Destroy() {
	// Destroy any remaining contexts
	for(auto it = contexts.begin(); it != contexts.end(); ++it) {
		it->second->RemoveReference();
	}
	contexts.clear();
	cContext->RemoveReference();
	// Clean up librocket
	Rocket::Core::Shutdown();
}

void UI::TestDisplay() {
	Rocket::Core::ElementDocument* doc = cContext->LoadDocument("/ui/testdocument.rml");
	if(doc) {
		doc->Show();
		doc->RemoveReference();
	}
}