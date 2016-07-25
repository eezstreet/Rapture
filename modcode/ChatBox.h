#pragma once
#include "ChatMessage.h"

class Chatbox {
private:
	vector<ChatMessage> vCurrentChatMessages;
	
	Cvar* cm_chatduration;
public:
	Chatbox();
	void Display();
	void AddChatMessage(int clientNum, const char* message);
};