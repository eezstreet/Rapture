#include "Server.h"

namespace Server {
	static bool bInitialized = false;

	/* Serverside Initialization */
	void Initialize() {
		if (bInitialized) {
			return;
		}

		trap->printf(PRIORITY_MESSAGE, "Server initialized successfully.\n");
		bInitialized = true;
	}

	/* Serverside Shutdown */
	void Shutdown() {
		if (!bInitialized) {
			return;
		}

		bInitialized = false;
	}

	/* Serverside Frame */
	void Frame() {

	}

	/* Serverside Packet Receieve */
	void OnChatPacketReceived(Packet* pPacket) {
		char message[CHAT_MAXLEN];
		int clientNum = pPacket->packetHead.clientNum;
		if (pPacket->packetHead.packetSize > CHAT_MAXLEN) {
			trap->printf(PRIORITY_WARNING, "Chat message from client %i exceeded CHAT_MAXLEN\n", clientNum);
			return;
		}

		sprintf(message, "Client %i: %s", clientNum, pPacket->packetData);
		trap->printf(PRIORITY_MESSAGE, "%s\n", message);
		trap->SendServerPacket(PACKET_RECVCHAT, -1, message, CHAT_MAXLEN);
	}

	bool ClientPacket(Packet* pPacket) {
		switch (pPacket->packetHead.type) {
			case PACKET_SENDCHAT:
				// Send chat to all other clients
				OnChatPacketReceived(pPacket);
				return true;
		}
		return false;
	}
}