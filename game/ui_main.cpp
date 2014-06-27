#include "ui_local.h"

using namespace UI;
using namespace Awesomium;

WebCore *wc = NULL;
static WebSession *sess = NULL;

vector<Menu*> vmMenus;
static vector<WebView*> renderables;
#define NUM_UI_VISIBLE	8
static SDL_Texture* uiTextures[NUM_UI_VISIBLE];
static int lastActiveLayer;

void AddRenderable(WebView* wv) {
	renderables.push_back(wv);
}

void RemoveRenderable(WebView* wv) {
	for(auto it = renderables.begin(); it != renderables.end(); ++it) {
		if(*it == wv) {
			renderables.erase(it);
			break;
		}
	}
}

WebView* currentFocus = NULL; // If this is non-null, then we only pipe input to that object, otherwise we do this for all renderables

RightClickCallback rccb = NULL;

/* UI Class */
void UI::Initialize() {
	R_Printf("UI::Initialize()\n");
	for(int i = 0; i < NUM_UI_VISIBLE; i++) {
		uiTextures[i] = SDL_CreateTexture((SDL_Renderer*)RenderCode::GetRenderer(), SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
			CvarSystem::GetIntegerValue("r_width"), CvarSystem::GetIntegerValue("r_height"));
	}
	lastActiveLayer = 0;

	R_Printf("Initializing Awesomium Webcore\n");
	WebConfig x;
	x.log_level = Awesomium::kLogLevel_Verbose;
	wc = WebCore::Initialize(x);
	R_Printf("Creating web session\n");
	sess = wc->CreateWebSession(WSLit((CvarSystem::GetStringValue("fs_homepath") + "/session/").c_str()), WebPreferences());
	R_Printf("creating main menu webview\n");
	MainMenu::GetSingleton();
	R_Printf("CreateConsole()\n");
	Console::GetSingleton();
}

void UI::Shutdown() {
	for(int i = 0; i < NUM_UI_VISIBLE; i++) {
		SDL_DestroyTexture(uiTextures[i]);
	}
	for(auto it = vmMenus.begin(); it != vmMenus.end(); ++it) {
		Menu* vmMenu = (*it);
		delete vmMenu;
	}
	vmMenus.clear();
	Console::DestroySingleton();
	WebCore::Shutdown();
}

void UI::Update() {
	wc->Update();
}

void UI::Restart() {
	for(int i = 0; i < NUM_UI_VISIBLE; i++) {
		SDL_DestroyTexture(uiTextures[i]);
		uiTextures[i] = SDL_CreateTexture((SDL_Renderer*)RenderCode::GetRenderer(), SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
			CvarSystem::GetIntegerValue("r_width"), CvarSystem::GetIntegerValue("r_height"));
	}
}

void UI::Render() {
	lastActiveLayer = 0;
	for_each(renderables.begin(), renderables.end(), [](WebView* wv) {
		if(lastActiveLayer >= NUM_UI_VISIBLE) {
			return;
		}
		BitmapSurface* bmp = (BitmapSurface*)(wv->surface());
		SDL_Texture* tex = uiTextures[lastActiveLayer];
		unsigned char* pixels;
		int pitch;
		if(SDL_LockTexture(tex, NULL, (void**)&pixels, &pitch) < 0) {
			R_Printf("Failed to lock texture: %s\n", SDL_GetError());
			return;
		}
		bmp->CopyTo(pixels, pitch, 4, false, false);
		SDL_UnlockTexture(tex);
		RenderCode::BlendTexture((void*)tex);
		//SDL_Surface *x = SDL_CreateRGBSurfaceFrom((void*)bmp->buffer(), bmp->width(), bmp->height(), 32, bmp->row_span(), 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
		//RenderCode::AddSurface((void*)x);
		lastActiveLayer++;
	});
}

void UI::RunJavaScript(Menu* ptMenu, const char* sJS) {
	if(!ptMenu) {
		return;
	}
	R_Printf("Executing JavaScript: %s\n", sJS);
	ptMenu->RunJavaScript(sJS);
}

/* Input */
// Based on https://gist.github.com/khrona/3239125
// Modified heavily to support SDL2 and my system

#define mapKey(a, b) case SDLK_##a: return Awesomium::KeyCodes::AK_##b;
int SDL_ScancodeToAwesomium(SDL_Keycode key) {
  switch (key) {
    mapKey(BACKSPACE, BACK)
    mapKey(TAB, TAB)
    mapKey(CLEAR, CLEAR)
    mapKey(RETURN, OEM_3) // HACK: Use ` key instead of ENTER. The ENTER key is broken and it's impossible to use OEM 3 on UI
    mapKey(PAUSE, PAUSE)
    mapKey(ESCAPE, ESCAPE)
    mapKey(SPACE, SPACE)
    mapKey(EXCLAIM, 1)
    mapKey(QUOTEDBL, 2)
    mapKey(HASH, 3)
    mapKey(DOLLAR, 4)
    mapKey(AMPERSAND, 7)
    mapKey(QUOTE, OEM_7)
    mapKey(LEFTPAREN, 9)
    mapKey(RIGHTPAREN, 0)
    mapKey(ASTERISK, 8)
    mapKey(PLUS, OEM_PLUS)
    mapKey(COMMA, OEM_COMMA)
    mapKey(MINUS, OEM_MINUS)
    mapKey(PERIOD, OEM_PERIOD)
    mapKey(SLASH, OEM_2)
    mapKey(0, 0)
    mapKey(1, 1)
    mapKey(2, 2)
    mapKey(3, 3)
    mapKey(4, 4)
    mapKey(5, 5)
    mapKey(6, 6)
    mapKey(7, 7)
    mapKey(8, 8)
    mapKey(9, 9)
    mapKey(COLON, OEM_1)
    mapKey(SEMICOLON, OEM_1)
    mapKey(LESS, OEM_COMMA)
    mapKey(EQUALS, OEM_PLUS)
    mapKey(GREATER, OEM_PERIOD)
    mapKey(QUESTION, OEM_2)
    mapKey(AT, 2)
    mapKey(LEFTBRACKET, OEM_4)
    mapKey(BACKSLASH, OEM_5)
    mapKey(RIGHTBRACKET, OEM_6)
    mapKey(CARET, 6)
    mapKey(UNDERSCORE, OEM_MINUS)
    mapKey(BACKQUOTE, OEM_3)
    mapKey(a, A)
    mapKey(b, B)
    mapKey(c, C)
    mapKey(d, D)
    mapKey(e, E)
    mapKey(f, F)
    mapKey(g, G)
    mapKey(h, H)
    mapKey(i, I)
    mapKey(j, J)
    mapKey(k, K)
    mapKey(l, L)
    mapKey(m, M)
    mapKey(n, N)
    mapKey(o, O)
    mapKey(p, P)
    mapKey(q, Q)
    mapKey(r, R)
    mapKey(s, S)
    mapKey(t, T)
    mapKey(u, U)
    mapKey(v, V)
    mapKey(w, W)
    mapKey(x, X)
    mapKey(y, Y)
    mapKey(z, Z)
    mapKey(DELETE, DELETE)
    mapKey(KP_0, NUMPAD0)
    mapKey(KP_1, NUMPAD1)
    mapKey(KP_2, NUMPAD2)
    mapKey(KP_3, NUMPAD3)
    mapKey(KP_4, NUMPAD4)
    mapKey(KP_5, NUMPAD5)
    mapKey(KP_6, NUMPAD6)
    mapKey(KP_7, NUMPAD7)
    mapKey(KP_8, NUMPAD8)
    mapKey(KP_9, NUMPAD9)
    mapKey(KP_PERIOD, DECIMAL)
    mapKey(KP_DIVIDE, DIVIDE)
    mapKey(KP_MULTIPLY, MULTIPLY)
    mapKey(KP_MINUS, SUBTRACT)
    mapKey(KP_PLUS, ADD)
	mapKey(KP_ENTER, OEM_3) // HACK
    mapKey(KP_EQUALS, UNKNOWN)
    mapKey(UP, UP)
    mapKey(DOWN, DOWN)
    mapKey(RIGHT, RIGHT)
    mapKey(LEFT, LEFT)
    mapKey(INSERT, INSERT)
    mapKey(HOME, HOME)
    mapKey(END, END)
    mapKey(PAGEUP, PRIOR)
    mapKey(PAGEDOWN, NEXT)
    mapKey(F1, F1)
    mapKey(F2, F2)
    mapKey(F3, F3)
    mapKey(F4, F4)
    mapKey(F5, F5)
    mapKey(F6, F6)
    mapKey(F7, F7)
    mapKey(F8, F8)
    mapKey(F9, F9)
    mapKey(F10, F10)
    mapKey(F11, F11)
    mapKey(F12, F12)
    mapKey(F13, F13)
    mapKey(F14, F14)
    mapKey(F15, F15)
    mapKey(NUMLOCKCLEAR, NUMLOCK)
    mapKey(CAPSLOCK, CAPITAL)
    mapKey(SCROLLLOCK, SCROLL)
    mapKey(RSHIFT, RSHIFT)
    mapKey(LSHIFT, LSHIFT)
    mapKey(RCTRL, RCONTROL)
    mapKey(LCTRL, LCONTROL)
    mapKey(RALT, RMENU)
    mapKey(LALT, LMENU)
    mapKey(RGUI, LWIN)
    mapKey(LGUI, RWIN)
    mapKey(MODE, MODECHANGE)
    //mapKey(COMPOSE, ACCEPT)
    mapKey(HELP, HELP)
    mapKey(PRINTSCREEN, SNAPSHOT)
    mapKey(SYSREQ, EXECUTE)
  default:
    return Awesomium::KeyCodes::AK_UNKNOWN;
  }
}

void InjectAKeyboardEvent(Awesomium::WebKeyboardEvent e) {
	if(currentFocus != NULL)
		currentFocus->InjectKeyboardEvent(e);
	else
		for(auto it = renderables.begin(); it != renderables.end(); ++it)
			(*it)->InjectKeyboardEvent(e);
}

void UI::KeyboardEvent(SDL_Keysym keysym, bool bIsKeyDown, char* text) {
	if(keysym.scancode == SDL_SCANCODE_GRAVE) {
		if(bIsKeyDown) {
			if(!Console::GetSingleton()->IsOpen()) {
				Console::GetSingleton()->Show();
			} else {
				Console::GetSingleton()->Hide();
			}
		}
		return;
	}

	WebKeyboardEvent e;
	if(bIsKeyDown)
		e.type = WebKeyboardEvent::kTypeKeyDown;
	else
		e.type = WebKeyboardEvent::kTypeKeyUp;

	char *dummy = (char*)malloc(20);
	e.virtual_key_code = SDL_ScancodeToAwesomium(keysym.sym);
	e.native_key_code = keysym.scancode;
	GetKeyIdentifierFromVirtualKeyCode(e.virtual_key_code, &dummy);
	strcpy(e.key_identifier, dummy);
	free(dummy);

	e.modifiers = 0;
	if(keysym.mod & KMOD_ALT)
		e.modifiers |= WebKeyboardEvent::kModAltKey;
	if(keysym.mod & KMOD_CTRL)
		e.modifiers |= WebKeyboardEvent::kModControlKey;
	if(keysym.mod & KMOD_GUI)
		e.modifiers |= WebKeyboardEvent::kModMetaKey;
	if(keysym.mod & KMOD_SHIFT)
		e.modifiers |= WebKeyboardEvent::kModShiftKey;
	if(keysym.mod & KMOD_NUM)
		e.modifiers |= WebKeyboardEvent::kModIsKeypad;

	if(e.virtual_key_code == 192) {
		e.text[0] = '`';
		e.native_key_code = 192;
		if(bIsKeyDown)
			UI::TextEvent("`");
		InjectAKeyboardEvent(e);
		return;
	}

	if(e.virtual_key_code == 8) {
		InjectAKeyboardEvent(e);
	}
	else if(!bIsKeyDown) {
		InjectAKeyboardEvent(e);
	}
	else {
		InjectAKeyboardEvent(e);
		if(!text)
			return;

		wstring wtext;
		towstring(text, wtext);
		e.text[0] = wtext[0];
		e.unmodified_text[0] = wtext[0];
		e.type = WebKeyboardEvent::kTypeChar;

		InjectAKeyboardEvent(e);
	}
}

void UI::TextEvent(char* text) {
	WebKeyboardEvent e;

	if(!text)
		return;

	wstring wtext;
	towstring(text, wtext);
	e.text[0] = wtext[0];
	e.type = WebKeyboardEvent::kTypeChar;

	InjectAKeyboardEvent(e);
}

void PipeMouseInputToWebView(WebView* wv, unsigned int buttonId, bool down) {
	if(buttonId == SDL_BUTTON_LEFT) {
		if(down)
			wv->InjectMouseDown(Awesomium::kMouseButton_Left);
		else
			wv->InjectMouseUp(Awesomium::kMouseButton_Left);
	}
	if(buttonId == SDL_BUTTON_MIDDLE) {
		if(down)
			wv->InjectMouseDown(Awesomium::kMouseButton_Middle);
		else
			wv->InjectMouseUp(Awesomium::kMouseButton_Middle);
	}
	if(buttonId == SDL_BUTTON_RIGHT) {
		if(down)
			wv->InjectMouseDown(Awesomium::kMouseButton_Right);
		else
			wv->InjectMouseUp(Awesomium::kMouseButton_Right);
	}
}

void UI::MouseButtonEvent(unsigned int buttonId, bool down) {
	if(currentFocus != NULL) {
		PipeMouseInputToWebView(currentFocus, buttonId, down);
		if(buttonId == SDL_BUTTON_RIGHT && down) {
			// special callback
			if(rccb != NULL) {
				rccb();
			}
		}
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

Menu* UI::RegisterStaticMenu(const char* menuName) {
	Menu* newMenu = new Menu(menuName);
	vmMenus.push_back(newMenu);
	return newMenu;
}

void UI::KillStaticMenu(Menu* menu) {
	if(vmMenus.empty()) {
		return;
	}
	for(auto it = vmMenus.begin(); it != vmMenus.end(); ++it) {
		if((*it) == menu) {
			Menu* ptMenu = (Menu*)menu;
			delete ptMenu;
			vmMenus.erase(it);
			return;
		}
	}
}