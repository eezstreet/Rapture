#pragma once
#include "sys_shared.h"

namespace UI {
	void Initialize();
	void RegisterContext(const string& name, int width, int height);
	void DestroyContext(const string& name);
	void Render();
	void SendInput();
	void Destroy();

	// test
	void TestDisplay();
};