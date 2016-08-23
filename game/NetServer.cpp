#include "sys_local.h"

namespace Network {
	namespace Server {

		// Drops a client.
		void DropClient(int clientNum) {
			// TODO: gamecode for dropped client
			R_Message(PRIORITY_MESSAGE, "DropClient: %i\n", clientNum);
			numConnectedClients--;
		}


		// Send a server packet to a single client
		void SendPacketTo(packetType_e packetType, int clientNum, size_t packetDataSize) {
			uint64_t ticks = SDL_GetTicks();
			Packet packet = { { packetType, 0, packetDataSize } };
			if (clientNum == myClientNum) {
				Network::Client::DispatchSinglePacket(packet);
			}
			else {
				auto it = mOtherConnectedClients.find(clientNum);
				if (it == mOtherConnectedClients.end()) {
					R_Message(PRIORITY_WARNING, "Tried to send a packet (%i) to invalid client %i\n", packetType, clientNum);
					return;
				}
				else {
					it->second->SendPacket(packet);
					it->second->lastSpoken = ticks;
				}
			}
		}

		// Dispatches a single packet that's been sent to us from the client
		void DispatchSinglePacket(Packet& packet, int clientNum) {
			switch (packet.packetHead.type) {
			case PACKET_PING:
				R_Message(PRIORITY_MESSAGE, "Server PING from %i\n", clientNum);
				SendPacketTo(PACKET_PONG, clientNum, 0);
				break;
			case PACKET_PONG:
				R_Message(PRIORITY_MESSAGE, "Server PONG\n");
				break;
			case PACKET_DROP:
				R_Message(PRIORITY_MESSAGE, "Client %i left.\n", clientNum);
				DropClient(clientNum);
				break;
			default:
				R_Message(PRIORITY_WARNING, "Unknown packet type %i\n", packet.packetHead.type);
				break;
			}
		}

		// Start a local server
		bool StartLocalServer() {
			myClientNum = 0;
			return localSocket->StartListening(net_port->Integer(), net_serverbacklog->Integer());
		}

		// Check for new incoming temporary connections
		void CheckTemporaryConnections() {
			// vTemporary contains a list of sockets that are not validated as clients. 
			// When they are validated as clients (or time out) the sockets are destroyed.
			// Here we are polling the local socket to see if there are any temporary connections awaiting acceptance
			uint64_t ticks = SDL_GetTicks();
			Packet genericPongPacket{ { PACKET_PONG, ticks, 0 }, nullptr };
			Packet outPacket{ { PACKET_PING, 0 }, 0 };

			// Check for a new incoming temporary connection
			if (localSocket->Select()) {
				Socket* newConnection = localSocket->CheckPendingConnections();
				vTemporaryConnections.push_back(newConnection);
			}

			// Iterate through each temporary connection
			for (auto it = vTemporaryConnections.begin(); it != vTemporaryConnections.end();) {
				Socket* pSocket = *it;
				bool bNewClient = false;
				bool bDeadSocket = false;
				int msLastHeard;

				while (!bNewClient && pSocket->Select()) {
					Packet thisPacket;
					if (!pSocket->ReadPacket(thisPacket)) {
						bDeadSocket = true;
						break;
					}

					switch (thisPacket.packetHead.type) {
					case PACKET_PING:
						pSocket->SendPacket(genericPongPacket);
						break;
					case PACKET_CLIENTATTEMPT:
					{
						if (true) {
							// Send an acceptance packet with the new client number. Also remove this socket from temporary read packets

							//
							// <<<CLIENT CONNECTED>>
							//
							outPacket.packetHead.sendTime = 0; // FIXME
							outPacket.packetHead.type = PACKET_CLIENTACCEPT;
							outPacket.packetHead.packetSize = 0; // FIXME

							mOtherConnectedClients[numConnectedClients] = pSocket;

							R_Message(PRIORITY_MESSAGE, "ClientAccept: %i\n", numConnectedClients++);
							bNewClient = true;
						}
						else {
							//
							// <<<CLIENT BLOCKED>>>
							//
							outPacket.packetHead.sendTime = 0; // FIXME
							outPacket.packetHead.type = PACKET_CLIENTDENIED;
							outPacket.packetHead.packetSize = 0; // FIXME
							R_Message(PRIORITY_MESSAGE, "ClientDenied --\n");
						}
						pSocket->SendPacket(outPacket);
					}
					break;
					}
				}

				if (!bDeadSocket || bNewClient) {
					// If it's not writeable, then it died
					// If it's a new client then we should erase it too
					it = vTemporaryConnections.erase(it);
					if (bDeadSocket) {
						delete pSocket;
					}
					continue;
				}

				// Check temporary connection for timeout
				msLastHeard = ticks - pSocket->lastHeardFrom;
				if (msLastHeard > net_timeout->Integer()) {
					R_Message(PRIORITY_MESSAGE, "Closing temporary connection due to timeout\n");
					it = vTemporaryConnections.erase(it);
					delete pSocket;
				}
				else {
					it++;
				}
			}
		}

		// Send packet from server -> client
		void SendPacket(packetType_e packetType, int clientNum, size_t packetDataSize) {
			if (clientNum != -1) {
				SendPacketTo(packetType, clientNum, packetDataSize);
			}
			else {
				for (auto it = mOtherConnectedClients.begin(); it != mOtherConnectedClients.end(); ++it) {
					SendPacketTo(packetType, it->first, packetDataSize);
				}
				SendPacketTo(packetType, 0, packetDataSize);	// Also send it to ourselves
			}
		}

		// Run all of the server stuff
		void Frame() {
			uint64_t ticks = SDL_GetTicks();

			// Try listening for any temporary connections
			CheckTemporaryConnections();

			// Poll each single client individually, so we can guarantee they're valid packets
			for (auto it = mOtherConnectedClients.begin(); it != mOtherConnectedClients.end();) {
				int clientNum = it->first;
				Socket* pSocket = it->second;
				bool bClientDropped = false;

				// Socket died at some point, remove from connected client list
				if (pSocket == nullptr) {
					it = mOtherConnectedClients.erase(it);
					continue;
				}

				// Read packets
				while (pSocket->Select()) {
					Packet packet;
					if (!pSocket->ReadPacket(packet)) {
						R_Message(PRIORITY_MESSAGE, "Client %i dropped.\n", clientNum);
						DropClient(clientNum);
						bClientDropped = true;
						break;
					}
					else {
						DispatchSinglePacket(packet, clientNum);
					}
				}

				// See if we need to drop the client for timeout
				if (!bClientDropped) {
					int msReceivedDifference = ticks - pSocket->lastHeardFrom;
					int msSentDifference = ticks - pSocket->lastSpoken;
					if (msReceivedDifference > net_timeout->Integer()) {
						R_Message(PRIORITY_MESSAGE, "Dropped %i due to timeout.\n", clientNum);
						bClientDropped = true;
					}
					else if (msReceivedDifference > net_timeout->Integer() / 2 && msSentDifference >= net_timeout->Integer() / 2) {
						R_Message(PRIORITY_MESSAGE, "Haven't heard anything from %i in a while, pinging...\n", clientNum);
						SendPacketTo(PACKET_PING, clientNum, 0);
					}
				}

				// Remove the client from client list if they dropped
				if (bClientDropped) {
					it = mOtherConnectedClients.erase(it);
					delete pSocket;
				}
				else {
					++it;
				}
			}

			// Run server frame
			if (callbacks[NIC_SERVERFRAME]) {
				callbacks[NIC_SERVERFRAME](nullptr);
			}

			// Write out any packets that need sending
			for (auto& message : vPacketsAwaitingSend) {
				Packet packet = message.first;
				int clientNum = message.second;
				if (clientNum != -1) {
					auto found = mOtherConnectedClients.find(clientNum);
					if (found == mOtherConnectedClients.end()) {
						R_Message(PRIORITY_WARNING,
							"Tried to send packet %i with bad client %i\n",
							packet.packetHead.type, clientNum);
						continue;
					}
					found->second->SendPacket(packet);
					found->second->lastSpoken = ticks;
				}
				else {
					for (auto it = mOtherConnectedClients.begin(); it != mOtherConnectedClients.end(); ++it) {
						it->second->SendPacket(packet);
					}
					Network::Client::DispatchSinglePacket(packet);	// Don't forget to send to ourselves!
				}
			}

			// Clear the list of packets that need sending
			vPacketsAwaitingSend.clear();
			packetReceivingCursor = 0;
			packetSendingCursor = 0;
		}
	}
}