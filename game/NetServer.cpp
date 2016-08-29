#include "sys_local.h"

namespace Network {
	namespace Server {
		/*
			Serverside - Engine packet serialization and deserialization functions
		*/
		packetSerializationFunc engineFuncs[PACKET_MODCODE_START] = {
			nullptr,														// PACKET_PING
			nullptr,														// PACKET_PONG
			Network::Packets::Server::Packet_DropSerialize,					// PACKET_DROP
			nullptr,														// PACKET_CLIENTATTEMPT
			Network::Packets::Server::Packet_ClientAcceptSerialize,			// PACKET_CLIENTACCEPT
			Network::Packets::Server::Packet_ClientDeniedSerialize,			// PACKET_CLIENTDENIED
			nullptr,														// PACKET_INFOREQUEST
			nullptr,														// PACKET_INFOREQUESTED
		};

		packetDeserializationFunc dengineFuncs[PACKET_MODCODE_START] = {
			Network::Packets::Server::Packet_PingDeserialize,				// PACKET_PING
			nullptr,														// PACKET_PONG
			Network::Packets::Server::Packet_DropDeserialize,				// PACKET_DROP
			Network::Packets::Server::Packet_ClientAttemptDeserialize,		// PACKET_CLIENTATTEMPT
			nullptr,														// PACKET_CLIENTACCEPT
			nullptr,														// PACKET_CLIENTDENIED
			nullptr,														// PACKET_INFOREQUEST
			nullptr,														// PACKET_INFOREQUESTED
		};


		/*
			Packet ops - Queueing and dispatching packets
		*/

		// Queue a packet that's going to be sent to a specific client
		// This function should never be called directly; QueuePacket should be called instead.
		void QueuePacketWithDestination(packetType_e packetType, int clientNum, void* extraData) {
			Packet queuedPacket{ { packetType, SDL_GetTicks(), 0 }, { 0 } };
			if (packetType < PACKET_MODCODE_START && engineFuncs[packetType]) {
				engineFuncs[packetType](queuedPacket, clientNum, extraData);
			}
			if (callbacks[NIC_SERVERSERIALIZE]) {
				packetSerializationFunc serFunc = (packetSerializationFunc)callbacks[NIC_SERVERSERIALIZE];
				serFunc(queuedPacket, clientNum, extraData);
			}
			vPacketsAwaitingSend.push_back(make_pair(queuedPacket, clientNum));
		}

		// Queue a packet to be sent.
		// If the clientNum is -1, it will send to all clients.
		void QueuePacket(packetType_e packetType, int clientNum, void* extraData) {
			if (clientNum < 0) {
				// Send it to all other connected clients
				for (auto it = mOtherConnectedClients.begin(); it != mOtherConnectedClients.end(); ++it) {
					QueuePacketWithDestination(packetType, it->first, extraData);
				}
				// Send it to ourself as well
				QueuePacketWithDestination(packetType, 0, extraData);
			}
			else {
				QueuePacketWithDestination(packetType, clientNum, extraData);
			}
		}

		// Dispatches a single packet that's been sent to us from the client
		void DispatchSinglePacket(Packet& packet, int clientNum) {
			if (dengineFuncs[packet.packetHead.type]) {
				dengineFuncs[packet.packetHead.type](packet, clientNum);
			}
			if (callbacks[NIC_SERVERDESERIALIZE]) {
				packetDeserializationFunc defunc = (packetDeserializationFunc)callbacks[NIC_SERVERDESERIALIZE];
				defunc(packet, clientNum);
			}
		}


		/*
			These actions are performed every frame.
			The process can be broken up as follows:
				0. Listen in for (and communicate with) temporary connections - connections which aren't guaranteed as clients
				1. Select each client socket and make sure it's still valid. Pump out any packets.
				2. Deserialize any packets and perform logic based on what we've received.
				3. Drop any clients who haven't responded in a long enough time.
				4. Perform gamecode logic - most of this is handled in gamex86.dll
				5. Serialize and send any packets which were queued from gamecode or any previous steps. Clear the queue.
		*/

		// Check for new incoming temporary connections
		void CheckTemporaryConnections() {
			// vTemporary contains a list of sockets that are not validated as clients. 
			// When they are validated as clients (or time out) the sockets are destroyed.
			// Here we are polling the local socket to see if there are any temporary connections awaiting acceptance
			uint64_t ticks = SDL_GetTicks();
			Packet genericPongPacket{ { PACKET_PONG, ticks, 0 }, { 0 } };
			Packet outPacket{ { PACKET_PING, 0 }, { 0 } };

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
							if (true) {	// FIXME
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

		// Send out any packets that are queued
		void ProcessPacketQueue() {
			uint64_t ticks = SDL_GetTicks();

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
						QueuePacket(PACKET_PING, clientNum, 0);
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
			ProcessPacketQueue();
		}
	}

	/*
		Misc uncategorized stuff that I can't find a better place for
	*/
	// Drops a client.
	void DropClient(int clientNum) {
		// TODO: gamecode for dropped client
		R_Message(PRIORITY_MESSAGE, "DropClient: %i\n", clientNum);
		numConnectedClients--;
	}

	// Start a local server
	bool StartLocalServer() {
		myClientNum = 0;
		return localSocket->StartListening(net_port->Integer(), net_serverbacklog->Integer());
	}
}