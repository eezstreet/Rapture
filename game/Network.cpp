#include "sys_local.h"

#define RAPTURE_DEFAULT_PORT		1750
#define RAPTURE_DEFAULT_BACKLOG		32
#define RAPTURE_DEFAULT_MAXCLIENTS	8

#define RAPTURE_NETPROTOCOL			0

namespace Network {
	using namespace ClientPacket;

	enum Netmode_e {
		Netmode_Red,		// No connections allowed on the server, and any current connections are terminated
		Netmode_Yellow,		// No new connections allowed, current connections aren't terminated
		Netmode_Green,		// Anything goes
	};

	enum Netstate_e {
		Netstate_NoConnect,		// Not connected to any network, not listening
		Netstate_Listen,		// Listening
		Netstate_NeedAuth,		// Connected to a server, need authorization packet
		Netstate_Authorized,	// Connected to a server and authorized
	};

	static Cvar*				net_port = nullptr;
	static Cvar*				net_serverbacklog = nullptr;
	static Cvar*				net_maxclients = nullptr;
	static Cvar*				net_netmode = nullptr;

	static Socket*				localSocket = nullptr;
	static map<int, Socket*>	mOtherConnectedClients;
	static Socket*				remoteSocket = nullptr;
	static vector<Packet>		vPacketsAwaitingSend;	// Goes in both directions
	static Netstate_e			currentNetState = Netstate_NoConnect;
	static vector<Socket*>		vTemporaryConnections;

	static int			numConnectedClients = 1;
	static int			myClientNum = 0;				// Client 0 is always the host
	static int			lastFreeClientNum = 1;

	RaptureGame*		sys = nullptr;

	void Netmode_Callback(int newValue) {
		if (newValue == Netmode_Red) {
			mOtherConnectedClients.clear();
			numConnectedClients = 1;
		}
	}

	// Initialize the network
	void Init() {
		net_port = Cvar::Get<int>("net_port", "Port used for networking (TCP)", (1 << CVAR_ARCHIVE), RAPTURE_DEFAULT_PORT);
		net_serverbacklog
			= Cvar::Get<int>("net_serverbacklog", "Maximum number of waiting connections for the server", (1 << CVAR_ARCHIVE), RAPTURE_DEFAULT_BACKLOG);
		net_maxclients = Cvar::Get<int>("net_maxclients", "Maximum number of clients allowed on server", (1 << CVAR_ROM) | (1 << CVAR_ARCHIVE), RAPTURE_DEFAULT_MAXCLIENTS);
		net_netmode = Cvar::Get<int>("net_netmode", "Current netmode", 0, Netmode_Red);

		net_netmode->AddCallback(Netmode_Callback);
		Sys_InitSockets();

		// Create a local socket so we can serve as the host later
		localSocket = new Socket(AF_INET6, SOCK_STREAM);
	}

	// Shut down the network, delete the local socket, disconnect any clients, etc.
	void Shutdown() {
		delete localSocket;

		Sys_ExitSockets();
	}

	// Dispatches a single packet that's been sent to us from the server
	void DispatchSingleServerPacket(Packet& packet) {
		switch (packet.packetHead.type) {
			case PACKET_CLIENTACCEPT:
				{
					// We have been accepted into the server
					ClientAcceptPacket* cPacket = (ClientAcceptPacket*)(packet.packetData);
					myClientNum = cPacket->clientNum;
					currentNetState = Netstate_Authorized;
				}
				return;
			case PACKET_CLIENTDENIED:
				{
					// We have been denied from the server
					ClientDeniedPacket* cPacket = (ClientDeniedPacket*)(packet.packetData);
					R_Message(PRIORITY_MESSAGE, "Denied entry from server: %s\n", cPacket->why);
					currentNetState = Netstate_NoConnect;
				}
				return;
			case PACKET_PING:
				{
					R_Message(PRIORITY_DEBUG, "Server PONG");
				}
				return;
		}

		if (sys && sys->trap) {
			sys->trap->interpretPacketFromServer(&packet);
		}
	}

	// Dispatches a single packet that's been sent to us from the client
	void DispatchSingleClientPacket(Packet& packet) {
		switch (packet.packetHead.type) {
			case PACKET_PING:
				R_Message(PRIORITY_DEBUG, "Server PING from %i\n", packet.packetHead.clientNum);
				SendServerPacketTo(PACKET_PING, packet.packetHead.clientNum, nullptr, 0);
				return;
		}

		if (sys && sys->trap) {
			sys->trap->interpretPacketFromClient(&packet);
		}
	}

	// Check for new incoming temporary connections
	void CheckTemporaryConnections() {
		Socket* newConnection = localSocket->CheckPendingConnections();
		if (newConnection != nullptr) {
			vTemporaryConnections.push_back(newConnection);
		}

		vector<Socket*> readReady, writeReady;	// WriteReady is not actually used in this case
		Socket::Select(vTemporaryConnections, readReady, writeReady);
		for (auto& socket : readReady) {
			Packet incPacket;
			socket->ReadPacket(incPacket);

			Packet outPacket;
			if (incPacket.packetHead.type == PACKET_CLIENTATTEMPT) {
				if (sys && sys->trap && sys->trap->acceptclient((ClientAttemptPacket*)incPacket.packetData)) {
					// Send an acceptance packet with the new client number. Also remove this socket from temporary read packets
					outPacket.packetHead.clientNum = lastFreeClientNum++;
					outPacket.packetHead.direction = Packet::PD_ServerClient;
					outPacket.packetHead.sendTime = 0; // FIXME
					outPacket.packetHead.type = PACKET_CLIENTACCEPT;
					outPacket.packetHead.packetSize = 0; // FIXME

					mOtherConnectedClients[outPacket.packetHead.clientNum] = socket;
				}
				else {
					outPacket.packetHead.clientNum = -1;
					outPacket.packetHead.direction = Packet::PD_ServerClient;
					outPacket.packetHead.sendTime = 0; // FIXME
					outPacket.packetHead.type = PACKET_CLIENTDENIED;
					outPacket.packetHead.packetSize = 0; // FIXME
				}
				socket->SendPacket(outPacket);
			}
		}
	}

	// Run all of the server stuff
	void ServerFrame() {
		// Try listening for any temporary connections
		CheckTemporaryConnections();

		// Create list of sockets
		vector<Socket*> vSockets = { localSocket };
		for (auto it = mOtherConnectedClients.begin(); it != mOtherConnectedClients.end(); ++it) {
			vSockets.push_back(it->second);
		}

		// Poll sockets
		vector<Socket*> vSocketsRead, vSocketsWrite;
		Socket::Select(vSockets, vSocketsRead, vSocketsWrite);

		// Deal with sockets that need reading
		for (auto& socket : vSocketsRead) {
			vector<Packet> vPackets;
			socket->ReadAllPackets(vPackets);
			for (auto& packet : vPackets) {
				DispatchSingleClientPacket(packet);
			}
		}

		// Deal with sockets that need writing
		for (auto& packet : vPacketsAwaitingSend) {
			int8_t clientNum = packet.packetHead.clientNum;
			if (clientNum != -1) {
				auto found = mOtherConnectedClients.find(clientNum);
				if (found == mOtherConnectedClients.end()) {
					R_Message(PRIORITY_WARNING,
						"Tried to send packet %i with bad client %i\n",
						packet.packetHead.type,
						packet.packetHead.clientNum);
					continue;
				}
				found->second->SendPacket(packet);
			}
			else {
				for (auto it = mOtherConnectedClients.begin(); it != mOtherConnectedClients.end(); ++it) {
					it->second->SendPacket(packet);
				}
				DispatchSingleServerPacket(packet);	// Don't forget to send to ourselves!
			}
		}
		vPacketsAwaitingSend.clear();

		if (sys && sys->trap) {
			sys->trap->runserverframe();
		}
	}

	// Run all of the client stuff
	void ClientFrame() {
		bool bRead, bWrite;
		Socket::SelectSingle(remoteSocket, bRead, bWrite);

		if (bRead) {
			vector<Packet> vPackets;
			localSocket->ReadAllPackets(vPackets);
			for (auto& packet : vPackets) {
				DispatchSingleServerPacket(packet);
			}
		}
		if (bWrite) {
			for (auto& packet : vPacketsAwaitingSend) {
				localSocket->SendPacket(packet);
			}
			vPacketsAwaitingSend.clear();
		}
		if (sys && sys->trap) {
			sys->trap->runclientframe();
		}
	}

	// Start running the client
	void StartRunningClient(const char* szSaveGame, RaptureGame* pGameModule) {
		sys = pGameModule;
		if (sys->trap != nullptr) {
			sys->trap->startclientfromsave(szSaveGame);
		}
	}

	// Start running the server
	void StartRunningServer(const char* szSaveGame, RaptureGame* pGameModule) {
		sys = pGameModule;
		if (sys->trap != nullptr) {
			sys->trap->startserverfromsave(szSaveGame);
		}
	}

	// Start a local server
	bool StartLocalServer(const char* szSaveGame, RaptureGame* pGameModule) {
		if (!localSocket->StartListening(net_port->Integer(), net_serverbacklog->Integer())) {
			return false;
		}
		StartRunningServer(szSaveGame, pGameModule);
		StartRunningClient(szSaveGame, pGameModule);
		return true;
	}

	// Join a remote server
	bool JoinServer(const char* szSaveGame, const char* hostname, RaptureGame* pGameModule) {
		if (!ConnectToRemote(hostname, net_port->Integer())) {
			return false;
		}
		StartRunningClient(szSaveGame, pGameModule);
		return true;
	}

	// Send a server packet to a single client
	void SendServerPacketTo(packetType_e packetType, int clientNum, void* packetData, size_t packetDataSize) {
		Packet packet = { { packetType, clientNum, Packet::PD_ServerClient, 0, packetDataSize }, packetData };
		if (clientNum == myClientNum) {
			DispatchSingleServerPacket(packet);
		}
		else {
			auto it = mOtherConnectedClients.find(clientNum);
			if (it == mOtherConnectedClients.end()) {
				R_Message(PRIORITY_WARNING, "Tried to send a packet (%i) to invalid client %i\n", packetType, clientNum);
				return;
			}
			else {
				it->second->SendPacket(packet);
			}
		}
	}

	// Send packet from server -> client
	void SendServerPacket(packetType_e packetType, int clientNum, void* packetData, size_t packetDataSize) {
		if (clientNum != -1) {
			SendServerPacketTo(packetType, clientNum, packetData, packetDataSize);
		}
		else for (auto it = mOtherConnectedClients.begin(); it != mOtherConnectedClients.end(); ++it) {
			SendServerPacketTo(packetType, it->first, packetData, packetDataSize);
		}
	}

	// Send packet from client -> server
	void SendClientPacket(packetType_e packetType, void* packetData, size_t packetDataSize) {
		// FIXME: use the correct timestamp
		Packet packet = { { packetType, myClientNum, Packet::PD_ClientServer, 0, packetDataSize }, packetData };
		if (remoteSocket == nullptr) {
			// Not connected to a remote server, send it to ourselves instead
			DispatchSingleClientPacket(packet);
		}
		else {
			remoteSocket->SendPacket(packet);
		}
	}

	// Determine if the local server is full.
	bool LocalServerFull() {
		return numConnectedClients >= net_maxclients->Integer();
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

	// Try and connect to a server
	bool ConnectToRemote(const char* hostname, int port) {
		DisconnectFromRemote();
		remoteSocket = new Socket(AF_INET6, SOCK_STREAM);
		
		bool connected = remoteSocket->Connect(hostname, port);
		if (!connected) {
			delete remoteSocket;
			remoteSocket = nullptr;
		}
		currentNetState = Netstate_NeedAuth;
		return connected;
	}

	// Disconnect from current remote server
	void DisconnectFromRemote() {
		if (sys && sys->trap) {
			sys->trap->saveandexit();
		}
		if (remoteSocket != nullptr) {
			delete remoteSocket;
			remoteSocket = nullptr;
		}
		currentNetState = Netstate_NoConnect;
		R_Message(PRIORITY_NOTE, "--- Disconnected ---\n");
	}
}