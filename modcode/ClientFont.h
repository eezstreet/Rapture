#pragma once
#include "Modcode.h"

namespace ClientFont {
	enum Fonts {
		FONT_CONSOLAS,
		FONT_SEGOE,
		FONT_MAX
	};

	void Initialize();
	void FinishInitialization();

	Font* RetrieveFont(const Fonts font);
}