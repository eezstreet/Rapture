#include "sys_local.h"
#include "ui_shared.h"

InputManager *Input = nullptr;

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

bool bVMInputBlocked = false;
void InputManager::SendKeyUpEvent(SDL_Keysym key, char* text) {
	SDL_Scancode k = key.scancode;

	UI::KeyboardEvent(key, false, text);
	if(RaptureGame::GetGameModule() != nullptr || RaptureGame::GetEditorModule() != nullptr) {
		if(!bVMInputBlocked) {
			RaptureGame::GetImport()->passkeypress(key.scancode);
		}
	}
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

void InputManager::SendMouseButtonEvent(unsigned int buttonId, unsigned char state, int x, int y) {
	if(state == SDL_PRESSED) {
		UI::MouseButtonEvent(buttonId, true);
		if(RaptureGame::GetGameModule() != nullptr || RaptureGame::GetEditorModule() != nullptr) {
			if(!bVMInputBlocked) {
				RaptureGame::GetImport()->passmousedown(x, y);
			}
		}
	} else {
		UI::MouseButtonEvent(buttonId, false);
		if(RaptureGame::GetGameModule() != nullptr || RaptureGame::GetEditorModule() != nullptr) {
			RaptureGame::GetImport()->passmouseup(x, y);
		}
	}
}

void InputManager::SendMouseMoveEvent(int x, int y) {
	if(RaptureGame::GetGameModule() != nullptr || RaptureGame::GetEditorModule() != nullptr) {
		RaptureGame::GetImport()->passmousemove(x, y);
	}
	UI::MouseMoveEvent(x, y);
}

void InputManager::SendTextInputEvent(char* text) {
	UI::TextEvent(text);
}