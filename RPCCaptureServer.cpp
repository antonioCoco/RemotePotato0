/*
sub netntlmv2 {
	my $pwd = shift;
	my $nthash = Authen::Passphrase::NTHash->new(passphrase => $pwd)->hash;
	my $domain = randstr(rand(15)+1);
	my $user = randusername(20);
	my $identity = Encode::encode("UTF-16LE", uc($user).$domain);
	my $s_challenge = randbytes(8);
	my $c_challenge = randbytes(8);
	my $temp = '\x01\x01' . "\x00"x6 . randbytes(8) . $c_challenge . "\x00"x4 . randbytes(20*rand()+1) . '\x00';
	my $ntproofstr = _hmacmd5(_hmacmd5($nthash, $identity), $s_challenge.$temp);
	# $ntresponse = $ntproofstr.$temp but we separate them with a :
	printf("%s\\%s:::%s:%s:%s::%s:netntlmv2\n", $domain, $user, binToHex($s_challenge), binToHex($ntproofstr), binToHex($temp), $pwd);
}

[+] Listening for events...
[HTTP] NTLMv2 Client   : 10.0.0.10
[HTTP] NTLMv2 Username : APT0\abc
[HTTP] NTLMv2 Hash     : abc::APT0:7c142eaff7a905c6:F59C58FEBD3F0921B6ABDDADF59BE57B:0101000000000000BDEC33A6FF83D7012F3EE10C58CEE737000000000200060053004D0042000100160053004D0042002D0054004F004F004C004B00490054000400120073006D0062002E006C006F00630061006C000300280073006500720076006500720032003000300033002E0073006D0062002E006C006F00630061006C000500120073006D0062002E006C006F00630061006C000800300030000000000000000100000000100000D57EDE632516F412A18840D6F3CEA171CB4EA16022A44CA3DCC24AF3443D88BD0A0010000000000000000000000000000000000009001C0048005400540050002F00310030002E0030002E0030002E00320030000000000000000000

*/

#define WIN32_LEAN_AND_MEAN

#include "Windows.h"
#include "stdio.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include "RPCCaptureServer.h"

#pragma comment (lib, "Ws2_32.lib")

void DoRpcServerCaptureCredsHash(wchar_t* rpcServerIp, wchar_t* rpcServerPort, wchar_t* rpcRelayServerListeningPort) {
	
}
