#pragma once
#include "Modcode.h"

namespace Server {
	void Initialize();
	void Shutdown();

	void Frame();
	bool SerializePacket(Packet& packet, int clientNum, void* extraData);
	bool DeserializePacket(Packet& packet, int clientNum);

	extern packetSerializationFunc serverFuncs[PACKET_MAX];
	extern packetDeserializationFunc dserverFuncs[PACKET_MAX];
}