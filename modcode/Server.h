#pragma once
#include "Modcode.h"

namespace Server {
	void Initialize();
	void Shutdown();

	void Frame();
	bool ClientPacket(Packet* packet);
}