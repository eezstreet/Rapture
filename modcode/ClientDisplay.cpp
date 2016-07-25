#include "ClientDisplay.h"
#include "ClientFont.h"
#include "ChatBox.h"
#include "Client.h"

namespace ClientDisplay {
	static Cvar* cm_drawfps = nullptr;
	static Cvar* cm_drawft = nullptr;
	static Cvar* cm_drawchat = nullptr;

	static Chatbox* chat = nullptr;

	static uint64_t lastTicks = 0;

	void Initialize() {
		cm_drawfps = trap->RegisterCvarBool("cm_drawfps", "Draw FPS (frames per second).", (1 << CVAR_ARCHIVE), false);
		cm_drawft = trap->RegisterCvarBool("cm_drawft", "Draw FT (frame time, milliseconds).", (1 << CVAR_ARCHIVE), false);
		cm_drawchat = trap->RegisterCvarBool("cm_drawchat", "Draw chatbox.", (1 << CVAR_ARCHIVE), true);

		chat = new Chatbox();
	}

	void Shutdown() {
		delete chat;
	}

	void DrawDisplay() {
		bool bDrawFPS, bDrawFrameTime, bDrawChat;
		uint64_t currentTicks = trap->GetTicks();

		trap->CvarBoolVal(cm_drawfps, &bDrawFPS);
		trap->CvarBoolVal(cm_drawft, &bDrawFrameTime);
		trap->CvarBoolVal(cm_drawchat, &bDrawChat);

		Font* consolasFont = ClientFont::RetrieveFont(ClientFont::FONT_CONSOLAS);

		if (bDrawFPS) {
			float fps = 1000.0f / currentTicks - lastTicks;
			char fpsBuffer[32] {0};
			sprintf(fpsBuffer, "FPS: %.2f", fps);
			trap->RenderShadedText(consolasFont, fpsBuffer, 0, 0, 0, 0, 0, 255, 255, 255);
		}
		if (bDrawFrameTime) {
			int frametime = currentTicks - lastTicks;
			char ftBuffer[32]{0};
			sprintf(ftBuffer, "Frametime: %i ms", frametime);
			trap->RenderShadedText(consolasFont, ftBuffer, 30, 0, 0, 0, 0, 255, 255, 255);
		}
		if (bDrawChat) {
			chat->Display();
		}

		lastTicks = currentTicks;
	}

	void ReceivedChatMessage(int clientNum, const char* message) {
		chat->AddChatMessage(clientNum, message);
	}
}