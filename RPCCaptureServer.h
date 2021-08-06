#define SECURITY_WIN32 

#pragma once
#include <security.h>

void ForgeNtlmType2(char*, int, char*, int*);
void InitTokenContextBuffer(PSecBufferDesc, PSecBuffer);
void PrintCapturedHash(char*, char*);
void DoRpcServerCaptureCredsHash(wchar_t*, wchar_t*, wchar_t*);