#include "Server.h"

namespace Server {
	static bool bInitialized = false;

	/*
		Packet serialization/deserialization functions.
		The serialization function is called when a packet is next in the queue to be written.
		The deserialization function is conversely called when a packet is received.
		The deserialization function is expected to call the appropriate handler for the data as well.
		If a serialization function is null, the engine will throw a warning upon queueing it for send from modcode.
		If a deserialization function is null, the engine will throw a warning upon receipt.
	*/
	packetSerializationFunc serverFuncs[PACKET_MAX] = {
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

	packetDeserializationFunc dserverFuncs[PACKET_MAX] = {
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

	/* Serverside Initialization */
	void Initialize() {
		if (bInitialized) {
			// Prevent double initialization
			return;
		}

		trap->printf(PRIORITY_MESSAGE, "Server initialized successfully.\n");
		bInitialized = true;
	}

	/* Serverside Shutdown */
	void Shutdown() {
		if (!bInitialized) {
			// Prevent double shutdown
			return;
		}

		bInitialized = false;
	}

	/* Serverside Frame */
	void Frame() {

	}

	/*
		Packet serialization function.
		This calls the appropriate function in serverFuncs (or returns false if it doesn't exist)
		Called from the engine when a send packet attempt has been dequeued.
		The data isn't sent across the wire in serverFuncs, it's translated to packetData.
	*/
	bool SerializePacket(Packet& packet, int clientNum, void* extraData) {
		if (serverFuncs[packet.packetHead.type]) {
			serverFuncs[packet.packetHead.type](packet, clientNum, extraData);
			return true;
		}
		return false;
	}

	/*
		Packet deserialization function.
		This calls the appropriate function in dserverFuncs (or returns false if it doesn't exist)
		Called from the engine when we have received a packet.
		The data is both deserialized and used in dserverFuncs
	*/
	bool DeserializePacket(Packet& packet, int clientNum) {
		if (dserverFuncs[packet.packetHead.type]) {
			dserverFuncs[packet.packetHead.type](packet, clientNum);
			return true;
		}
		return false;
	}
}