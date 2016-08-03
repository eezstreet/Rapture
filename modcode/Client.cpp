#include "Client.h"
#include "ClientDisplay.h"
#include "ClientFont.h"

namespace Client {
	static bool bInitialized = false;

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

	/* Code that gets run every packet */
	bool ServerPacket(Packet* pPacket) {
		switch (pPacket->packetHead.type) {
			case PACKET_RECVCHAT:
				if (pPacket->packetHead.packetSize > 0) {
					ClientDisplay::ReceivedChatMessage(0, (const char*)pPacket->packetData);
				}
				return true;
		}
		return false;
	}
}