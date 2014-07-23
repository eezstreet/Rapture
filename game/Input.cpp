#include "sys_local.h"
#include "ui_shared.h"

InputManager *Input = nullptr;

const string keycodeNames[] = {
	"unknown",
	"a",
	"b",
	"c",
	"d",
	"e",
	"f",
	"g",
	"h",
	"i",
	"j",
	"k",
	"l",
	"m",
	"n",
	"o",
	"p",
	"q",
	"r",
	"s",
	"t",
	"u",
	"v",
	"w",
	"x",
	"y",
	"z",
	"1",
	"2",
	"3",
	"4",
	"5",
	"6",
	"7",
	"8",
	"9",
	"0",
	"enter",
	"esc",
	"backspace",
	"tab",
	"space",
	"-",
	"=",
	"[",
	"]",
	"\\",
	"\\",
	";",
	"'",
	"`",
	",",
	".",
	"/",
	"capslock",
	"f1",
	"f2",
	"f3",
	"f4",
	"f5",
	"f6",
	"f7",
	"f8",
	"f9",
	"f10",
	"f11",
	"f12",
	"printscreen",
	"scrolllock",
	"pause",
	"insert",
	"home",
	"pageup",
	"delete",
	"end",
	"pagedown",
	"right",
	"left",
	"down",
	"up",
#ifdef _WIN32
	"numlock",
#else
	"clear",
#endif
	"kp_divide",
	"kp_multiply",
	"kp_minus",
	"kp_plus",
	"kp_enter",
	"kp_1",
	"kp_2",
	"kp_3",
	"kp_4",
	"kp_5",
	"kp_6",
	"kp_7",
	"kp_8",
	"kp_9",
	"kp_period",
#ifdef _WIN32
	"contextual",
#else
	"compose",
#endif
	"power",
	// HACK
	"lctrl",
	"lshift",
	"lalt",
	"lgui",
	"rctrl",
	"rshift",
	"ralt",
	"rgui"
	// END HACK
};

void InitInput() {
	Input = new InputManager();
}

void DeleteInput() {
	delete Input;
}

void InputManager::SendKeyDownEvent(SDL_Keysym key, char* text) {
	SDL_Scancode k = key.scancode;
	auto it = find(thisFrameKeysDown.begin(), thisFrameKeysDown.end(), k);
	if(it != thisFrameKeysDown.end())
		return;
	thisFrameKeysDown.push_back(k);

	// TODO: keycatchers
	UI::KeyboardEvent(key, true, text);
}

void InputManager::SendKeyUpEvent(SDL_Keysym key, char* text) {
	SDL_Scancode k = key.scancode;
	if(thisFrameKeysDown.end() == thisFrameKeysDown.begin()) {
		return;
	}
	auto it = find(thisFrameKeysDown.begin(), thisFrameKeysDown.end(), k);
	if(it == thisFrameKeysDown.end())
		return;
	thisFrameKeysDown.erase(it);

	// TODO: keycatchers
	UI::KeyboardEvent(key, false, text);
}

void InputManager::InputFrame() {
	SDL_PumpEvents();
	for(auto it = thisFrameKeysDown.begin(); it != thisFrameKeysDown.end(); ++it)
		ExecuteBind(binds[*it]);
	thisFrameKeysDown.clear();
}

void InputManager::BindCommand(const string& keycodeArg, string commandArg) {
	int i;
	for(i = 0; i < SDL_SCANCODE_F19; i++) {
		if(keycodeNames[i] == keycodeArg) { // FIXME
			break;
		}
	}
	if(i >= SDL_SCANCODE_KP_EQUALS) {
		i += 121; // HACK
	}
	SDL_Scancode sc = (SDL_Scancode)i;
	binds[sc] = commandArg;
}

InputManager::InputManager() {
	Keycatcher = nullptr;
}

void InputManager::ExecuteBind(const string& bindStuff) {
}

bool bVMInputBlocked = false;
void InputManager::SendMouseButtonEvent(unsigned int buttonId, unsigned char state, int x, int y) {
	if(state == SDL_PRESSED) {
		UI::MouseButtonEvent(buttonId, true);
		if(RaptureGame::GetGameModule() != nullptr) {
			if(!bVMInputBlocked) {
				RaptureGame::GetImport()->passmousedown(x, y);
			}
		}
	} else {
		UI::MouseButtonEvent(buttonId, false);
		if(RaptureGame::GetGameModule() != nullptr) {
			RaptureGame::GetImport()->passmouseup(x, y);
		}
	}
}

void InputManager::SendMouseMoveEvent(int x, int y) {
	if(RaptureGame::GetGameModule() != nullptr) {
		RaptureGame::GetImport()->passmousemove(x, y);
	}
	UI::MouseMoveEvent(x, y);
}

void InputManager::SendTextInputEvent(char* text) {
	UI::TextEvent(text);
}