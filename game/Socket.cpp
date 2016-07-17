#include "sys_local.h"
#ifdef _WIN32
#include <WinSock2.h>
#include <Ws2TcpIp.h>
#endif

#define INET_PORTLEN	16

/* Non-static member functions */

// Creates a new socket object with family and type
Socket::Socket(int af_, int type_) {
	af = af_;
	type = type_;
	internalSocket = socket(af, type, IPPROTO_TCP);
	if (internalSocket < 0 && af == AF_INET6) {
		// retry using IPv4
		af = AF_INET;
		internalSocket = socket(af, type, IPPROTO_TCP);
	}

	// Set some extra options
	int yes = 1;
	int no = 0;
	setsockopt(internalSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&yes, sizeof(yes));
	setsockopt(internalSocket, IPPROTO_TCP, TCP_NODELAY, (const char*)&yes, sizeof(yes));

	// If it's IPv6 (or unspecified) we should allow IPv4 connections as well
	if (af == AF_INET6) {
		setsockopt(internalSocket, IPPROTO_IPV6, IPV6_V6ONLY, (const char*)&no, sizeof(no));
	}
}

// Creates a new socket object from another socket object (copy constructor)
Socket::Socket(Socket& other) {
	af = other.af;
	type = other.type;
	internalSocket = other.internalSocket;
}

// Creates a new socket from an internal socket and some information on the address.
Socket::Socket(addrinfo& connectingClientInfo, socket_t socket) {
	af = connectingClientInfo.ai_family;
	type = connectingClientInfo.ai_socktype;
	internalSocket = socket;

	// Set some extra options
	int yes = 1;
	int no = 0;
	setsockopt(internalSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&yes, sizeof(yes));
	setsockopt(internalSocket, IPPROTO_TCP, TCP_NODELAY, (const char*)&yes, sizeof(yes));

	if (af == AF_INET6) {
		setsockopt(internalSocket, IPPROTO_IPV6, IPV6_V6ONLY, (const char*)&no, sizeof(no));
	}
}

Socket::~Socket() {
	Disconnect();
	closesocket(internalSocket);
}

bool Socket::SetNonBlocking() {
	unsigned long ulMode = 1;

	// Enable non-blocking recv and write
	ioctlsocket(internalSocket, FIONBIO, &ulMode);
	if (ulMode != 1) {
		R_Message(PRIORITY_ERROR, "Couldn't establish non-blocking socket\n");
		return false;
	}

	return true;
}

// Binds the socket and starts listening
bool Socket::StartListening(unsigned short port, uint32_t backlog) {
	addrinfo hints;
	addrinfo* value;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = af;
	hints.ai_socktype = type;
	hints.ai_flags = AI_PASSIVE;

	char szPort[16] = { 0 };
	sprintf(szPort, "%i", port);

	getaddrinfo(nullptr, szPort, &hints, &value);
	int code = ::bind(internalSocket, value->ai_addr, value->ai_addrlen);
	if (code != 0) {
		R_Message(PRIORITY_ERROR, "Failed to bind socket on port %s (code %i)\n", szPort, code);
		return false;
	}

	if (!SetNonBlocking()) {
		return false;
	}

	listen(internalSocket, backlog);
	R_Message(PRIORITY_MESSAGE, "Now listening on port %i\n", port);
	return true;
}

// Check for any pending connections on this line. Returns a newly created socket if we found a connection.
Socket* Socket::CheckPendingConnections() {
	sockaddr_storage clientInfo;
	int sockSize = sizeof(clientInfo);
	sockaddr* genericClientInfo = (sockaddr*)&clientInfo;
	socket_t value = accept(internalSocket, genericClientInfo, &sockSize);
	if (value < 0) {
		// No connection found
		return nullptr;
	}

	// Found a connection, fill out some preliminary data about the client that connected
	addrinfo connectingInfo = { 0 };
	connectingInfo.ai_family = clientInfo.ss_family;
	connectingInfo.ai_protocol = IPPROTO_TCP;
	connectingInfo.ai_socktype = type;
	switch (connectingInfo.ai_family) {
		case AF_INET:	// IPv4
		{
			sockaddr_in* sockIn = (sockaddr_in*)genericClientInfo;
			connectingInfo.ai_addrlen = sizeof(sockaddr_in);
			connectingInfo.ai_addr = (sockaddr*)Zone::Alloc(connectingInfo.ai_addrlen, "network");
			memcpy(connectingInfo.ai_addr, sockIn, connectingInfo.ai_addrlen);
			R_Message(PRIORITY_MESSAGE, "Pending connection: %s\n", inet_ntoa(sockIn->sin_addr));
		}
		break;
		case AF_INET6:	// IPv6
		{
			sockaddr_in6* sockIn = (sockaddr_in6*)genericClientInfo;
			char ipBuffer[INET6_ADDRSTRLEN];
			inet_ntop(AF_INET6, &sockIn->sin6_addr, ipBuffer, sizeof(ipBuffer));
			connectingInfo.ai_addrlen = sizeof(sockaddr_in6);
			connectingInfo.ai_addr = (sockaddr*)Zone::Alloc(connectingInfo.ai_addrlen, "network");
			memcpy(connectingInfo.ai_addr, sockIn, connectingInfo.ai_addrlen);
			R_Message(PRIORITY_MESSAGE, "Pending connection: %s\n", ipBuffer);
		}
		break;
	}

	lastHeardFrom = SDL_GetTicks();
	return new Socket(connectingInfo, value);
}

// Sends an entire chunk of data across the network, without fragmentation
bool Socket::SendEntireData(void* data, size_t dataSize) {
	size_t sent = 0;
	while (sent < dataSize) {
		int numSent = send(internalSocket, (const char*)data + sent, dataSize - sent, 0);
		if (numSent < 0) {
			int errorNum;
			const char* errMsg = Sys_SocketError(errorNum);
			R_Message(PRIORITY_ERROR, "Socket::SendEntireData: %s (error code %i)\n", errMsg, errorNum);
			return false;
		}
		sent += numSent;
	}
	return true;
}

// Send a packet across the network.
// Guaranteed delivery (no fragmentation), does not block.
bool Socket::SendPacket(Packet& outgoing) {
	bool sent = SendEntireData(&outgoing.packetHead, sizeof(outgoing.packetHead));
	if (!sent) {
		R_Message(PRIORITY_WARNING,
			"Failed to send packet header (packet type: %i; clientNum: %i)\n",
			outgoing.packetHead.type, outgoing.packetHead.clientNum);
		return false;
	}
	if (outgoing.packetHead.packetSize > 0) {
		sent = SendEntireData(outgoing.packetData, outgoing.packetHead.packetSize);
		if (!sent) {
			R_Message(PRIORITY_WARNING,
				"Failed to send packet data (packet type: %i; clientNum: %i\n",
				outgoing.packetHead.type, outgoing.packetHead.clientNum);
			return false;
		}
	}
	lastSpoken = SDL_GetTicks();
	return true;
}

// Read an entire block of memory from a socket, without fragmentation.
bool Socket::ReadEntireData(void* data, size_t dataSize) {
	size_t received = 0;
	while (received < dataSize) {
		int numRead = recv(internalSocket, (char*)data + received, dataSize - received, 0);
		if (numRead == 0) {
			return false;		// dead connection
		}
		if (numRead < 0) {
			int errorNum;
			const char* errorMsg = Sys_SocketError(errorNum);
			R_Message(PRIORITY_ERROR, "Socket::ReadEntireData failed. %s (error code %i)\n", errorMsg, errorNum);
			return false;
		}
		received += numRead;
	}
	return true;
}

// Read a packet from a socket.
// Guaranteed delivery (no fragmentation), may block.
bool Socket::ReadPacket(Packet& incomingPacket) {
	memset(&incomingPacket, 0, sizeof(incomingPacket));

	if (!ReadEntireData(&incomingPacket.packetHead, sizeof(incomingPacket.packetHead))) {
		// TODO: make it rain fire from the sky, we dropped a packet
		return false;
	}

	if (incomingPacket.packetHead.packetSize > 0) {
		incomingPacket.packetData = Zone::Alloc(incomingPacket.packetHead.packetSize, "network");
		if (!ReadEntireData(&incomingPacket.packetData, incomingPacket.packetHead.packetSize)) {
			Zone::FastFree(incomingPacket.packetData, "network");
			return false;
		}
	}

	lastHeardFrom = SDL_GetTicks();
	return true;
}

// Connect this socket to a hostname and port.
// Not necessary unless the af/type constructor or the copy constructor are used.
// Also establishes this socket as being nonblocking.
bool Socket::Connect(const char* hostname, unsigned short port) {
	// First we need to resolve the hostname before we can bind the socket
	addrinfo hints{ af == AF_INET6 ? AI_V4MAPPED : 0, af, 0, IPPROTO_TCP, 0, nullptr, nullptr, nullptr };
	addrinfo* results;
	char szPort[INET_PORTLEN] {0};
	char ipBuffer[INET6_ADDRSTRLEN] {0};

	std::sprintf(szPort, "%i", port);

	// Convert the IP address into a valid IPv6 address
	int dwReturn = getaddrinfo(hostname, szPort, &hints, &results);
	if (dwReturn != 0) {
		R_Message(PRIORITY_ERROR, "Could not resolve hostname %s (reason: %s)\n", hostname, gai_strerror(dwReturn));
		return false;
	}

	// Create the sockaddr fields necessary for the connection
	SOCKADDR_STORAGE* in;
	size_t addrSize;
	switch (af) {
		case AF_INET6:
		{
			sockaddr_in6* in6 = (sockaddr_in6*)results->ai_addr;
			inet_ntop(af, &in6->sin6_addr, ipBuffer, sizeof(ipBuffer));
			in6->sin6_port = htons(port);
			in6->sin6_family = AF_INET6;
			in = (SOCKADDR_STORAGE*)in6;
			addrSize = sizeof(sockaddr_in6);
		}
		break;
		case AF_INET:
		{
			sockaddr_in* in4 = (sockaddr_in*)results->ai_addr;
			inet_ntop(af, &in4->sin_addr, ipBuffer, sizeof(ipBuffer));
			in4->sin_port = htons(port);
			in4->sin_family = AF_INET;
			in = (SOCKADDR_STORAGE*)in4;
			addrSize = sizeof(sockaddr_in);
		}
		break;
	}

	R_Message(PRIORITY_MESSAGE, "%s resolved to %s\n", hostname, ipBuffer);

	// Finally actually connect to the remote (in blocking mode)
	if (connect(internalSocket, (sockaddr*)in, addrSize) != 0) {
		int errorCode;
		const char* errorMsg = Sys_SocketError(errorCode);
		R_Message(PRIORITY_ERROR, "Socket::Connect failed (%i: %s)\n", errorCode, errorMsg);
		return false;
	}

	freeaddrinfo(results);
	// Set us as non-blocking
	if (!SetNonBlocking()) {
		return false;
	}

	lastHeardFrom = SDL_GetTicks();

	return true;
}

// Disconnects a socket.
// Called automatically on destroyed.
void Socket::Disconnect() {
	shutdown(internalSocket, SD_SEND);
}

/* Static member functions */

bool Socket::Select() {
	fd_set readSet{ 0 };
	timeval timeout{ 0, 0 };
	
	FD_SET(internalSocket, &readSet);
	select(1, &readSet, nullptr, nullptr, &timeout);

	if (FD_ISSET(internalSocket, &readSet)) {
		return true;
	}
	else {
		return false;
	}
}