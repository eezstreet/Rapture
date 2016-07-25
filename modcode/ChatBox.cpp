#include "ChatBox.h"
#include "ClientFont.h"

Chatbox::Chatbox() {
	cm_chatduration = trap->RegisterCvarInt("cm_chatduration", "Amount of time that chat should remain onscreen (MS)", (1 << CVAR_ARCHIVE), 20000);
}

void Chatbox::Display() {
	uint64_t currentTicks = trap->GetTicks();
	int duration;

	const int chatX = 10;
	int chatY = 30;
	Font* segoeFont = ClientFont::RetrieveFont(ClientFont::FONT_SEGOE);

	trap->CvarIntVal(cm_chatduration, &duration);

	for (auto it = vCurrentChatMessages.begin(); it != vCurrentChatMessages.end(); ) {
		// Display the chat message onscreen.
		trap->RenderShadedText(segoeFont, it->szMessage, chatX, chatY, 25, 25, 25, 255, 255, 255);
		chatY += 5;

		// Determine whether the chat message should be removed
		uint64_t difference = it->uTicks - currentTicks;
		if (difference > duration) {
			it = vCurrentChatMessages.erase(it);
		}
		else {
			++it;
		}
	}
}

void Chatbox::AddChatMessage(int clientNum, const char* message) {
	uint64_t currentTicks = trap->GetTicks();

	ChatMessage msg{ currentTicks, clientNum, { 0 } };
	if (clientNum != -1) {
		sprintf(msg.szMessage, "Client %i: %s", clientNum, message);
	}
	else {
		strncpy(msg.szMessage, message, CHAT_MAXLEN);
	}

	vCurrentChatMessages.push_back(msg);
}