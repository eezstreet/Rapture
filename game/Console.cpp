#include "sys_local.h"
#include "ui_local.h"

using namespace Awesomium;

Console* con = NULL;

static WebView* consoleView = NULL;

void CreateConsole() {
	string conFileName = "file://" + File::GetFileSearchPath("ui/console.html");
	consoleView = wc->CreateWebView(1024, 768);
	consoleView->LoadURL(WebURL(WSLit(conFileName.c_str())));
	consoleView->SetTransparent(true);
	AddRenderable(consoleView);
	currentFocus = consoleView;
}

void DestroyConsole() {
	RemoveRenderable(consoleView);
	consoleView->Destroy();
	consoleView = NULL;
	currentFocus = NULL;
}

Console::Console() {

}