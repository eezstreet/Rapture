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
		char message[140];
		if (pPacket->packetHead.packetSize > CHAT_MAXLEN) {
			trap->printf(PRIORITY_WARNING, "Chat message from client %i exceeded CHAT_MAXLEN\n", pPacket->packetHead.clientNum);
			return;
		}
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