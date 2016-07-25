#pragma once
#include "ChatMessage.h"

class Chatbox {
private:
	vector<ChatMessage> vCurrentChatMessages;
	
	Cvar* cm_chatduration;
public:
	Chatbox();
	~Chatbox();
	void Display();
	void AddChatMessage(int clientNum, const char* message);

	static void ChatCommand(vector<string>& args);
};