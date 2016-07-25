#pragma once
#include "Modcode.h"

namespace Client {
	void Initialize();
	void Shutdown();

	void Frame();
	bool ServerPacket(Packet* pPacket);
}