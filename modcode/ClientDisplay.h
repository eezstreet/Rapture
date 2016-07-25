#pragma once

namespace ClientDisplay {
	void Initialize();
	void Shutdown();
	void DrawDisplay();

	void ReceivedChatMessage(int clientNum, const char* message);
}