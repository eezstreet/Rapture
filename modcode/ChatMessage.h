#pragma once
#include "Modcode.h"

struct ChatMessage {
	uint64_t uTicks;
	uint32_t uSender;
	char szMessage[CHAT_MAXLEN];
};