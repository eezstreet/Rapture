#include "ui_local.h"

using namespace UI;
using namespace Awesomium;

WebCore *wc = NULL;
static WebSession *sess = NULL;
static WebView *mainView = NULL;

static bool bConsoleIsActive = false;

static vector<WebView*> renderables;
void AddRenderable(WebView* wv) {
	renderables.push_back(wv);
}

void RemoveRenderable(WebView* wv) {
	for(auto it = renderables.begin(); it != renderables.end(); ++it) {
		if(*it == wv)
			renderables.erase(it);
	}
}

WebView* currentFocus = NULL; // If this is non-null, then we only pipe input to that object, otherwise we do this for all renderables

/* UI Class */
void UI::Initialize() {
	wc = WebCore::Initialize(WebConfig());
	sess = wc->CreateWebSession(WSLit("C:\\Rapture\\Gamedata\\core\\session"), WebPreferences()); // TODO: use homepath
	mainView = wc->CreateWebView(1024, 768, sess);
	mainView->LoadURL(WebURL(WSLit("http://jkhub.org")));
	while(mainView->IsLoading())
		wc->Update();
	AddRenderable(mainView);

}

void UI::Shutdown() {
	if(bConsoleIsActive)
		DestroyConsole(); // destroy the console if it's open
	WebCore::Shutdown();
}

void UI::Update() {
	wc->Update();
}

void UI::Render() {
	for(auto it = renderables.begin(); it != renderables.end(); ++it) {
		BitmapSurface* bmp = (BitmapSurface*)(*it)->surface();
		if(!bmp)
			return;
		SDL_Surface *x = SDL_CreateRGBSurfaceFrom((void*)bmp->buffer(), bmp->width(), bmp->height(), 32, bmp->row_span(), 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
		//SDL_ConvertSurfaceFormat(x, SDL_PIXELFORMAT_BGRA8888, 0);
		RenderCode::AddSurface((void*)x);
	}
}

void UI::KeyboardEvent(SDL_Scancode sc) {
	if(sc == SDL_SCANCODE_GRAVE) {
		bConsoleIsActive = !bConsoleIsActive;
		if(bConsoleIsActive) {
			// Activate the console.
			CreateConsole();
		}
		else {
			// Deactivate the console.
			DestroyConsole();
		}
	}
}

void PipeMouseInputToWebView(WebView* wv, unsigned int buttonId, bool down) {
	if(buttonId & SDL_BUTTON_LMASK) {
		if(down)
			wv->InjectMouseDown(Awesomium::kMouseButton_Left);
		else
			wv->InjectMouseUp(Awesomium::kMouseButton_Left);
	}
	if(buttonId & SDL_BUTTON_MMASK) {
		if(down)
			wv->InjectMouseDown(Awesomium::kMouseButton_Middle);
		else
			wv->InjectMouseUp(Awesomium::kMouseButton_Middle);
	}
	if(buttonId & SDL_BUTTON_RMASK) {
		if(down)
			wv->InjectMouseDown(Awesomium::kMouseButton_Right);
		else
			wv->InjectMouseUp(Awesomium::kMouseButton_Right);
	}
}

void UI::MouseButtonEvent(unsigned int buttonId, bool down) {
	if(currentFocus != NULL) {
		PipeMouseInputToWebView(currentFocus, buttonId, down);
	}
	else {
		for(auto it = renderables.begin(); it != renderables.end(); ++it) {
			PipeMouseInputToWebView(*it, buttonId, down);
		}
	}
}

void PipeMouseMovementToWebView(WebView* wv, int x, int y) {
	if(wv)
		wv->InjectMouseMove(x, y);
}

void UI::MouseMoveEvent(int x, int y) {
	if(currentFocus)
		currentFocus->InjectMouseMove(x, y);
	else {
		for(auto it = renderables.begin(); it != renderables.end(); ++it) {
			(*it)->InjectMouseMove(x,y);
		}
	}
}