#pragma once
#include "Windows.h"

SOCKET CreateRPCSocketListen(const wchar_t*);
SOCKET CreateRPCSocketReflect(const wchar_t*, const wchar_t*);
void ForgeAndAlterType2Rpc(char*, int, int, char*, int, char*);
void ExtractType3FromRpc(char*, int, char*, int*);
int findNTLMBytes(char*, int);

//debug
void hexDump2(char*, void*, int);