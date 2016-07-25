#include "ChatBox.h"
#include "ClientFont.h"

Chatbox::Chatbox() {
	cm_chatduration = trap->RegisterCvarInt("cm_chatduration", "Amount of time that chat should remain onscreen (MS)", (1 << CVAR_ARCHIVE), 20000);

	trap->AddCommand("chat", ChatCommand);
}

Chatbox::~Chatbox() {
	trap->RemoveCommand("chat");
}

void Chatbox::Display() {
	uint64_t currentTicks = trap->GetTicks();
	int duration;

	const int chatX = 10;
	int chatY = 200;
	Font* segoeFont = ClientFont::RetrieveFont(ClientFont::FONT_CONSOLAS);

	trap->CvarIntVal(cm_chatduration, &duration);

	for (auto it = vCurrentChatMessages.begin(); it != vCurrentChatMessages.end(); ) {
		// Display the chat message onscreen.
		trap->RenderShadedText(segoeFont, it->szMessage, chatX, chatY, 0, 0, 0, 255, 255, 255);
		chatY += 20;

		// Determine whether the chat message should be removed
		uint64_t difference = currentTicks - it->uTicks;
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
	strncpy(msg.szMessage, message, CHAT_MAXLEN);

	vCurrentChatMessages.push_back(msg);
}

void Chatbox::ChatCommand(vector<string>& args) {
	if (args.size() < 2) {
		trap->printf(PRIORITY_MESSAGE, "usage: chat <message>\n");
		return;
	}

	// Concatenate all arguments after the first one.
	char chatMessage[CHAT_MAXLEN]{0};
	size_t chatLen = 0;
	for (auto it = args.begin() + 1; it != args.end(); ++it) {
		size_t argLen = it->length() + 1;
		if (chatLen + argLen > CHAT_MAXLEN) {
			break;
		}
		strcat(chatMessage, it->c_str());
		strcat(chatMessage, " ");
		chatLen += argLen;
	}
	
	trap->SendClientPacket(PACKET_SENDCHAT, chatMessage, CHAT_MAXLEN);
}