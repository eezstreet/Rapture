#pragma once
#include "Modcode.h"

namespace Client {
	void Initialize();
	void Shutdown();

	void Frame();
	bool SerializePacket(Packet& packet, int clientNum, void* extraData);
	bool DeserializePacket(Packet& packet, int clientNum);

	extern packetSerializationFunc clientFuncs[PACKET_MAX];
	extern packetDeserializationFunc dclientFuncs[PACKET_MAX];
}