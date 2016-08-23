#include "sys_local.h"

namespace Network {

	Cvar*				net_port = nullptr;
	Cvar*				net_serverbacklog = nullptr;
	Cvar*				net_maxclients = nullptr;
	Cvar*				net_netmode = nullptr;
	Cvar*				net_timeout = nullptr;
	Cvar*				net_ipv6 = nullptr;

	Socket*				localSocket = nullptr;
	map<int, Socket*>	mOtherConnectedClients;
	Socket*				remoteSocket = nullptr;
	vector<packetMsg>	vPacketsAwaitingSend;
	Netstate_e			currentNetState = Netstate_NoConnect;
	vector<Socket*>		vTemporaryConnections;

	int			numConnectedClients = 1;
	int			myClientNum = 0;				// Client 0 is always the host
	int			lastFreeClientNum = 1;

	char			packetSendingBuffer[RAPTURE_NETBUFFER_SIZE] {0};
	char			packetReceivingBuffer[RAPTURE_NETBUFFER_SIZE] {0};
	size_t		packetSendingCursor = 0;
	size_t		packetReceivingCursor = 0;
	networkCallbackFunction	callbacks[NIC_MAX] {nullptr};

	void Netmode_Callback(int newValue) {
		/*if (newValue == Netmode_Red) {
			mOtherConnectedClients.clear();
			numConnectedClients = 1;
		}*/
	}

	// Initialize the network
	void Init() {
		Zone::NewTag("network");
		net_port = Cvar::Get<int>("net_port", "Port used for networking (TCP)", (1 << CVAR_ARCHIVE), RAPTURE_DEFAULT_PORT);
		net_serverbacklog
			= Cvar::Get<int>("net_serverbacklog", "Maximum number of waiting connections for the server", (1 << CVAR_ARCHIVE), RAPTURE_DEFAULT_BACKLOG);
		net_maxclients = Cvar::Get<int>("net_maxclients", "Maximum number of clients allowed on server", (1 << CVAR_ROM) | (1 << CVAR_ARCHIVE), RAPTURE_DEFAULT_MAXCLIENTS);
		net_netmode = Cvar::Get<int>("net_netmode", "Current netmode", 0, Netmode_Red);
		net_timeout = Cvar::Get<int>("net_timeout", "Timeout duration, in milliseconds", 0, 90000);
		net_ipv6 = Cvar::Get<bool>("net_ipv6", "Whether to use IPv6 addresses", (1 << CVAR_ARCHIVE), false);

		net_netmode->AddCallback(Netmode_Callback);
		Sys_InitSockets();

		// Create a local socket so we can serve as the host later
		localSocket = new Socket(net_ipv6->Bool() ? AF_INET6 : AF_INET, SOCK_STREAM);
	}

	// Shut down the network, delete the local socket, disconnect any clients, etc.
	void Shutdown() {
		delete localSocket;

		Sys_ExitSockets();
	}

	// Determine if the local server is full.
	bool LocalServerFull() {
		return numConnectedClients >= net_maxclients->Integer();
	}

	void AddCallback(NetworkInterfaceCallbacks callback, networkCallbackFunction func) {
		if (callback == NIC_ALL) {
			R_Message(PRIORITY_WARNING, "Callback added to NIC_ALL (invalid)\n");
			return;
		}
		callbacks[callback] = func;
	}

	void RemoveCallback(NetworkInterfaceCallbacks callback) {
		if (callback == NIC_ALL) {
			memset(callbacks, 0, sizeof(networkCallbackFunction) * NIC_MAX);
		}
		else {
			callbacks[callback] = nullptr;
		}
	}
}

