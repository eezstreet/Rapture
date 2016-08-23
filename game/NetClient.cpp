#include "sys_local.h"

namespace Network {
	namespace Client {
		// Dispatches a single packet that's been sent to us from the server
		void DispatchSinglePacket(Packet& packet) {
			switch (packet.packetHead.type) {
			case PACKET_CLIENTACCEPT:
			{
				// We have been accepted into the server
				R_Message(PRIORITY_MESSAGE, "Authorization successful");
				myClientNum = 1; // FIXME
				currentNetState = Netstate_Authorized;
			}
			break;
			case PACKET_CLIENTDENIED:
			{
				// We have been denied from the server
				R_Message(PRIORITY_MESSAGE, "Denied entry from server\n");
				currentNetState = Netstate_NoConnect;
			}
			break;
			case PACKET_PING:
			{
				R_Message(PRIORITY_MESSAGE, "Server PING\n");
				SendPacket(PACKET_PONG, 0);
			}
			break;
			case PACKET_PONG:
				// TODO: use the difference from last PING packet to determine client latency
				R_Message(PRIORITY_MESSAGE, "Client PONG\n");
				break;
			case PACKET_DROP:
			{
				R_Message(PRIORITY_MESSAGE, "The server has forcibly terminated the connection.\n");
				DisconnectFromRemote();
			}
			break;
			default:
				R_Message(PRIORITY_WARNING, "Unknown packet type %i\n", packet.packetHead.type);
				break;
			}
		}

		// Try and connect to a server
		bool ConnectToRemote(const char* hostname, int port) {
			DisconnectFromRemote();
			remoteSocket = new Socket(net_ipv6->Bool() ? AF_INET6 : AF_INET, SOCK_STREAM);

			bool connected = remoteSocket->Connect(hostname, port);
			if (!connected) {
				delete remoteSocket;
				remoteSocket = nullptr;
			}
			currentNetState = Netstate_NeedAuth;
			return connected;
		}

		// Join a remote server
		bool JoinServer(const char* hostname) {
			bool connected = ConnectToRemote(hostname, net_port->Integer());
			if (!connected) {
				return false;
			}
			SendPacket(PACKET_CLIENTATTEMPT, 0);
			return true;
		}

		// Send packet from client -> server
		void SendPacket(packetType_e packetType, size_t packetDataSize) {
			Packet packet = { { packetType, 0, packetDataSize } };
			if (remoteSocket == nullptr) {
				// Not connected to a remote server, send it to ourselves instead
				Network::Server::DispatchSinglePacket(packet, myClientNum);
			}
			else {
				remoteSocket->SendPacket(packet);
			}
		}

		void Connect(const char* hostname) {
			int port = net_port->Integer();
			R_Message(PRIORITY_MESSAGE, "Connecting to %s:%i\n", hostname, port);
			if (ConnectToRemote(hostname, port)) {
				R_Message(PRIORITY_MESSAGE, "Connection established.\n", hostname, port);
			}
			else {
				R_Message(PRIORITY_MESSAGE, "Could not connect to %s:%i\n", hostname, port);
				DisconnectFromRemote();
			}
		}

		// Disconnect from current remote server
		void DisconnectFromRemote() {
			if (currentNetState == Netstate_NoConnect) {
				// Not connected in the first place
				return;
			}
			if (remoteSocket != nullptr) {
				delete remoteSocket;
				remoteSocket = nullptr;
			}
			if (callbacks[NIC_EXIT]) {
				callbacks[NIC_EXIT](nullptr);
			}
			currentNetState = Netstate_NoConnect;
			R_Message(PRIORITY_NOTE, "--- Disconnected ---\n");
		}

		// Run all of the client stuff
		void Frame() {
			uint64_t ticks = SDL_GetTicks();

			// Only do this stuff if we're on a remote server
			if (remoteSocket) {
				// Read all packets from the server
				while (remoteSocket->Select()) {
					Packet packet;
					if (!remoteSocket->ReadPacket(packet)) {
						// Close the connection?
						R_Message(PRIORITY_MESSAGE, "Connection lost.\n");
						DisconnectFromRemote();
						return;
					}
					else {
						DispatchSinglePacket(packet);
					}
				}

				// Write packets to the server
				for (auto& message : vPacketsAwaitingSend) {
					remoteSocket->SendPacket(message.first);
				}

				// Clear list of packets that need sent
				vPacketsAwaitingSend.clear();

				// Check for timeout
				int msLastHeard = ticks - remoteSocket->lastHeardFrom;
				int msLastSpoken = ticks - remoteSocket->lastSpoken;
				if (msLastHeard > net_timeout->Integer()) {
					// Drop due to timeout
					R_Message(PRIORITY_MESSAGE, "No response from server in %i milliseconds, dropping...\n", msLastHeard);
					DisconnectFromRemote();
				}
				else if (msLastHeard > net_timeout->Integer() / 2 && msLastSpoken > net_timeout->Integer() / 2) {
					// Send a PING packet to make sure we're still alive
					R_Message(PRIORITY_MESSAGE, "No server response in %i milliseconds, pinging...\n", msLastHeard);
					SendPacket(PACKET_PING, 0);
				}
			}

			// Run gamecode frame
			if (callbacks[NIC_CLIENTFRAME]) {
				callbacks[NIC_CLIENTFRAME](nullptr);
			}

			packetReceivingCursor = 0;
			packetSendingCursor = 0;
		}
	}
}