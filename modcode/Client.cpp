#include "Client.h"
#include "ClientDisplay.h"
#include "ClientFont.h"

namespace Client {
	static bool bInitialized = false;

	/* 
		Packet serialization/deserialization functions.
		The serialization function is called when a packet is next in the queue to be written.
		The deserialization function is conversely called when a packet is received.
		The deserialization function is expected to call the appropriate handler for the data as well.
		If a serialization function is null, the engine will throw a warning upon queueing it for send from modcode.
		If a deserialization function is null, the engine will throw a warning upon receipt.
	*/
	packetSerializationFunc clientFuncs[PACKET_MAX] = {
		nullptr,			// PACKET_PING
		nullptr,			// PACKET_PONG
		nullptr,			// PACKET_DROP
		nullptr,			// PACKET_CLIENTATTEMPT
		nullptr,			// PACKET_CLIENTACCEPT
		nullptr,			// PACKET_CLIENTDENIED
		nullptr,			// PACKET_INFOREQUEST
		nullptr,			// PACKET_INFOREQUESTED
		nullptr,			// PACKET_SENDCHAT // FIXME
		nullptr,			// PACKET_RECVCHAT // FIXME
	};

	packetDeserializationFunc dclientFuncs[PACKET_MAX] = {
		nullptr,			// PACKET_PING
		nullptr,			// PACKET_PONG
		nullptr,			// PACKET_DROP
		nullptr,			// PACKET_CLIENTATTEMPT
		nullptr,			// PACKET_CLIENTACCEPT
		nullptr,			// PACKET_CLIENTDENIED
		nullptr,			// PACKET_INFOREQUEST
		nullptr,			// PACKET_INFOREQUESTED
		nullptr,			// PACKET_SENDCHAT // FIXME
		nullptr,			// PACKET_RECVCHAT // FIXME
	};

	/* Initialization code */
	void Initialize() {
		if (bInitialized) {
			return;
		}
		ClientFont::Initialize();
		ClientDisplay::Initialize();

		trap->printf(PRIORITY_MESSAGE, "Waiting for additional elements to complete...\n");
		ClientFont::FinishInitialization();

		trap->printf(PRIORITY_MESSAGE, "Client initialized successfully.\n");
		bInitialized = true;
	}

	/* Shutdown code */
	void Shutdown() {
		if (!bInitialized) {
			return;
		}
		ClientDisplay::Shutdown();
		bInitialized = false;
	}

	/* Code that gets run every frame */
	void Frame() {
		ClientDisplay::DrawDisplay();
	}

	/*
		Packet serialization function.
		This calls the appropriate function in clientFuncs (or returns false if it doesn't exist)
		Called from the engine when a send packet attempt has been dequeued.
		The data isn't sent across the wire in clientFuncs, it's translated to packetData.
	*/
	bool SerializePacket(Packet& packet, int clientNum, void* extraData) {
		if (clientFuncs[packet.packetHead.type]) {
			clientFuncs[packet.packetHead.type](packet, clientNum, extraData);
			return true;
		}
		return false;
	}

	/*
		Packet deserialization function.
		This calls the appropriate function in dclientFuncs (or returns false if it doesn't exist)
		Called from the engine when we have received a packet.
		The data is both deserialized and used in dclientFuncs
	*/
	bool DeserializePacket(Packet& packet, int clientNum) {
		if (dclientFuncs[packet.packetHead.type]) {
			dclientFuncs[packet.packetHead.type](packet, clientNum);
			return true;
		}
		return false;
	}
}