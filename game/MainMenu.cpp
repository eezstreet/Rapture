#include "ui_local.h"
/* Namespace management */
using namespace Awesomium;

MainMenu* MainMenu::singleton = nullptr;

MainMenu::MainMenu() {
	int renderWidth = 0, renderHeight = 0;
	bool bFullscreen = false;
	Video::GetWindowInfo(&renderWidth, &renderHeight, &bFullscreen);

	R_Message(PRIORITY_NOTE, "Loading main menu...");
	wView = UI::wc->CreateWebView(renderWidth, renderHeight, UI::sess);
	// TODO: refactor all of this into a inherited func
	wView->LoadURL(WebURL(WSLit("asset://Rapture/menus/mainmenu")));
	while(wView->IsLoading())
		UI::wc->Update();
	R_Message(PRIORITY_NOTE, "done.\n");
	UI::StartDrawingMenu(this);
	UI::currentFocus = wView;
	global = wView->CreateGlobalJavascriptObject(WSLit("GameManager"));
	wView->set_js_method_handler(this);
	JSObject jObj = global.ToObject();
	SetupBaseCommands(&jObj);
}

MainMenu::~MainMenu() {
	UI::StopDrawingMenu(this);
	wView->Destroy();
	wView = nullptr;
	UI::currentFocus = nullptr;
}

MainMenu::MainMenu(MainMenu& other) {
	// Shouldn't ever happen since we're using singleton construct
	wView = other.wView;

}

MainMenu& MainMenu::operator=(MainMenu& other) {
	// Shouldn't ever happen since we're using singleton construct
	wView = other.wView;
	return (*this);
}

void MainMenu::OnMethodCall(WebView* caller, unsigned int remote_caller_id,
		const WebString& method_name, const JSArray& args) {
	if(!ExecuteBaseCommand(ToString(method_name), args))
		return;
	// Check for our command
}

JSValue MainMenu::OnMethodCallWithReturnValue(WebView* caller, unsigned int remote_caller_id,
		const WebString& method_name, const JSArray& args) {
	auto x = ExecuteBaseCommandWithReturn(ToString(method_name), args);
	if(x.first)
		return x.second;
	// Check for our command
	return JSValue(false);
}