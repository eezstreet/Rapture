#include "sys_local.h"

namespace Network {
	namespace Client {
		/*
			Clientside - Engine packet serialization and deserialization functions
		*/
		packetSerializationFunc engineFuncs[PACKET_MODCODE_START] = {
			nullptr,														// PACKET_PING
			nullptr,														// PACKET_PONG
			Network::Packets::Client::Packet_DropSerialize,					// PACKET_DROP
			Network::Packets::Client::Packet_ClientAttemptSerialize,		// PACKET_CLIENTATTEMPT
			nullptr,														// PACKET_CLIENTACCEPT
			nullptr,														// PACKET_CLIENTDENIED
			nullptr,														// PACKET_INFOREQUEST
			nullptr,														// PACKET_INFOREQUESTED
		};

		packetDeserializationFunc dengineFuncs[PACKET_MODCODE_START] = {
			Network::Packets::Server::Packet_PingDeserialize,				// PACKET_PING
			nullptr,														// PACKET_PONG
			Network::Packets::Client::Packet_DropDeserialize,				// PACKET_DROP
			nullptr,														// PACKET_CLIENTATTEMPT
			Network::Packets::Client::Packet_ClientAcceptDeserialize,		// PACKET_CLIENTACCEPT
			Network::Packets::Client::Packet_ClientDeniedDeserialize,		// PACKET_CLIENTDENIED
			nullptr,														// PACKET_INFOREQUEST
			nullptr,														// PACKET_INFOREQUESTED
		};


		/*
			Packet ops - Queueing and dispatching packets
		*/

		// Queue a packet to be sent from the client to the server
		void QueuePacket(packetType_e packetType, void* extraData) {
			Packet packet = { { packetType, 0, 0 }, { 0 } };
			if (packetType < PACKET_MODCODE_START && engineFuncs[packetType]) {
				engineFuncs[packetType](packet, myClientNum, extraData);
			}
			if (callbacks[NIC_CLIENTSERIALIZE]) {
				packetSerializationFunc serialFunc = (packetSerializationFunc)callbacks[NIC_CLIENTSERIALIZE];
				serialFunc(packet, myClientNum, extraData);
			}
			if (remoteSocket) {
				vPacketsAwaitingSend.push_back(make_pair(packet, myClientNum));
			}
			else {
				Network::Server::DispatchSinglePacket(packet, myClientNum);
			}
		}

		// Dispatches a single packet that's been sent to us from the server
		void DispatchSinglePacket(Packet& packet) {
			if (dengineFuncs[packet.packetHead.type]) {
				dengineFuncs[packet.packetHead.type](packet, myClientNum);
			}
			if (callbacks[NIC_CLIENTDESERIALIZE]) {
				packetDeserializationFunc dfunc = (packetDeserializationFunc)callbacks[NIC_CLIENTDESERIALIZE];
				dfunc(packet, myClientNum);
			}
/*			switch (packet.packetHead.type) {
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
*/
		}

		/*
			Actions which are performed every frame.
			This is significantly less complex than the server process.
				0. If we're not connected to a server, skip to step #, otherwise...
				1. Read/deserialize packets, perform logic on what was read
				2. Write packets from the previous frame
				3. Check whether we should disconnect from the server 
		*/
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

					if (remoteSocket == nullptr) {
						Network::Server::DispatchSinglePacket(message.first, myClientNum);
					}
					else {
						remoteSocket->SendPacket(message.first);
					}
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
					QueuePacket(PACKET_PING, nullptr);
				}
			}

			// Run gamecode frame
			if (callbacks[NIC_CLIENTFRAME]) {
				callbacks[NIC_CLIENTFRAME](nullptr);
			}
		}

		/*
			Actions which are initiated from the client to try and connect/disconnect from servers.
		*/
		// Try and connect to a server
		// Don't call this directly, call JoinServer instead
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
			QueuePacket(PACKET_CLIENTATTEMPT, nullptr);
			return true;
		}

		// Not used
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
	}
}